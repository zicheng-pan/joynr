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
require("../../../node-unit-test-helper");
const BrowserMessagingStub = require("../../../../../main/js/joynr/messaging/browser/BrowserMessagingStub");

describe("libjoynr-js.joynr.messaging.browser.BrowserMessagingStub", () => {
    let webMessagingStub, browserMessagingStub, windowId, joynrMessage;

    beforeEach(() => {
        function WebMessagingStub() {}
        webMessagingStub = new WebMessagingStub();
        webMessagingStub.transmit = jasmine.createSpy("transmit");

        browserMessagingStub = new BrowserMessagingStub({
            webMessagingStub
        });

        windowId = "mywindowId";
        function JoynrMessage() {}
        joynrMessage = new JoynrMessage();
    });

    it("is of correct type and has all members", () => {
        expect(BrowserMessagingStub).toBeDefined();
        expect(typeof BrowserMessagingStub === "function").toBeTruthy();
        expect(browserMessagingStub).toBeDefined();
        expect(browserMessagingStub instanceof BrowserMessagingStub).toBeTruthy();
        expect(browserMessagingStub.transmit).toBeDefined();
        expect(typeof browserMessagingStub.transmit === "function").toBeTruthy();
    });

    it("calls correctly webMessagingStub.transmit correctly", () => {
        browserMessagingStub.transmit(joynrMessage);
        expect(webMessagingStub.transmit).toHaveBeenCalledWith({
            windowId: undefined,
            message: joynrMessage
        });
    });

    it("calls correctly webMessagingStub.transmit with windowId correctly", () => {
        browserMessagingStub = new BrowserMessagingStub({
            windowId,
            webMessagingStub
        });

        browserMessagingStub.transmit(joynrMessage);
        expect(webMessagingStub.transmit).toHaveBeenCalledWith({
            windowId,
            message: joynrMessage
        });
    });
});
