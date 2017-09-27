/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2017 BMW Car IT GmbH
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

#include <cassert>
#include <limits>
#include <string>
#include <stdint.h>
#include <memory>

#include "MyRadioHelper.h"
#include "joynr/vehicle/RadioProxy.h"
#include "joynr/vehicle/RadioNewStationDiscoveredBroadcastFilterParameters.h"
#include "joynr/JoynrRuntime.h"
#include "joynr/ISubscriptionListener.h"
#include "joynr/SubscriptionListener.h"
#include "joynr/OnChangeWithKeepAliveSubscriptionQos.h"
#include "joynr/MulticastSubscriptionQos.h"
#include "joynr/serializer/Serializer.h"
#include "joynr/Logger.h"
#include "joynr/Future.h"
#ifdef JOYNR_ENABLE_DLT_LOGGING
#include <dlt/dlt.h>
#endif // JOYNR_ENABLE_DLT_LOGGING

using namespace joynr;

// A class that listens to messages generated by subscriptions
class RadioStationListener : public SubscriptionListener<vehicle::RadioStation>
{
public:
    RadioStationListener() = default;

    ~RadioStationListener() = default;

    void onSubscribed(const std::string& subscriptionId)
    {
        MyRadioHelper::prettyLog(
                logger,
                "ATTRIBUTE SUBSCRIPTION current station successful, subscriptionId: " +
                        subscriptionId);
    }

    void onReceive(const vehicle::RadioStation& value)
    {
        MyRadioHelper::prettyLog(
                logger, "ATTRIBUTE SUBSCRIPTION current station: " + value.toString());
    }

    void onError(const exceptions::JoynrRuntimeException& error)
    {
        if (error.getTypeName() == exceptions::PublicationMissedException::TYPE_NAME()) {
            MyRadioHelper::prettyLog(logger,
                                     "ATTRIBUTE SUBSCRIPTION Publication Missed, subscriptionId: " +
                                             error.getMessage());
        } else {
            MyRadioHelper::prettyLog(logger, "ATTRIBUTE SUBSCRIPTION error: " + error.getMessage());
        }
    }

private:
    ADD_LOGGER(RadioStationListener);
};

INIT_LOGGER(RadioStationListener);

// A class that listens to messages generated by subscriptions
class WeakSignalBroadcastListener : public SubscriptionListener<vehicle::RadioStation>
{
public:
    WeakSignalBroadcastListener() = default;

    ~WeakSignalBroadcastListener() = default;

    void onSubscribed(const std::string& subscriptionId)
    {
        MyRadioHelper::prettyLog(
                logger,
                "BROADCAST SUBSCRIPTION weak signal successful, subscriptionId: " + subscriptionId);
    }

    void onReceive(const vehicle::RadioStation& value)
    {
        MyRadioHelper::prettyLog(logger, "BROADCAST SUBSCRIPTION weak signal: " + value.toString());
    }

private:
    ADD_LOGGER(WeakSignalBroadcastListener);
};

INIT_LOGGER(WeakSignalBroadcastListener);

// A class that listens to messages generated by subscriptions
class NewStationDiscoveredBroadcastListener
        : public SubscriptionListener<vehicle::RadioStation, vehicle::GeoPosition>
{
public:
    NewStationDiscoveredBroadcastListener() = default;

    ~NewStationDiscoveredBroadcastListener() = default;

    void onSubscribed(const std::string& subscriptionId)
    {
        MyRadioHelper::prettyLog(
                logger,
                "BROADCAST SUBSCRIPTION new station discovered successful, subscriptionId: " +
                        subscriptionId);
    }

    void onReceive(const vehicle::RadioStation& discoveredStation,
                   const vehicle::GeoPosition& geoPosition)
    {
        MyRadioHelper::prettyLog(logger,
                                 "BROADCAST SUBSCRIPTION new station discovered: " +
                                         discoveredStation.toString() + " at " +
                                         geoPosition.toString());
    }

private:
    ADD_LOGGER(NewStationDiscoveredBroadcastListener);
};

INIT_LOGGER(NewStationDiscoveredBroadcastListener);

//------- Main entry point -------------------------------------------------------

int main(int argc, char* argv[])
{
    using joynr::vehicle::Radio::AddFavoriteStationErrorEnum;

// Register app at the dlt-daemon for logging
#ifdef JOYNR_ENABLE_DLT_LOGGING
    DLT_REGISTER_APP("JYRC", argv[0]);
#endif // JOYNR_ENABLE_DLT_LOGGING

    // Get a logger
    joynr::Logger logger("MyRadioConsumerApplication");

    // Check the usage
    std::string programName(argv[0]);
    if (argc != 2) {
        JOYNR_LOG_ERROR(logger, "USAGE: {} <provider-domain>", programName);
        return 1;
    }

    // Get the provider domain
    std::string providerDomain(argv[1]);
    JOYNR_LOG_INFO(logger, "Creating proxy for provider on domain {}", providerDomain);

    // Get the current program directory
    std::string dir(MyRadioHelper::getAbsolutePathToExectuable(programName));

    // Initialise the JOYn runtime
    std::string pathToMessagingSettings(dir + "/resources/radio-app-consumer.settings");

    std::shared_ptr<JoynrRuntime> runtime = JoynrRuntime::createRuntime(pathToMessagingSettings);

    // Create proxy builder
    std::shared_ptr<ProxyBuilder<vehicle::RadioProxy>> proxyBuilder =
            runtime->createProxyBuilder<vehicle::RadioProxy>(providerDomain);

    // Messaging Quality of service
    std::int64_t qosMsgTtl = 30000;                // Time to live is 30 secs in one direction
    std::int64_t qosCacheDataFreshnessMs = 400000; // Only consider data cached for < 400 secs

    // Find the provider with the highest priority set in ProviderQos
    DiscoveryQos discoveryQos;
    // As soon as the discovery QoS is set on the proxy builder, discovery of suitable providers
    // is triggered. If the discovery process does not find matching providers within the
    // arbitration timeout duration it will be terminated and you will get an arbitration exception.
    discoveryQos.setDiscoveryTimeoutMs(40000);
    // Provider entries in the global capabilities directory are cached locally. Discovery will
    // consider entries in this cache valid if they are younger as the max age of cached
    // providers as defined in the QoS. All valid entries will be processed by the arbitrator when
    // searching
    // for and arbitrating the "best" matching provider.
    // NOTE: Valid cache entries might prevent triggering a lookup in the global capabilities
    //       directory. Therefore, not all providers registered with the global capabilities
    //       directory might be taken into account during arbitration.
    discoveryQos.setCacheMaxAgeMs(std::numeric_limits<std::int64_t>::max());
    // The discovery process outputs a list of matching providers. The arbitration strategy then
    // chooses one or more of them to be used by the proxy.
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY);

    // Build a proxy
    std::shared_ptr<vehicle::RadioProxy> proxy =
            proxyBuilder->setMessagingQos(MessagingQos(qosMsgTtl))
                    ->setDiscoveryQos(discoveryQos)
                    ->build();

    vehicle::RadioStation currentStation;
    try {
        proxy->getCurrentStation(currentStation);
    } catch (exceptions::JoynrException& e) {
        assert(false);
    }
    MyRadioHelper::prettyLog(logger, "ATTRIBUTE GET: " + currentStation.toString());
    // Run a short subscription using the proxy
    // Set the Quality of Service parameters for the subscription

    // The provider will send a notification whenever the value changes. The number of sent
    // notifications may be limited by the min interval QoS.
    // NOTE: The provider must support on-change notifications in order to use this feature by
    //       calling the <attribute>Changed method of the <interface>Provider class whenever the
    //       <attribute> value changes.
    auto subscriptionQos = std::make_shared<OnChangeWithKeepAliveSubscriptionQos>();
    // The provider will maintain at least a minimum interval idle time in milliseconds between
    // successive notifications, even if on-change notifications are enabled and the value changes
    // more often. This prevents the consumer from being flooded by updated values. The filtering
    // happens on the provider's side, thus also preventing excessive network traffic.
    subscriptionQos->setMinIntervalMs(5 * 1000);
    // The provider will send notifications every maximum interval in milliseconds, even if the
    // value didn't change. It will send notifications more often if on-change notifications are
    // enabled, the value changes more often, and the minimum interval QoS does not prevent it. The
    // maximum interval can thus be seen as a sort of heart beat.
    subscriptionQos->setMaxIntervalMs(8 * 1000);
    // The provider will send notifications until the end date is reached. The consumer will not
    // receive any notifications (neither value notifications nor missed publication notifications)
    // after this date.
    // setValidityMs will set the end date to current time millis + validity
    subscriptionQos->setValidityMs(60 * 1000);
    // Notification messages will be sent with this time-to-live. If a notification message can not
    // be delivered within its TTL, it will be deleted from the system.
    // NOTE: If a notification message is not delivered due to an expired TTL, it might raise a
    //       missed publication notification (depending on the value of the alert interval QoS).
    subscriptionQos->setAlertAfterIntervalMs(10 * 1000);

    // Subscriptions go to a listener object
    std::shared_ptr<ISubscriptionListener<vehicle::RadioStation>> listener(
            new RadioStationListener());

    // Subscribe to the radio station.
    std::shared_ptr<Future<std::string>> currentStationSubscriptionIdFuture =
            proxy->subscribeToCurrentStation(listener, subscriptionQos);

    // broadcast subscription

    // The provider will send a notification whenever the value changes. The number of sent
    // notifications may be limited by the min interval QoS.
    // NOTE: The provider must support on-change notifications in order to use this feature by
    //       calling the <broadcast>EventOccurred method of the <interface>Provider class whenever
    //       the <broadcast> should be triggered.
    auto weakSignalBroadcastSubscriptionQos = std::make_shared<MulticastSubscriptionQos>();

    // The provider will send notifications until the end date is reached. The consumer will not
    // receive any notifications (neither value notifications nor missed publication notifications)
    // after this date.
    // setValidityMs will set the end date to current time millis + validity
    weakSignalBroadcastSubscriptionQos->setValidityMs(60 * 1000);

    std::shared_ptr<ISubscriptionListener<vehicle::RadioStation>> weakSignalBroadcastListener(
            new WeakSignalBroadcastListener());
    std::shared_ptr<Future<std::string>> weakSignalBroadcastSubscriptionIdFuture =
            proxy->subscribeToWeakSignalBroadcast(
                    weakSignalBroadcastListener, weakSignalBroadcastSubscriptionQos);

    // selective broadcast subscription

    auto newStationDiscoveredBroadcastSubscriptionQos = std::make_shared<OnChangeSubscriptionQos>();
    newStationDiscoveredBroadcastSubscriptionQos->setMinIntervalMs(2 * 1000);
    newStationDiscoveredBroadcastSubscriptionQos->setValidityMs(180 * 1000);
    std::shared_ptr<ISubscriptionListener<vehicle::RadioStation, vehicle::GeoPosition>>
            newStationDiscoveredBroadcastListener(new NewStationDiscoveredBroadcastListener());
    vehicle::RadioNewStationDiscoveredBroadcastFilterParameters
            newStationDiscoveredBroadcastFilterParams;
    newStationDiscoveredBroadcastFilterParams.setHasTrafficService("true");
    vehicle::GeoPosition positionOfInterest(48.1351250, 11.5819810); // Munich
    std::string positionOfInterestJson(joynr::serializer::serializeToJson(positionOfInterest));
    newStationDiscoveredBroadcastFilterParams.setPositionOfInterest(positionOfInterestJson);
    newStationDiscoveredBroadcastFilterParams.setRadiusOfInterestArea("200000"); // 200 km
    std::shared_ptr<Future<std::string>> newStationDiscoveredBroadcastSubscriptionIdFuture =
            proxy->subscribeToNewStationDiscoveredBroadcast(
                    newStationDiscoveredBroadcastFilterParams,
                    newStationDiscoveredBroadcastListener,
                    newStationDiscoveredBroadcastSubscriptionQos);
    // add favorite radio station
    vehicle::RadioStation favoriteStation("99.3 The Fox Rocks", false, vehicle::Country::CANADA);
    bool success;
    try {
        proxy->addFavoriteStation(success, favoriteStation);
        MyRadioHelper::prettyLog(
                logger, "METHOD: added favorite station: " + favoriteStation.toString());
        proxy->addFavoriteStation(success, favoriteStation);
    } catch (exceptions::ApplicationException& e) {
        if (e.getError<AddFavoriteStationErrorEnum::Enum>() ==
            AddFavoriteStationErrorEnum::DUPLICATE_RADIOSTATION) {
            MyRadioHelper::prettyLog(
                    logger,
                    "METHOD: add favorite station a second time failed with the following "
                    "expected exception: " +
                            e.getName());
        } else {
            MyRadioHelper::prettyLog(
                    logger,
                    "METHOD: add favorite station a second time failed with the following "
                    "UNEXPECTED exception: " +
                            e.getName());
        }
    }

    try {
        favoriteStation.setName("");
        proxy->addFavoriteStation(success, favoriteStation);
    } catch (exceptions::ProviderRuntimeException& e) {
        if (e.getMessage() == MyRadioHelper::MISSING_NAME()) {
            MyRadioHelper::prettyLog(logger,
                                     "METHOD: add favorite station with empty name failed with the "
                                     "following "
                                     "expected exception: " +
                                             e.getMessage());
        } else {
            MyRadioHelper::prettyLog(logger,
                                     "METHOD: add favorite station with empty name failed "
                                     "with the following "
                                     "UNEXPECTED exception: " +
                                             e.getMessage());
        }
    }

    // shuffle the stations
    MyRadioHelper::prettyLog(logger, "METHOD: calling shuffle stations");
    proxy->shuffleStations();
    // Run until the user hits q
    int key;

    while ((key = MyRadioHelper::getch()) != 'q') {
        joynr::vehicle::GeoPosition location;
        joynr::vehicle::Country::Enum country;
        switch (key) {
        case 's':
            proxy->shuffleStations();
            break;
        case 'm':
            proxy->getLocationOfCurrentStation(country, location);
            MyRadioHelper::prettyLog(logger,
                                     "METHOD: getLocationOfCurrentStation: country: " +
                                             joynr::vehicle::Country::getLiteral(country) +
                                             ", location: " + location.toString());
            break;
        default:
            MyRadioHelper::prettyLog(logger,
                                     "USAGE press\n"
                                     " q\tto quit\n"
                                     " s\tto shuffle stations\n");
            break;
        }
    }

    // unsubscribe
    std::string currentStationSubscriptionId;
    try {
        currentStationSubscriptionIdFuture->get(2000, currentStationSubscriptionId);
        proxy->unsubscribeFromCurrentStation(currentStationSubscriptionId);
    } catch (const exceptions::JoynrRuntimeException& e) {
        JOYNR_LOG_ERROR(
                logger, "UNSUBSCRIBE from ATTRIBUTE current station FAILED: {}", e.getMessage());
    }
    std::string weakSignalBroadcastSubscriptionId;
    try {
        weakSignalBroadcastSubscriptionIdFuture->get(2000, weakSignalBroadcastSubscriptionId);
        proxy->unsubscribeFromWeakSignalBroadcast(weakSignalBroadcastSubscriptionId);
    } catch (const exceptions::JoynrRuntimeException& e) {
        JOYNR_LOG_ERROR(
                logger, "UNSUBSCRIBE from BROADCAST weak signal FAILED: {}", e.getMessage());
    }
    std::string newStationDiscoveredBroadcastSubscriptionId;
    try {
        newStationDiscoveredBroadcastSubscriptionIdFuture->get(
                2000, newStationDiscoveredBroadcastSubscriptionId);
        proxy->unsubscribeFromNewStationDiscoveredBroadcast(
                newStationDiscoveredBroadcastSubscriptionId);
    } catch (const exceptions::JoynrRuntimeException& e) {
        JOYNR_LOG_ERROR(logger,
                        "UNSUBSCRIBE from BROADCAST new station discovered FAILED: {}",
                        e.getMessage());
    }

    return 0;
}
