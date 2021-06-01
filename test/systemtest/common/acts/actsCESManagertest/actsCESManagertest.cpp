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

#include <iostream>
#include <string>

#include "common_event_manager.h"
#define private public
#define protected public
#include "common_event_manager_service.h"
#undef private
#undef protected
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"

#include "testConfigParser.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
std::mutex g_mtx;
const time_t g_TIME_OUT_SECONDS_LIMIT = 3;
const time_t g_TIME_OUT_SECONDS = 3;

const std::string CompareStr = "cesComparesStrForCase";
const std::string CompareStrFalse = "cesComparesStrForCaseFalse";

const int32_t g_CODE_COMPARE1 = 1;
const int32_t g_CODE_COMPARE2 = 200;
int SIGNUMFIRST = 0;
int SIGNUMSECOND = 0;
int SIGNUMTHIRD = 0;
}  // namespace

class CommonEventServicesSystemTest : public CommonEventSubscriber {
public:
    explicit CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesSystemTest(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTest::CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesSystemTest::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " ActsCESManagertest:CommonEventServicesSystemTest:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == CompareStr) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE1);
        ++SIGNUMFIRST;
    }
    g_mtx.unlock();
}

class CommonEventServicesSystemTestSubscriber : public CommonEventSubscriber {
public:
    explicit CommonEventServicesSystemTestSubscriber(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesSystemTestSubscriber(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTestSubscriber::CommonEventServicesSystemTestSubscriber(
    const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesSystemTestSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " ActsCESManagertest:CommonEventServicesSystemTestSubscriber:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == CompareStr) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE1);
        ++SIGNUMSECOND;
    } else if (action == CompareStrFalse) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE2);
        ++SIGNUMTHIRD;
    }
}

class ActsCESManagertest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    bool SubscribeCommonEventTest(const CommonEventSubscribeInfo &subscriberInfo);
    bool UnsubscribeCommonEventTest(const CommonEventSubscribeInfo &subscriberInfo);
    bool PublishCommonEventTest(const std::string &eventName);
    static StressTestLevel stLevel_;
};
StressTestLevel ActsCESManagertest::stLevel_{};

void ActsCESManagertest::SetUpTestCase()
{
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
}

void ActsCESManagertest::TearDownTestCase()
{}

void ActsCESManagertest::SetUp()
{}

void ActsCESManagertest::TearDown()
{}

bool ActsCESManagertest::SubscribeCommonEventTest(const CommonEventSubscribeInfo &subscriberInfo)
{
    bool result = false;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscriberInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    return result;
}

bool ActsCESManagertest::UnsubscribeCommonEventTest(const CommonEventSubscribeInfo &subscriberInfo)
{
    bool result = false;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscriberInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    return result;
}

/*
 * @tc.number: CES_SubscriptionEvent_0100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);

        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}  // namespace EventFwk

/*
 * @tc.number: CES_SubscriptionEvent_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "1";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0300, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0300 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0300 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0400
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0400, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = ".............";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string with \0
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "HELLO\0\0\0WORLD";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0500 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0500 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0600
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered succseefully
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETPRIORITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(100);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0600 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0600 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0700
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: erify that the ordered common event was unsubsribered successfully
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBE_SETPRIORITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(100);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_0700 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        UnsubscribeCommonEventTest(subscribeInfo);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0700 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0800
 * @tc.name: SubscribeCommonEvent
 * @tc.desc:  Verify the same input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0800, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "TESTEVENT1";
    std::string eventName3 = "TESTEVENT1";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = SubscribeCommonEventTest(subscribeInfo1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = SubscribeCommonEventTest(subscribeInfo2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        bool result3 = SubscribeCommonEventTest(subscribeInfo3);

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_0800 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        UnsubscribeCommonEventTest(subscribeInfo1);
        UnsubscribeCommonEventTest(subscribeInfo2);
        UnsubscribeCommonEventTest(subscribeInfo3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0800 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0900
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the same input empty string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0900, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "";
    std::string eventName2 = "";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = SubscribeCommonEventTest(subscribeInfo1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = SubscribeCommonEventTest(subscribeInfo2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        bool result3 = SubscribeCommonEventTest(subscribeInfo3);

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_0900 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        UnsubscribeCommonEventTest(subscribeInfo1);
        UnsubscribeCommonEventTest(subscribeInfo2);
        UnsubscribeCommonEventTest(subscribeInfo3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_0900 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1000
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1000, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "TESTEVENT2";
    std::string eventName3 = "TESTEVENT3";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = SubscribeCommonEventTest(subscribeInfo1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = SubscribeCommonEventTest(subscribeInfo2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        bool result3 = SubscribeCommonEventTest(subscribeInfo3);

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_1000 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        UnsubscribeCommonEventTest(subscribeInfo1);
        UnsubscribeCommonEventTest(subscribeInfo2);
        UnsubscribeCommonEventTest(subscribeInfo3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1000 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc:  Verify the normal input string two times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TEST1";
    std::string eventName2 = "TEST2";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = SubscribeCommonEventTest(subscribeInfo1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = SubscribeCommonEventTest(subscribeInfo2);

        if (!result1 || !result2) {
            result = false;
            printf("CES_SubscriptionEvent_1100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        UnsubscribeCommonEventTest(subscribeInfo1);
        UnsubscribeCommonEventTest(subscribeInfo2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1200, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "1";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = SubscribeCommonEventTest(subscribeInfo1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = SubscribeCommonEventTest(subscribeInfo2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        bool result3 = SubscribeCommonEventTest(subscribeInfo3);

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_1200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        UnsubscribeCommonEventTest(subscribeInfo1);
        UnsubscribeCommonEventTest(subscribeInfo2);
        UnsubscribeCommonEventTest(subscribeInfo3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_1300 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1300 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1400
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_1400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CCES_SubscriptionEvent_1500
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_1500 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1500 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1600
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is quotes
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1600, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "..................";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_1600 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1600 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1700
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string with \0
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1700, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "HELLO\0\0\0WORLD";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEvent_1700 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1700 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1800
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT5";
    std::string eventName3 = "TESTEVENT6";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        if (SubscribeCommonEventTest(subscribeInfo1)) {
            result1 = UnsubscribeCommonEventTest(subscribeInfo1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        if (SubscribeCommonEventTest(subscribeInfo2)) {
            result2 = UnsubscribeCommonEventTest(subscribeInfo2);
        }

        bool result3 = false;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        if (SubscribeCommonEventTest(subscribeInfo3)) {
            result3 = UnsubscribeCommonEventTest(subscribeInfo3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_1800 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1800 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1900
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal input string two times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TEST3";
    std::string eventName2 = "TEST4";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        if (SubscribeCommonEventTest(subscribeInfo1)) {
            result1 = UnsubscribeCommonEventTest(subscribeInfo1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        if (SubscribeCommonEventTest(subscribeInfo2)) {
            result2 = UnsubscribeCommonEventTest(subscribeInfo2);
        }

        if (!result1 || !result2) {
            result = false;
            printf("CES_SubscriptionEvent_1900 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_1900 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_2000
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_2000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT7";
    std::string eventName2 = "3";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        if (SubscribeCommonEventTest(subscribeInfo1)) {
            result1 = UnsubscribeCommonEventTest(subscribeInfo1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        if (SubscribeCommonEventTest(subscribeInfo2)) {
            result2 = UnsubscribeCommonEventTest(subscribeInfo2);
        }

        bool result3 = false;
        MatchingSkills matchingSkills3;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        if (SubscribeCommonEventTest(subscribeInfo3)) {
            result3 = UnsubscribeCommonEventTest(subscribeInfo3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_2000 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_2000 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_2100
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the same input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_2100, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT4";
    std::string eventName3 = "TESTEVENT4";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = false;
        if (SubscribeCommonEventTest(subscribeInfo1)) {
            result1 = UnsubscribeCommonEventTest(subscribeInfo1);
        }

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = false;
        if (SubscribeCommonEventTest(subscribeInfo2)) {
            result2 = UnsubscribeCommonEventTest(subscribeInfo2);
        }

        MatchingSkills matchingSkills3;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        bool result3 = false;
        if (SubscribeCommonEventTest(subscribeInfo3)) {
            result3 = UnsubscribeCommonEventTest(subscribeInfo3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_2100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_2100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_2200
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the same input empty string three time
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_2200, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "";
    std::string eventName2 = "";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        bool result1 = false;
        if (SubscribeCommonEventTest(subscribeInfo1)) {
            result1 = UnsubscribeCommonEventTest(subscribeInfo1);
        }

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        bool result2 = false;
        if (SubscribeCommonEventTest(subscribeInfo2)) {
            result2 = UnsubscribeCommonEventTest(subscribeInfo2);
        }

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        bool result3 = false;
        if (SubscribeCommonEventTest(subscribeInfo3)) {
            result3 = UnsubscribeCommonEventTest(subscribeInfo3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            printf("CES_SubscriptionEvent_2200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEvent_2200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0100
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            printf("CES_SendEvent_0100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ENTITY";
    std::string eventAction = "TESTEVENT_PUBLISH_ENTITY";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            printf("CES_SendEvent_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0300
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME";
    std::string eventAction = "TESTEVENT_PUBLISH_SCHEME";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            printf("CES_SendEvent_0300 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0300 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0400
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme and add entity
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string eventAction = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string scheme = "SETSCHEME";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            printf("CES_SendEvent_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0500
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_0500 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0500 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0600
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        std::string entity = "ADDENTITY";
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_0600 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0600 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_0700 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0700 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0800
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_0800 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0800 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0900
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        std::string scheme = "SETSCHEME";
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_0900 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_0900 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1000
 * @tc.name: SPublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1000, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_1000 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_1000 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1100
 * @tc.name: PublishCommonEvent
 * @tc.desc: VVerify set scheme and add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_1100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_1100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEvent_1200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEvent_1200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0100
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICHY_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICHY_ACTION_INFO_FALSE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0200
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0300
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0300 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0300 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0400
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CCES_SendEventSetViscosity_0500
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0500 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0500 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0600
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0600 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0600 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0700
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0700 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0700 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0800
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SendEventSetViscosity_0800 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0800 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0900
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_GETSTICKY_";
    std::string eventAction = "TESTEVENT_GETSTICKY_";
    std::string eventActionStr = "TESTEVENT_GETSTICKY_Str";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        if (SubscribeCommonEventTest(subscribeInfo) &&
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = true;
        }
        EXPECT_TRUE(result);
        result = false;
        CommonEventData stickyData;
        CommonEventManager::GetStickyCommonEvent(eventAction, stickyData);
        if (eventActionStr == stickyData.GetWant().GetAction()) {
            result = false;
            printf("CES_SendEventSetViscosity_0900 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_0900 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_1000
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: epublish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_1000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "TESTEVENT_GETSTICKY_FALSE";
    std::string eventAction = "TESTEVENT_GETSTICKY_FALSE";
    std::string actionTest = "CHECKTESTACTION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        if (SubscribeCommonEventTest(subscribeInfo) &&
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = true;
        }
        EXPECT_TRUE(result);
        result = false;
        CommonEventData stickyData;
        if (CommonEventManager::GetStickyCommonEvent(actionTest, stickyData)) {
            result = false;
            printf("CES_SendEventSetViscosity_1000 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SendEventSetViscosity_1000 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_ReceiveEvent_0100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    if (SubscribeCommonEventTest(subscribeInfo) && (CommonEventManager::PublishCommonEvent(commonEventData))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, g_TIME_OUT_SECONDS_LIMIT);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_ReceiveEvent_0200
 * @tc.name: OnReceiveEvent
 * @tc.desc: erify the function when add entity
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ENTITY";
    std::string eventAction = "TESTEVENT_RECEIVE_ENTITY";
    std::string entity = "ADDENTITY";
    bool result = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    if (SubscribeCommonEventTest(subscribeInfo) && (CommonEventManager::PublishCommonEvent(commonEventData))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_ReceiveEvent_0300
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION_INFO_FALSE";
    bool stickty = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    if (SubscribeCommonEventTest(subscribeInfo) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, g_TIME_OUT_SECONDS_LIMIT);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_ReceiveEvent_0400
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION_INFO_TRUE";
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    if (SubscribeCommonEventTest(subscribeInfo) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, g_TIME_OUT_SECONDS_LIMIT);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_ReceiveEvent_0500
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_RECEIVE_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    bool result = true;
    bool stickty = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    if (SubscribeCommonEventTest(subscribeInfo) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_ReceiveEvent_0600
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0600, Function | MediumTest | Level1)
{
    std::string eventName = "testEventReceiveEntityInfoFalse";
    std::string eventAction = "testEventReceiveEntityInfoFalse";
    std::string entity = "addEntity";
    bool result = true;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    if (SubscribeCommonEventTest(subscribeInfo) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0100
 * @tc.name: AddEvent
 * @tc.desc: Verify add an event Theme
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTADDTHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEventTheme_0100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEventTheme_0100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0200
 * @tc.name: AddEvent
 * @tc.desc: erify add multiple enent themes
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TESTADDTHEME1";
    std::string eventName2 = "TESTADDTHEME2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName1);
        matchingSkills.AddEvent(eventName2);

        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SubscriptionEventTheme_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName1);
        matchingSkills.RemoveEvent(eventName2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEventTheme_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0300
 * @tc.name: MatchEvent
 * @tc.desc: Verify march a event theme
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_MATCHEVENTTEST";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        Want wantTest;
        wantTest.SetAction(eventName);
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!matchingSkills.Match(wantTest)) {
            result = false;
            printf("CES_SubscriptionEventTheme_0300 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEventTheme_0300 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0400
 * @tc.name: MatchEvent
 * @tc.desc: Verify march other event theme
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0400, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = "TESTMATCHEVENTTOPICAL";
    std::string eventNameCompare = "TESTMATCHEVENTTOPICAL_COMPARE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        Want wantTest;
        wantTest.SetAction(eventNameCompare);
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (matchingSkills.Match(wantTest)) {
            result = false;
            printf("CES_SubscriptionEventTheme_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SubscriptionEventTheme_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1300
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1300, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    bool result = false;
    bool sysResult = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    if (SubscribeCommonEventTest(subscribeInfo)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        g_mtx.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    GTEST_LOG_(INFO) << " CES_TC_053 result =  " << result;
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS) {
            sysResult = true;
            break;
        }
    }
    GTEST_LOG_(INFO) << " CES_TC_053 seconds =  " << seconds;
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_SendEvent_1400
 * @tc.name: PublishCommonEvent
 * @tc.desc:The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1400, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;
    bool result = false;
    bool sysResult = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    if (SubscribeCommonEventTest(subscribeInfo)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        g_mtx.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_SendEvent_1500
 * @tc.name: PublishCommonEvent
 * @tc.desc:The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1500, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    bool result = false;
    bool sysResult = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    if (SubscribeCommonEventTest(subscribeInfo)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        g_mtx.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_SendEvent_1600
 * @tc.name: PublishCommonEvent
 * @tc.desc: publisher cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1600, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED;
    bool result = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    if (SubscribeCommonEventTest(subscribeInfo) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // System events published by ordinary publishers, the publication fails, and the receiver cannot receive it
    EXPECT_FALSE(result);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_SendEvent_1700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Both subscribers subscribe to the event, after the event is published, both the subscribers can receive
 * the event
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1700, Function | MediumTest | Level1)
{
    std::string eventName = CompareStr;
    std::string eventAction = CompareStr;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        commonEventData.SetCode(1);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTestSubscriber>(subscribeInfo);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
            CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        }
        sleep(1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    EXPECT_TRUE(SIGNUMFIRST == stLevel_.CESLevel && SIGNUMSECOND == stLevel_.CESLevel && SIGNUMTHIRD == 0);
    printf("SIGNUMFIRST: %d    SIGNUMSECOND: %d    SIGNUMTHIRD: %d     stLevel_.CESLevel: %d\n",
        SIGNUMFIRST,
        SIGNUMSECOND,
        SIGNUMTHIRD,
        stLevel_.CESLevel);
    SIGNUMFIRST = 0;
    SIGNUMSECOND = 0;
    SIGNUMTHIRD = 0;
}

/*
 * @tc.number: CES_SendEvent_1800
 * @tc.name: PublishCommonEvent
 * @tc.desc: One subscriber subscribe to the event and another subscriber does not subscribe to the event,
 *           after the event is published, subscribed subscriber can receive the event and unsubscribed
 *           subscribe can not receive the event
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1800, Function | MediumTest | Level1)
{
    std::string eventName = CompareStrFalse;
    std::string eventAction = CompareStrFalse;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        commonEventData.SetCode(200);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTestSubscriber>(subscribeInfo);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        }
        sleep(1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    EXPECT_TRUE(SIGNUMFIRST == 0 && SIGNUMSECOND == 0 && SIGNUMTHIRD == stLevel_.CESLevel);
    printf("SIGNUMFIRST: %d    SIGNUMSECOND: %d    SIGNUMTHIRD: %d     stLevel_.CESLevel: %d\n",
        SIGNUMFIRST,
        SIGNUMSECOND,
        SIGNUMTHIRD,
        stLevel_.CESLevel);
    SIGNUMFIRST = 0;
    SIGNUMSECOND = 0;
    SIGNUMTHIRD = 0;
}

/*
 * @tc.number: CES_SetEventAuthority_0100
 * @tc.name: SetPermission
 * @tc.desc: eSet permission for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0200
 * @tc.name: SetPermission and  SetPriority
 * @tc.desc: Set permission and priority for common event subscribers and  verify  successfully subscribe to
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0300
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        subscribeInfo.SetDeviceId(deviceId);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0400
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and verify successfully Unsubscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0500
 * @tc.name: SetPermission and  SetPriority
 * @tc.desc: Set permission and priority for common event subscribers and verify successfully Unsubscribe to
 * common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0500 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0500 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0600
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * Unsubscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        subscribeInfo.SetDeviceId(deviceId);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0600 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0600 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0700
 * @tc.name: OnReceiveEvent
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    std::vector<std::string> permissins;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        permissins.emplace_back(permissin);
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0700 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
        permissins.clear();
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0700 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0800
 * @tc.name: SetSubscriberPermissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully receive common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0800, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION_R";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_PERMISSION_R";
    std::string permissin = "PERMISSION";
    std::vector<std::string> permissins;
    permissins.emplace_back(permissin);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberPermissions(permissins);
    if (SubscribeCommonEventTest(subscribeInfo) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    EXPECT_LT(seconds, g_TIME_OUT_SECONDS_LIMIT);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_SetEventAuthority_0900
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETTHREADMODE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        if (!SubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_0700 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_0700 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1000
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully Unsubscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_1000, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_SETTHREADMODE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!UnsubscribeCommonEventTest(subscribeInfo)) {
            result = false;
            printf("CES_SetEventAuthority_1000 : %d \n", i);
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_1000 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1100
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully publish to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_1100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            printf("CES_SetEventAuthority_1000 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_SetEventAuthority_1000 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1200
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully receive  to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_1200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    if (SubscribeCommonEventTest(subscribeInfo) && (CommonEventManager::PublishCommonEvent(commonEventData))) {
        GTEST_LOG_(INFO) << " CES_TC_070 result";
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = true;
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_TRUE(result);
    g_mtx.unlock();
    UnsubscribeCommonEventTest(subscribeInfo);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0100
 * @tc.name: GetEvent
 * @tc.desc: check to get the added event
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_GETMATCHINGSKILLS";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        MatchingSkills testMatching = subscribeInfo.GetMatchingSkills();
        if (!(testMatching.GetEvent(0) == eventName)) {
            result = false;
            printf("CES_VerifyMatchingSkills_0100 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_VerifyMatchingSkills_0100 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0200
 * @tc.name: GetEntity
 * @tc.desc: check to get the added entity
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_GETENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!(matchingSkills.GetEntity(0) == entity)) {
            result = false;
            printf("CES_VerifyMatchingSkills_0200 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_VerifyMatchingSkills_0200 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0300
 * @tc.name: HasEntity
 * @tc.desc: verify that entity is  in MatchingSkills
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_HASENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!matchingSkills.HasEntity(entity)) {
            result = false;
            printf("CES_VerifyMatchingSkills_0300 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_VerifyMatchingSkills_0300 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0400
 * @tc.name: RemoveEntity
 * @tc.desc: verify that the entity was successfully removed
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_REMOVEENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        matchingSkills.RemoveEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (matchingSkills.HasEntity(entity)) {
            result = false;
            printf("CES_VerifyMatchingSkills_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_VerifyMatchingSkills_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0500
 * @tc.name: CountEntities
 * @tc.desc: verify that count correct number of entities
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_ENTITYCOUNT";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        EXPECT_TRUE(SubscribeCommonEventTest(subscribeInfo));
        if (!(matchingSkills.CountEntities() >= 1)) {
            result = false;
            printf("CES_VerifyMatchingSkills_0400 : %d \n", i);
            break;
        } else {
            result = true;
        }
        UnsubscribeCommonEventTest(subscribeInfo);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("CES_VerifyMatchingSkills_0400 : %d \n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

}  // namespace EventFwk
}  // namespace OHOS