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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "joynr/Future.h"
#include "tests/utils/MockObjects.h"

using ::testing::Property;
using ::testing::Eq;
using ::testing::ByRef;
using ::testing::NotNull;
using ::testing::_;
using namespace ::testing;
using namespace joynr;

/*
 * This tests the Future class.
 */


class FutureTest : public ::testing::Test {
public:

    FutureTest()
        : intFuture(),
          voidFuture() {
    }

    void SetUp() {
    }

protected:
    Future<int> intFuture;
    Future<void> voidFuture;
};

TEST_F(FutureTest, getValueAndStatusAfterResultReceived) {
    intFuture.onSuccess(10);
    int actualValue;

    // try retrieving the results with timeout
    ASSERT_NO_THROW(intFuture.get(1, actualValue));
    ASSERT_EQ(10, actualValue);
    ASSERT_EQ(RequestStatusCode::OK, intFuture.getStatus().getCode());

    // try retrieving the results a second time with timeout
    ASSERT_NO_THROW(intFuture.get(2, actualValue));
    ASSERT_EQ(10, actualValue);
    ASSERT_EQ(RequestStatusCode::OK, intFuture.getStatus().getCode());

    // try retrieving the results a third time without timeout
    ASSERT_NO_THROW(intFuture.get(actualValue));
    ASSERT_EQ(10, actualValue);
    ASSERT_EQ(RequestStatusCode::OK, intFuture.getStatus().getCode());
}

TEST_F(FutureTest, isOKReturnsTrueWhenStatusIsOk) {
    ASSERT_FALSE(intFuture.isOk());
    intFuture.onSuccess(10);
    ASSERT_TRUE(intFuture.isOk());
}

TEST_F(FutureTest, getStatusAndErrorAfterFailiureReceived) {
    intFuture.onError(RequestStatus(RequestStatusCode::ERROR), exceptions::ProviderRuntimeException("exceptionMessageIntFuture"));
    ASSERT_EQ(RequestStatusCode::ERROR, intFuture.getStatus().getCode());
    int actualValue;

    // get error without timeout
    try {
        intFuture.get(actualValue);
        ADD_FAILURE()<< "expected ProviderRuntimeException";
    } catch (exceptions::ProviderRuntimeException& e) {
        ASSERT_EQ(e.getMessage(), "exceptionMessageIntFuture");
    }

    // get error with timeout
    try {
        intFuture.get(1, actualValue);
        ADD_FAILURE()<< "expected ProviderRuntimeException";
    } catch (exceptions::ProviderRuntimeException& e) {
        ASSERT_EQ(e.getMessage(), "exceptionMessageIntFuture");
    }
}

TEST_F(FutureTest, getValueAndStatusBeforeOperationFinishes) {
    ASSERT_EQ(RequestStatusCode::IN_PROGRESS, intFuture.getStatus().getCode());

    int value;
    try {
        intFuture.get(1, value);
        ADD_FAILURE()<< "expected JoynrTimeOutException";
    } catch (exceptions::JoynrTimeOutException& e) {
        ASSERT_EQ(e.getMessage(), "Request did not finish in time");
    }
}

TEST_F(FutureTest, getValueAndStatusForVoidAfterResultReceived) {
    voidFuture.onSuccess();
    ASSERT_EQ(RequestStatusCode::OK, voidFuture.getStatus().getCode());

    // try retrieving the results with timeout
    ASSERT_NO_THROW(voidFuture.get(1));
    ASSERT_EQ(RequestStatusCode::OK, voidFuture.getStatus().getCode());

    // try retrieving the results a second time with timeout
    ASSERT_NO_THROW(voidFuture.get(2));
    ASSERT_EQ(RequestStatusCode::OK, voidFuture.getStatus().getCode());

    // try retrieving the results a third time without timeout
    ASSERT_NO_THROW(voidFuture.get());
    ASSERT_EQ(RequestStatusCode::OK, voidFuture.getStatus().getCode());
}

TEST_F(FutureTest, getStatusAndErrorForVoidAfterFailureReceived) {
    voidFuture.onError(RequestStatus(RequestStatusCode::ERROR), exceptions::ProviderRuntimeException("exceptionMessageVoidFuture"));
    ASSERT_EQ(RequestStatusCode::ERROR, voidFuture.getStatus().getCode());

    // get error without timeout
    try {
        voidFuture.get();
        ADD_FAILURE()<< "expected ProviderRuntimeException";
    } catch (exceptions::ProviderRuntimeException& e) {
        ASSERT_EQ(e.getMessage(), "exceptionMessageVoidFuture");
    }

    // get error with timeout
    try {
        voidFuture.get(1);
        ADD_FAILURE()<< "expected ProviderRuntimeException";
    } catch (exceptions::ProviderRuntimeException& e) {
        ASSERT_EQ(e.getMessage(), "exceptionMessageVoidFuture");
    }
}

TEST_F(FutureTest, getValueAndStatusForVoidBeforeOperationFinishes) {
    ASSERT_EQ(RequestStatusCode::IN_PROGRESS, voidFuture.getStatus().getCode());

    try {
            voidFuture.get(1);
            ADD_FAILURE()<< "expected JoynrTimeOutException";
        } catch (exceptions::JoynrTimeOutException& e) {
            ASSERT_EQ(e.getMessage(), "Request did not finish in time");
        }
}

TEST_F(FutureTest, waitForFinishWithTimer) {
    try {
        intFuture.wait(5);
        FAIL()<< "expected JoynrTimeOutException";
    } catch (exceptions::JoynrTimeOutException& e) {
        RequestStatus requestStatus = intFuture.getStatus();
        EXPECT_EQ(RequestStatusCode::IN_PROGRESS, requestStatus.getCode());
    }
}

TEST_F(FutureTest, waitForFinishWithTimerForVoid) {
    try {
        voidFuture.wait(5);
        FAIL()<< "expected JoynrTimeOutException";
    } catch (exceptions::JoynrTimeOutException& e) {
        RequestStatus requestStatus = voidFuture.getStatus();
        EXPECT_EQ(RequestStatusCode::IN_PROGRESS, requestStatus.getCode());
    }
}
