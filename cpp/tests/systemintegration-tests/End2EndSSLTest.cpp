/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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
#include "joynr/PrivateCopyAssign.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/utils/MockObjects.h"

#include "runtimes/cluster-controller-runtime/JoynrClusterControllerRuntime.h"
#include "joynr/MessagingSettings.h"
#include "joynr/SettingsMerger.h"
#include "tests/utils/MockObjects.h"
#include "joynr/vehicle/GpsProxy.h"
#include "joynr/types/ProviderQos.h"
#include "joynr/RequestStatus.h"
#include "joynr/Future.h"
#include "joynr/Util.h"

using namespace ::testing;
using namespace joynr;

class End2EndSSLTest : public Test{
public:
    QString domain;
    JoynrClusterControllerRuntime* runtime;

    End2EndSSLTest() :
        domain(),
        runtime(NULL)
    {
        QSettings* settings = SettingsMerger::mergeSettings(QString("test-resources/integrationtest.settings"));
        SettingsMerger::mergeSettings(QString("test-resources/sslintegrationtest.settings"), settings);
        SettingsMerger::mergeSettings(QString("test-resources/libjoynrintegrationtest.settings"), settings);
        runtime = new JoynrClusterControllerRuntime(NULL, settings);
        QString uuid = Util::createUuid();
        domain = "cppEnd2EndSSLTest_Domain_" + uuid;
    }

    // Sets up the test fixture.
    void SetUp(){
       runtime->start();
    }

    // Tears down the test fixture.
    void TearDown(){
        bool deleteChannel = true;
        runtime->stop(deleteChannel);

        // Remove participant id persistence file
        QFile::remove(LibjoynrSettings::DEFAULT_PARTICIPANT_IDS_PERSISTENCE_FILENAME());
        QThreadSleep::msleep(550);
    }

    ~End2EndSSLTest(){
        delete runtime;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(End2EndSSLTest);
};

TEST_F(End2EndSSLTest, call_rpc_method_and_get_expected_result)
{

    // Create a provider
    QSharedPointer<MockGpsProvider> mockProvider(new MockGpsProvider());
    runtime->registerCapability<vehicle::GpsProvider>(domain, mockProvider, QString());
    QThreadSleep::msleep(550);

    // Build a proxy
    ProxyBuilder<vehicle::GpsProxy>* gpsProxyBuilder = runtime->getProxyBuilder<vehicle::GpsProxy>(domain);
    DiscoveryQos discoveryQos;
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY);
    discoveryQos.setDiscoveryTimeout(1000);

    qlonglong qosRoundTripTTL = 40000;
    QSharedPointer<vehicle::GpsProxy> gpsProxy(gpsProxyBuilder
            ->setRuntimeQos(MessagingQos(qosRoundTripTTL))
            ->setCached(false)
            ->setDiscoveryQos(discoveryQos)
            ->build());

    // Call the provider and wait for a result
    QSharedPointer<Future<int> >gpsFuture (gpsProxy->calculateAvailableSatellites());
    gpsFuture->waitForFinished();

    int expectedValue = 42; //as defined in MockGpsProvider
    EXPECT_EQ(expectedValue, gpsFuture->getValue());
    delete gpsProxyBuilder;
}
