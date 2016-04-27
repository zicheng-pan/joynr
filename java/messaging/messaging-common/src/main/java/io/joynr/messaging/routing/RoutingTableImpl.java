package io.joynr.messaging.routing;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
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

import java.util.Map.Entry;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.collect.Maps;
import com.google.inject.Inject;
import com.google.inject.Singleton;
import com.google.inject.name.Named;
import io.joynr.messaging.ConfigurableMessagingSettings;
import io.joynr.runtime.SystemServicesSettings;
import joynr.system.RoutingTypes.Address;

@Singleton
public class RoutingTableImpl implements RoutingTable {
    private static final Logger logger = LoggerFactory.getLogger(RoutingTableImpl.class);
    ConcurrentMap<String, Address> hashMap = Maps.newConcurrentMap();

    RoutingTableImpl() {
    }

    // CHECKSTYLE:OFF
    @Inject
    public RoutingTableImpl(@Named(ConfigurableMessagingSettings.PROPERTY_CAPABILITIES_DIRECTORY_PARTICIPANT_ID) String capabilitiesDirectoryParticipantId,
                            @Named(ConfigurableMessagingSettings.PROPERTY_CAPABILITIES_DIRECTORY_ADDRESS) Address capabiltitiesDirectoryAddress,
                            @Named(ConfigurableMessagingSettings.PROPERTY_DOMAIN_ACCESS_CONTROLLER_PARTICIPANT_ID) String domainAccessControllerParticipantId,
                            @Named(ConfigurableMessagingSettings.PROPERTY_DOMAIN_ACCESS_CONTROLLER_ADDRESS) Address domainAccessControllerAddress,
                            @Named(SystemServicesSettings.PROPERTY_CC_DISCOVERY_PROVIDER_PARTICIPANT_ID) String discoveryProviderParticipantId,
                            @Named(SystemServicesSettings.PROPERTY_CC_MESSAGING_ADDRESS) Address discoveryProviderAddress) {
        // CHECKSTYLE:ON
        this.put(capabilitiesDirectoryParticipantId, capabiltitiesDirectoryAddress);
        this.put(domainAccessControllerParticipantId, domainAccessControllerAddress);
        this.put(discoveryProviderParticipantId, discoveryProviderAddress);
    }

    @Override
    public Address get(String participantId) {
        logger.debug("lookup participant: {}", participantId);
        dumpRoutingTableEntry();
        Address result = hashMap.get(participantId);
        logger.debug("Returning: {}", result);
        return result;
    }

    private void dumpRoutingTableEntry() {
        if (logger.isTraceEnabled()) {
            StringBuilder message = new StringBuilder("Routing table entries:\n");
            for (Entry<String, Address> eachEntry : hashMap.entrySet()) {
                message.append("\t> ")
                       .append(eachEntry.getKey())
                       .append("\t-\t")
                       .append(eachEntry.getValue())
                       .append("\n");
            }
            logger.trace(message.toString());
        }
    }

    @Override
    public Address put(String participantId, Address address) {
        logger.debug("adding endpoint address: {} for participant with ID {}", address, participantId);
        Address result = hashMap.putIfAbsent(participantId, address);
        logger.debug("Returning: {}", result);
        return result;
    }

    @Override
    public boolean containsKey(String participantId) {
        boolean containsKey = hashMap.containsKey(participantId);
        logger.debug("checking for participant: {} success: {}", participantId, containsKey);
        if (!containsKey) {
            dumpRoutingTableEntry();
        }
        return containsKey;
    }

    @Override
    public void remove(String participantId) {
        hashMap.remove(participantId);

    }
}
