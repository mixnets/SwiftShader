// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Vulkan unit tests that provide coverage for functionality not tested by
// the dEQP test suite. Also used as a smoke test.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Yarn/Pool.hpp"
#include "Yarn/Scheduler.hpp"
#include "Yarn/TicketQueue.hpp"
#include "Yarn/WaitGroup.hpp"

#include <mutex>
#include <condition_variable>

class YarnTests : public testing::Test {};

TEST(YarnTests, SchedulerConstructAndDestruct)
{
    auto scheduler = new yarn::Scheduler();
    delete scheduler;
}

TEST(YarnTests, SchedulerBindGetUnbind)
{
    auto scheduler = new yarn::Scheduler();
    scheduler->bind();
    auto got = yarn::Scheduler::get();
    ASSERT_EQ(scheduler, got);
    scheduler->unbind();
    got = yarn::Scheduler::get();
    ASSERT_EQ(got, nullptr);
    delete scheduler;
}

TEST(YarnTests, WaitGroupDone)
{
    yarn::WaitGroup wg(2); // Should not require a scheduler.
    wg.done();
    wg.done();
}

TEST(YarnTests, WaitGroupDoneTooMany)
{
    yarn::WaitGroup wg(2); // Should not require a scheduler.
    wg.done();
    wg.done();
    EXPECT_DEATH(wg.done(), "done\\(\\) called too many times");
}

/*
TEST(YarnTests, WaitGroupWaitRequiresScheduler)
{
    yarn::WaitGroup wg(1);
    EXPECT_DEATH(wg.wait(), "yarn::WaitGroup::wait\\(\\) requires a yarn::Scheduler to be bound");
}
*/

struct SchedulerParams
{
    int numWorkerThreads;

    friend std::ostream& operator<<(std::ostream& os, const SchedulerParams& params) {
        return os << "SchedulerParams{" <<
            "numWorkerThreads: " << params.numWorkerThreads <<
            "}";
    }
};

class SchedulerTests : public testing::TestWithParam<SchedulerParams>
{
public:
    void SetUp() override
    {
        auto &params = GetParam();

        auto scheduler = new yarn::Scheduler();
        scheduler->bind();
        scheduler->setWorkerThreadCount(params.numWorkerThreads);
    }

    void TearDown() override
    {
        auto scheduler = yarn::Scheduler::get();
        scheduler->unbind();
        delete scheduler;
    }
};

INSTANTIATE_TEST_SUITE_P(SchedulerParams, SchedulerTests, testing::Values(
    SchedulerParams{0},
    SchedulerParams{1},
    SchedulerParams{2},
    SchedulerParams{4},
    SchedulerParams{8},
    SchedulerParams{64}
));

TEST_P(SchedulerTests, SetAndGetWorkerThreadCount)
{
    ASSERT_EQ(yarn::Scheduler::get()->getWorkerThreadCount(), GetParam().numWorkerThreads);
}

TEST_P(SchedulerTests, DestructWithPendingTasks)
{
    for (int i = 0; i < 10000; i++)
    {
        yarn::schedule([] {});
    }
}

TEST_P(SchedulerTests, DestructWithPendingFibers)
{
    yarn::WaitGroup wg(1);
    for (int i = 0; i < 10000; i++)
    {
        yarn::schedule([=] { wg.wait(); });
    }
    wg.done();

    auto scheduler = yarn::Scheduler::get();
    scheduler->unbind();
    delete scheduler;

    // Rebind a new scheduler so SchedulerTests::TearDown() is happy.
    (new yarn::Scheduler())->bind();
}

TEST_P(SchedulerTests, WaitGroup_OneTask)
{
    yarn::WaitGroup wg(1);
    std::atomic<int> counter = {0};
    yarn::schedule([&counter, wg] {
        counter++;
        wg.done();
    });
    wg.wait();
    ASSERT_EQ(counter.load(), 1);
}

TEST_P(SchedulerTests, WaitGroup_10Tasks)
{
    yarn::WaitGroup wg(10);
    std::atomic<int> counter = {0};
    for (int i = 0; i < 10; i++)
    {
        yarn::schedule([&counter, wg] {
            counter++;
            wg.done();
        });
    }
    wg.wait();
    ASSERT_EQ(counter.load(), 10);
}

TEST_P(SchedulerTests, UnboundedPool_ConstructDestruct)
{
    yarn::UnboundedPool<int> pool;
}

TEST_P(SchedulerTests, FixedSizePool_ConstructDestruct)
{
    yarn::FixedSizePool<int, 10> pool;
}

TEST_P(SchedulerTests, UnboundedPool_Borrow)
{
    yarn::UnboundedPool<int> pool;
    for (int i = 0; i < 100; i++)
    {
        pool.borrow();
    }
}

TEST_P(SchedulerTests, UnboundedPool_ConcurrentBorrow)
{
    yarn::UnboundedPool<int> pool;
    constexpr int iterations = 10000;
    yarn::WaitGroup wg(iterations);
    for (int i = 0; i < iterations; i++) {
        yarn::schedule([=] { pool.borrow(); wg.done(); });
    }
    wg.wait();
}

TEST_P(SchedulerTests, FixedSizePool_Borrow)
{
    yarn::FixedSizePool<int, 100> pool;
    for (int i = 0; i < 100; i++)
    {
        pool.borrow();
    }
}

TEST_P(SchedulerTests, FixedSizePool_ConcurrentBorrow)
{
    yarn::FixedSizePool<int, 10> pool;
    constexpr int iterations = 10000;
    yarn::WaitGroup wg(iterations);
    for (int i = 0; i < iterations; i++) {
        yarn::schedule([=]
        {
            pool.borrow();
            wg.done();
        });
    }
    wg.wait();
}

struct CtorDtorCounter
{
    CtorDtorCounter() { ctor_count++; }
    ~CtorDtorCounter() { dtor_count++; }
    static void reset() { ctor_count = 0; dtor_count = 0; }
    static int ctor_count;
    static int dtor_count;
};

int CtorDtorCounter::ctor_count = -1;
int CtorDtorCounter::dtor_count = -1;

TEST_P(SchedulerTests, UnboundedPool_PolicyReconstruct)
{
    CtorDtorCounter::reset();
    yarn::UnboundedPool<CtorDtorCounter, yarn::PoolPolicy::Reconstruct> pool;
    ASSERT_EQ(CtorDtorCounter::ctor_count, 0);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    {
        auto loan = pool.borrow();
        ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
    {
        auto loan = pool.borrow();
        ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 2);
}

TEST_P(SchedulerTests, FixedSizePool_PolicyReconstruct)
{
    CtorDtorCounter::reset();
    yarn::FixedSizePool<CtorDtorCounter, 10, yarn::PoolPolicy::Reconstruct> pool;
    ASSERT_EQ(CtorDtorCounter::ctor_count, 0);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    {
        auto loan = pool.borrow();
        ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
    {
        auto loan = pool.borrow();
        ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 2);
}

TEST_P(SchedulerTests, UnboundedPool_PolicyPreserve)
{
    CtorDtorCounter::reset();
    {
        yarn::UnboundedPool<CtorDtorCounter, yarn::PoolPolicy::Preserve> pool;
        int ctor_count;
        {
            auto loan = pool.borrow();
            ASSERT_NE(CtorDtorCounter::ctor_count, 0);
            ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
            ctor_count = CtorDtorCounter::ctor_count;
        }
        ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
        {
            auto loan = pool.borrow();
            ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
            ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
        }
        ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, CtorDtorCounter::dtor_count);
}

TEST_P(SchedulerTests, FixedSizePool_PolicyPreserve)
{
    CtorDtorCounter::reset();
    {
        yarn::FixedSizePool<CtorDtorCounter, 10, yarn::PoolPolicy::Preserve> pool;
        int ctor_count;
        {
            auto loan = pool.borrow();
            ASSERT_NE(CtorDtorCounter::ctor_count, 0);
            ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
            ctor_count = CtorDtorCounter::ctor_count;
        }
        ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
        {
            auto loan = pool.borrow();
            ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
            ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
        }
        ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
        ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, CtorDtorCounter::dtor_count);
}

TEST_P(SchedulerTests, TicketQueue)
{
    yarn::Ticket::Queue queue;

    constexpr int count = 1000;
    std::atomic<int> next = {0};
    int result[count] = {};

    for (int i = 0; i < count; i++)
    {
        auto ticket = queue.take();
        yarn::schedule([ticket, i, &result, &next] {
            ticket.wait();
            result[next++] = i;
            ticket.done();
        });
    }

    queue.take().wait();

    for (int i = 0; i < count; i++)
    {
        ASSERT_EQ(result[i], i);
    }
}
