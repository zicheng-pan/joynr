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

#ifndef PERFORMANCECONSUMER_H
#define PERFORMANCECONSUMER_H

#include <condition_variable>
#include <mutex>
#include <chrono>
#include <limits>
#include <exception>

#include "joynr/JoynrRuntime.h"
#include "joynr/DiscoveryQos.h"
#include "joynr/ProxyBuilder.h"

#include "joynr/tests/performance/EchoProxy.h"

namespace joynr
{

struct IPerformanceConsumer
{
    virtual void runByteArray() = 0;
    virtual void runString() = 0;
    virtual void runStruct() = 0;
};

template <typename Impl>
class PerformanceConsumer : public IPerformanceConsumer
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using ClockResolution = std::chrono::microseconds;
    using EchoProxy = joynr::tests::performance::EchoProxy;
    using ByteArray = std::vector<std::int8_t>;
    using ComplexStruct = joynr::tests::performance::Types::ComplexStruct;

public:
    PerformanceConsumer(std::unique_ptr<JoynrRuntime> joynrRuntime,
                        std::size_t messageCount,
                        std::size_t stringLength,
                        std::size_t byteArraySize,
                        const std::string& domain)
            : runtime(std::move(joynrRuntime)),
              messageCount(messageCount),
              stringLength(stringLength),
              byteArraySize(byteArraySize),
              durationVector(messageCount)
    {
        if (messageCount == 0) {
            throw std::invalid_argument("messageCount must be >= 1");
        }
        std::unique_ptr<ProxyBuilder<EchoProxy>> proxyBuilder(
                runtime->createProxyBuilder<EchoProxy>(domain));

        if (!proxyBuilder) {
            std::cerr << "Failed to create Echo proxy builder" << std::endl;
            exit(EXIT_FAILURE);
        }

        DiscoveryQos discoveryQos;
        discoveryQos.setCacheMaxAge(std::numeric_limits<std::int64_t>::max());
        discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY);

        try {
            // Build a proxy
            echoProxy.reset(proxyBuilder->setMessagingQos(MessagingQos(ttl))
                                    ->setCached(false)
                                    ->setDiscoveryQos(discoveryQos)
                                    ->build());
        } catch (const std::exception& e) {
            std::cerr << "Failed to build Echo proxy: " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void runByteArray() override
    {
        run(&Impl::loopByteArray, getFilledByteArray());
    }

    void runString() override
    {
        run(&Impl::loopString, getFilledString());
    }

    void runStruct() override
    {
        run(&Impl::loopStruct, getFilledStruct());
    }

    template <typename LoopFun, typename... Args>
    void run(LoopFun fun, Args&&... args)
    {
        const auto startLoop = Clock::now();
        (static_cast<Impl*>(this)->*fun)(std::forward<Args>(args)...);
        const auto endLoop = Clock::now();
        auto totalDuration = std::chrono::duration_cast<ClockResolution>(endLoop - startLoop);
        printStatistics(totalDuration);
    }

protected:
    ComplexStruct getFilledStruct() const
    {
        return ComplexStruct(32, 64, getFilledByteArray(), getFilledString());
    }

    std::string getFilledString() const
    {
        return std::string(stringLength, '#');
    }

    ByteArray getFilledByteArray() const
    {
        ByteArray data(byteArraySize);
        // fill data with sequentially increasing numbers
        std::iota(data.begin(), data.end(), 0);
        return data;
    }

    std::size_t messageCount;
    std::unique_ptr<EchoProxy> echoProxy;
    std::vector<ClockResolution> durationVector;

    std::size_t stringLength;
    std::size_t byteArraySize;

private:
    void printStatistics(ClockResolution totalDuration)
    {
        using DoubleMilliSeconds = std::chrono::duration<double, std::milli>;
        auto maxDelayDuration = std::chrono::duration_cast<DoubleMilliSeconds>(
                *std::max_element(durationVector.cbegin(), durationVector.cend()));
        auto minDelayDuration = std::chrono::duration_cast<DoubleMilliSeconds>(
                *std::min_element(durationVector.cbegin(), durationVector.cend()));
        auto sumDelayDuration = std::chrono::duration_cast<DoubleMilliSeconds>(std::accumulate(
                durationVector.cbegin(), durationVector.cend(), ClockResolution(0)));
        double meanDelay = sumDelayDuration.count() / messageCount;

        using DoubleSeconds = std::chrono::duration<double>;
        auto totalDurationSec = std::chrono::duration_cast<DoubleSeconds>(totalDuration);
        double msgPerSec = messageCount / totalDurationSec.count();

        std::cerr << "----- statistics -----" << std::endl;
        std::cerr << "totalDuration:\t" << totalDurationSec.count() << " [s]" << std::endl;
        std::cerr << "maxDelay:\t\t" << maxDelayDuration.count() << " [ms]" << std::endl;
        std::cerr << "minDelay:\t\t" << minDelayDuration.count() << " [ms]" << std::endl;
        std::cerr << "meanDelay:\t\t" << meanDelay << " [ms]" << std::endl;
        std::cerr << "msg/sec:\t\t" << msgPerSec << std::endl;
    }

    std::unique_ptr<JoynrRuntime> runtime;
    std::uint64_t ttl = 10000;
};

class SyncEchoConsumer : public PerformanceConsumer<SyncEchoConsumer>
{
public:
    using PerformanceConsumer::PerformanceConsumer;

    void loopByteArray(const ByteArray& data)
    {
        ByteArray responseData;
        auto fun = [this, &data, &responseData]() { echoProxy->echoByteArray(responseData, data); };
        loop(fun);
    }

    void loopString(const std::string& data)
    {
        std::string responseData;
        auto fun = [this, &data, &responseData]() { echoProxy->echoString(responseData, data); };
        loop(fun);
    }

    void loopStruct(const ComplexStruct& data)
    {
        ComplexStruct responseData;
        auto fun = [this, &data, &responseData]() {
            echoProxy->echoComplexStruct(responseData, data);
        };
        loop(fun);
    }

private:
    template <typename Fun>
    void loop(Fun&& fun)
    {
        for (std::size_t i = 0; i < messageCount; ++i) {
            const auto start = Clock::now();
            fun();
            const auto end = Clock::now();
            durationVector[i] = std::chrono::duration_cast<ClockResolution>(end - start);
        }
    }
};

class AsyncEchoConsumer : public PerformanceConsumer<AsyncEchoConsumer>
{
public:
    using PerformanceConsumer::PerformanceConsumer;

    void loopByteArray(const ByteArray& data)
    {
        auto fun =
                [this, &data](auto onSuccess) { echoProxy->echoByteArrayAsync(data, onSuccess); };
        loop(fun);
    }

    void loopString(const std::string& data)
    {
        auto fun = [this, &data](auto onSuccess) { echoProxy->echoStringAsync(data, onSuccess); };
        loop(fun);
    }

    void loopStruct(const ComplexStruct& data)
    {
        auto fun = [this, &data](auto onSuccess) {
            echoProxy->echoComplexStructAsync(data, onSuccess);
        };
        loop(fun);
    }

    void notifyDone()
    {
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
        lock.unlock();
        cv.notify_one();
    }

private:
    template <typename Fun>
    void loop(Fun&& fun)
    {
        done = false;
        msgCounterReceived = 0;

        for (uint32_t i = 0; i < messageCount; ++i) {

            const auto start = Clock::now();

            auto onSuccess = [this, i, start](const auto&) {
                const auto end = Clock::now();
                durationVector[i] = std::chrono::duration_cast<ClockResolution>(end - start);
                msgCounterReceived++;
                // check if this is the last message
                if (msgCounterReceived == messageCount) {
                    this->notifyDone();
                }
            };
            fun(onSuccess);
        }

        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return done; });
    }

    std::atomic<uint32_t> msgCounterReceived;
    bool done = false;
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace joynr

#endif // PERFORMANCECONSUMER_H