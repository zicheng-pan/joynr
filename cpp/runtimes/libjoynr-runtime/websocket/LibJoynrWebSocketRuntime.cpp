/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
 * %%
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * #L%
 */
#include "runtimes/libjoynr-runtime/websocket/LibJoynrWebSocketRuntime.h"

#include "libjoynr/websocket/WebSocketMessagingStubFactory.h"
#include "joynr/system/RoutingTypes/WebSocketClientAddress.h"
#include "libjoynr/websocket/WebSocketLibJoynrMessagingSkeleton.h"
#include "joynr/Util.h"
#include "joynr/JsonSerializer.h"
#include "joynr/Semaphore.h"
#include "libjoynr/websocket/WebSocketPpClient.h"

namespace joynr
{

INIT_LOGGER(LibJoynrWebSocketRuntime);

LibJoynrWebSocketRuntime::LibJoynrWebSocketRuntime(Settings* settings)
        : LibJoynrRuntime(settings),
          wsSettings(*settings),
          wsLibJoynrMessagingSkeleton(nullptr),
          websocket(new WebSocketPpClient(wsSettings))
{
    std::string uuid = util::createUuid();
    // remove dashes
    uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
    std::string libjoynrMessagingId = "libjoynr.messaging.participantid_" + uuid;
    auto libjoynrMessagingAddress =
            std::make_shared<const joynr::system::RoutingTypes::WebSocketClientAddress>(
                    libjoynrMessagingId);

    // send initialization message containing libjoynr messaging address
    std::string initializationMsg = JsonSerializer::serialize(*libjoynrMessagingAddress);
    JOYNR_LOG_TRACE(logger,
                    "OUTGOING sending websocket intialization message\nmessage: {}\nto: {}",
                    initializationMsg,
                    libjoynrMessagingAddress->toString());
    auto connectionEstablishedSemaphore = std::make_shared<Semaphore>(0);
    auto connectCallback = [this, initializationMsg, connectionEstablishedSemaphore]() mutable {
        auto onFailure = [this](const exceptions::JoynrRuntimeException& e) {
            // initialization message will be sent after reconnect
            JOYNR_LOG_ERROR(logger,
                            "Sending websocket initialization message failed. Error: {}",
                            e.getMessage());
        };
        websocket->sendTextMessage(initializationMsg, onFailure);
        if (connectionEstablishedSemaphore) {
            connectionEstablishedSemaphore->notify();
            connectionEstablishedSemaphore = nullptr;
        }

    };
    websocket->registerConnectCallback(connectCallback);

    // create connection to parent routing service
    auto ccMessagingAddress = std::make_shared<const joynr::system::RoutingTypes::WebSocketAddress>(
            wsSettings.createClusterControllerMessagingAddress());

    websocket->connect(*ccMessagingAddress);

    auto factory = std::make_unique<WebSocketMessagingStubFactory>();
    factory->addServer(*ccMessagingAddress, websocket);

    connectionEstablishedSemaphore->wait();
    LibJoynrRuntime::init(std::move(factory), libjoynrMessagingAddress, ccMessagingAddress);
}

LibJoynrWebSocketRuntime::~LibJoynrWebSocketRuntime()
{
    delete wsLibJoynrMessagingSkeleton;
    wsLibJoynrMessagingSkeleton = nullptr;
}

void LibJoynrWebSocketRuntime::startLibJoynrMessagingSkeleton(MessageRouter& messageRouter)
{
    wsLibJoynrMessagingSkeleton = new WebSocketLibJoynrMessagingSkeleton(messageRouter);
    websocket->registerReceiveCallback([&](const std::string& msg) {
        wsLibJoynrMessagingSkeleton->onTextMessageReceived(msg);
    });
}

void LibJoynrWebSocketRuntime::onWebSocketError(const std::string& errorMessage)
{
    JOYNR_LOG_ERROR(logger, "WebSocket error occurred: {}", errorMessage);
}

} // namespace joynr
