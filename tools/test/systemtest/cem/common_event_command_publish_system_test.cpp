/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "datetime_ex.h"
#include "common_event_command.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "tool_system_test.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace {
const std::string STRING_EVENT = "com.ces.event";
const std::string STRING_CODE = "1024";
const std::string STRING_CODE_TWO = "2048";
const std::string STRING_DATA = "data";
const std::string STRING_DATA_TWO = "data_two";
const std::string STRING_DEVICE_ID_001 = "device_001";
const std::string STRING_DEVICE_ID_002 = "device_002";
const std::string STRING_DEVICE_ID_003 = "device_003";
const time_t TIME_OUT_SECONDS_LIMIT = 5;
std::mutex mtx;
}  // namespace

class CemCommandPublishSystemTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CemCommandPublishSystemTest::SetUpTestCase()
{}

void CemCommandPublishSystemTest::TearDownTestCase()
{}

void CemCommandPublishSystemTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void CemCommandPublishSystemTest::TearDown()
{}

class CommonEventSubscriberTest : public CommonEventSubscriber {
public:
    explicit CommonEventSubscriberTest(const CommonEventSubscribeInfo &subscribeInfo)
        : CommonEventSubscriber(subscribeInfo)
    {}

    ~CommonEventSubscriberTest()
    {}

    void OnReceiveEvent(const CommonEventData &commonEventData)
    {
        GTEST_LOG_(INFO) << "OnReceiveEvent";

        std::string deviceId = GetSubscribeInfo().GetDeviceId();
        GTEST_LOG_(INFO) << "deviceId = " << deviceId;

        int code = commonEventData.GetCode();
        GTEST_LOG_(INFO) << "code = " << code;

        std::string data = commonEventData.GetData();
        GTEST_LOG_(INFO) << "data = " << data;

        if (deviceId == STRING_DEVICE_ID_001) {
            if (atoi(STRING_CODE.c_str()) == code && STRING_DATA == data) {
                mtx.unlock();
            }
        } else if (deviceId == STRING_DEVICE_ID_002) {
            if (atoi(STRING_CODE_TWO.c_str()) == code) {
                mtx.unlock();
            }
        } else if (deviceId == STRING_DEVICE_ID_003) {
            if (STRING_DATA_TWO == data) {
                mtx.unlock();
            }
        } else {
            mtx.unlock();
        }
    }
};

/**
 * @tc.number: Cem_Command_Publish_SystemTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name>" command.
 */
HWTEST_F(CemCommandPublishSystemTest, Cem_Command_Publish_SystemTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subcribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make a subcriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    // lock the mutex
    mtx.lock();

    // publish a common event
    std::string command = "cem publish -e " + STRING_EVENT;
    std::string commandResult = ToolSystemTest::ExecuteCommand(command);

    EXPECT_EQ(commandResult, STRING_PUBLISH_COMMON_EVENT_OK + "\n");

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    // unsubscribe the common event
    CommonEventManager::UnSubscribeCommonEvent(subscriberTestPtr);

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);

    // unlock the mutex
    mtx.unlock();
}

/**
 * @tc.number: Cem_Command_Publish_SystemTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -c <code> -d <data>" command.
 */
HWTEST_F(CemCommandPublishSystemTest, Cem_Command_Publish_SystemTest_0200, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subcribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // set device id
    subscribeInfo.SetDeviceId(STRING_DEVICE_ID_001);

    // make a subcriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    // lock the mutex
    mtx.lock();

    // publish a common event with code and data
    std::string command = "cem publish -e " + STRING_EVENT + " -c " + STRING_CODE + " -d " + STRING_DATA;
    std::string commandResult = ToolSystemTest::ExecuteCommand(command);

    EXPECT_EQ(commandResult, STRING_PUBLISH_COMMON_EVENT_OK + "\n");

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    // unsubscribe the common event
    CommonEventManager::UnSubscribeCommonEvent(subscriberTestPtr);

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);

    // unlock the mutex
    mtx.unlock();
}

/**
 * @tc.number: Cem_Command_Publish_SystemTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -c <code>" command.
 */
HWTEST_F(CemCommandPublishSystemTest, Cem_Command_Publish_SystemTest_0300, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subcribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // set device id
    subscribeInfo.SetDeviceId(STRING_DEVICE_ID_002);

    // make a subcriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    // lock the mutex
    mtx.lock();

    // publish a common event with code and data
    std::string command = "cem publish -e " + STRING_EVENT + " -c " + STRING_CODE_TWO;
    std::string commandResult = ToolSystemTest::ExecuteCommand(command);

    EXPECT_EQ(commandResult, STRING_PUBLISH_COMMON_EVENT_OK + "\n");

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    // unsubscribe the common event
    CommonEventManager::UnSubscribeCommonEvent(subscriberTestPtr);

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);

    // unlock the mutex
    mtx.unlock();
}

/**
 * @tc.number: Cem_Command_Publish_SystemTest_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -d <data>" command.
 */
HWTEST_F(CemCommandPublishSystemTest, Cem_Command_Publish_SystemTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subcribe info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // set device id
    subscribeInfo.SetDeviceId(STRING_DEVICE_ID_003);

    // make a subcriber object
    auto subscriberTestPtr = std::make_shared<CommonEventSubscriberTest>(subscribeInfo);
    // subscribe a common event
    CommonEventManager::SubscribeCommonEvent(subscriberTestPtr);

    // lock the mutex
    mtx.lock();

    // publish a common event with code and data
    std::string command = "cem publish -e " + STRING_EVENT + " -d " + STRING_DATA_TWO;
    std::string commandResult = ToolSystemTest::ExecuteCommand(command);

    EXPECT_EQ(commandResult, STRING_PUBLISH_COMMON_EVENT_OK + "\n");

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    // unsubscribe the common event
    CommonEventManager::UnSubscribeCommonEvent(subscriberTestPtr);

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);

    // unlock the mutex
    mtx.unlock();
}
