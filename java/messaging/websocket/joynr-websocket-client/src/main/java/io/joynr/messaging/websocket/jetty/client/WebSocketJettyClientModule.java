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
package io.joynr.messaging.websocket.jetty.client;

import com.google.inject.AbstractModule;

import io.joynr.messaging.websocket.LibjoynrWebSocketModule;
import io.joynr.messaging.websocket.WebSocketEndpointFactory;

public class WebSocketJettyClientModule extends AbstractModule {

    @Override
    protected void configure() {
        install(new LibjoynrWebSocketModule());
        bind(WebSocketEndpointFactory.class).to(WebSocketJettyClientFactory.class);
    }
}
