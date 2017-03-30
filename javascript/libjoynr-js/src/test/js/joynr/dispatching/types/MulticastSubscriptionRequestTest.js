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

define([
    "joynr/dispatching/types/MulticastSubscriptionRequest",
    "joynr/proxy/OnChangeSubscriptionQos",
    "joynr/proxy/MulticastSubscriptionQos"
], function(MulticastSubscriptionRequest, OnChangeSubscriptionQos, MulticastSubscriptionQos) {

    describe("libjoynr-js.joynr.dispatching.types.MulticastSubscriptionRequest", function() {

        var qosSettings = {
            expiryDateMs : 1
        };

        it("is defined", function() {
            expect(MulticastSubscriptionRequest).toBeDefined();
        });

        it("is instantiable", function() {
            var multicastSubscriptionRequest = new MulticastSubscriptionRequest({
                multicastId : "multicastId",
                subscribedToName : "multicastName",
                subscriptionId : "testSubscriptionId",
                qos : new MulticastSubscriptionQos(qosSettings)
            });
            expect(multicastSubscriptionRequest).toBeDefined();
            expect(multicastSubscriptionRequest).not.toBeNull();
            expect(typeof multicastSubscriptionRequest === "object").toBeTruthy();
            expect(multicastSubscriptionRequest instanceof MulticastSubscriptionRequest)
                    .toBeTruthy();
        });

        it("handles missing parameters correctly", function() {
            // does not throw, with qos
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscribedToName : "multicastName",
                    subscriptionId : "testSubscriptionId",
                    qos : new MulticastSubscriptionQos(qosSettings)
                });
            }).not.toThrow();

            // does not throw, without qos
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscribedToName : "multicastName",
                    subscriptionId : "testSubscriptionId"
                });
            }).not.toThrow();

            // throws on wrongly typed subscribedToName
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscribedToName : {},
                    subscriptionId : "testSubscriptionId",
                    qos : new MulticastSubscriptionQos(qosSettings)
                });
            }).toThrow();

            // throws on missing subscribedToName
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscriptionId : "testSubscriptionId",
                    qos : new MulticastSubscriptionQos(qosSettings)
                });
            }).toThrow();

            // throws on missing multicastId
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    subscriptionId : "subscriptionId",
                    subscribedToName : "subscribedToName",
                    qos : new MulticastSubscriptionQos(qosSettings)
                });
            }).toThrow();

            // throws on missing subscriptionId
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscribedToName : "subscribedToName",
                    qos : new MulticastSubscriptionQos(qosSettings)
                });
            }).toThrow();

            // throws on missing settings object type
            expect(function() {
                var subReq = new MulticastSubscriptionRequest();
            }).toThrow();

            // throws on wrong settings object type
            expect(function() {
                var subReq = new MulticastSubscriptionRequest("wrong type");
            }).toThrow();
            // throws on incorrect qos
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscribedToName : "attributeName",
                    subscriptionId : "testSubscriptionId",
                    qos : 1000
                });
            }).toThrow();

            // does not throw if old OnChangeSubscriptonQos is passed
            expect(function() {
                var subReq = new MulticastSubscriptionRequest({
                    multicastId : "multicastId",
                    subscribedToName : "attributeName",
                    subscriptionId : "testSubscriptionId",
                    qos : new OnChangeSubscriptionQos(qosSettings)
                });
            }).not.toThrow();
        });

        it("is constructs with correct member values", function() {
            var multicastId = "multicastId";
            var subscribedToName = "subscribedToName";
            var subscriptionQos = new MulticastSubscriptionQos(qosSettings);
            var subscriptionId = "testSubscriptionId";

            var subscriptionRequest = new MulticastSubscriptionRequest({
                multicastId : "multicastId",
                subscribedToName : subscribedToName,
                qos : subscriptionQos,
                subscriptionId : subscriptionId
            });

            expect(subscriptionRequest.multicastId).toEqual(multicastId);
            expect(subscriptionRequest.subscribedToName).toEqual(subscribedToName);
            expect(subscriptionRequest.qos).toEqual(subscriptionQos);
            expect(subscriptionRequest.subscriptionId).toEqual(subscriptionId);
        });

    });

}); // require
