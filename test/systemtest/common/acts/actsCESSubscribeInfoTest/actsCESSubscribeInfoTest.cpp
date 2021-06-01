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
#include "common_event_subscribe_info.h"

#include "testConfigParser.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
const int32_t PRIORITYTEST1 = 1;
const int32_t PRIORITYTEST2 = 2;
const int32_t PRIORITYTEST3 = 0;
const int32_t PRIORITYTEST4 = -1;
const int32_t PRIORITYTEST5 = -2;

const std::string PERMISSIONTEST1 = "permissiontest1";
const std::string PERMISSIONTEST2 = "permissiontest2";
const std::string PERMISSIONTEST3 = "permissiontest3";
const std::string PERMISSIONTEST4 = "permissiontest4";
const std::string PERMISSIONTEST5 = "permissiontest5";

const std::string DEVICEIDTEST1 = "deviceIdtest1";
const std::string DEVICEIDTEST2 = "deviceIdtest2";
const std::string DEVICEIDTEST3 = "deviceIdtest3";
const std::string DEVICEIDTEST4 = "deviceIdtest4";
const std::string DEVICEIDTEST5 = "deviceIdtest5";

}  // namespace
class ActsCESSubscribeInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static StressTestLevel stLevel_;
};
StressTestLevel ActsCESSubscribeInfoTest::stLevel_{};

void ActsCESSubscribeInfoTest::SetUpTestCase()
{
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
}

void ActsCESSubscribeInfoTest::TearDownTestCase()
{}

void ActsCESSubscribeInfoTest::SetUp()
{}

void ActsCESSubscribeInfoTest::TearDown()
{}

/*
 * @tc.number: ACTS_SubscribeInfo_Priority_TC_0100
 * @tc.name: SetPriority and GetPriority
 * @tc.desc: Verify the function of  CommonEventSubscribeInfo: SetPriority and GetPriority
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Priority_TC_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "setPriorityTest1";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(PRIORITYTEST1);
        if (subscribeInfo.GetPriority() != PRIORITYTEST1) {
            printf("ACTS_SubscribeInfo_Priority_TC_0100 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Priority_TC_0100  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Priority_TC_0200
 * @tc.name: SetPriority and GetPriority
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by another event name: SetPriority and GetPriority
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Priority_TC_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "priorityTest2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(PRIORITYTEST2);
        if (subscribeInfo.GetPriority() != PRIORITYTEST2) {
            printf("ACTS_SubscribeInfo_Priority_TC_0200 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Priority_TC_0200  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Priority_TC_0300
 * @tc.name: SetPriority and GetPriority
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetPriority and GetPriority
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Priority_TC_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "priorityTest3";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(PRIORITYTEST3);
        if (subscribeInfo.GetPriority() != PRIORITYTEST3) {
            printf("ACTS_SubscribeInfo_Priority_TC_0300 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Priority_TC_0300  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Priority_TC_0400
 * @tc.name: SetPriority and GetPriority
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetPriority and GetPriority
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Priority_TC_0400, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "priorityTest4";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(PRIORITYTEST4);
        if (subscribeInfo.GetPriority() != PRIORITYTEST4) {
            printf("ACTS_SubscribeInfo_Priority_TC_0400 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Priority_TC_0400  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Priority_TC_0500
 * @tc.name: SetPriority and GetPriority
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetPriority and GetPriority
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Priority_TC_0500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "priorityTest5";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(PRIORITYTEST5);
        if (subscribeInfo.GetPriority() != PRIORITYTEST5) {
            printf("ACTS_SubscribeInfo_Priority_TC_0500 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Priority_TC_0500  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Permission_TC_0100
 * @tc.name: SetPermission and GetPermission
 * @tc.desc: Verify the function of CommonEventSubscribeInfo: SetPermission and GetPermission
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Permission_TC_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "permissionTEST1";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(PERMISSIONTEST1);
        if (subscribeInfo.GetPermission() != PERMISSIONTEST1) {
            printf("ACTS_SubscribeInfo_Permission_TC_0100 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Permission_TC_0100  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Permission_TC_0200
 * @tc.name: SetPermission and GetPermission
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by another event name: SetPermission and
 *                      GetPermission
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Permission_TC_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "permissionTEST2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(PERMISSIONTEST2);
        if (subscribeInfo.GetPermission() != PERMISSIONTEST2) {
            printf("ACTS_SubscribeInfo_Permission_TC_0200 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Permission_TC_0200  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Permission_TC_0300
 * @tc.name: SetPermission and GetPermission
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetPermission and GetPermission
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Permission_TC_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "permissionTEST3";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(PERMISSIONTEST3);
        if (subscribeInfo.GetPermission() != PERMISSIONTEST3) {
            printf("ACTS_SubscribeInfo_Permission_TC_0300 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Permission_TC_0300  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Permission_TC_0400
 * @tc.name: SetPermission and GetPermission
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetPermission and GetPermission
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Permission_TC_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "permissionTEST4";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(PERMISSIONTEST4);
        if (subscribeInfo.GetPermission() != PERMISSIONTEST4) {
            printf("ACTS_SubscribeInfo_Permission_TC_0400 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Permission_TC_0400  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_Permission_TC_0500
 * @tc.name: SetPermission and GetPermission
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetPermission and GetPermission
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_Permission_TC_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "permissionTEST5";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(PERMISSIONTEST5);
        if (subscribeInfo.GetPermission() != PERMISSIONTEST5) {
            printf("ACTS_SubscribeInfo_Permission_TC_0500 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_Permission_TC_0500  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_ThreadMode_TC_0100
 * @tc.name: SetThreadMode and GetThreadMode of ThreadMode::HANDLER
 * @tc.desc:  Verify the function of CommonEventSubscribeInfo: SetThreadMode and GetThreadMode of
 *                       ThreadMode::HANDLER
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_ThreadMode_TC_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "threadModeTESTHANDLER";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        if (subscribeInfo.GetThreadMode() != CommonEventSubscribeInfo::ThreadMode::HANDLER) {
            printf("ACTS_SubscribeInfo_ThreadMode_TC_0100 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_ThreadMode_TC_0100  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_ThreadMode_TC_0200
 * @tc.name: SetThreadMode and GetThreadMode of ThreadMode::POST
 * @tc.desc: Verify the function of CommonEventSubscribeInfo: SetThreadMode and GetThreadMode of ThreadMode::POST
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_ThreadMode_TC_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "threadModeTESTPOST";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::POST);
        if (subscribeInfo.GetThreadMode() != CommonEventSubscribeInfo::ThreadMode::POST) {
            printf("ACTS_SubscribeInfo_ThreadMode_TC_0200 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_ThreadMode_TC_0200  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_ThreadMode_TC_0300
 * @tc.name: SetThreadMode and GetThreadMode of ThreadMode::ASYNC
 * @tc.desc: Verify the function of CommonEventSubscribeInfo: SetThreadMode and GetThreadMode of
 *                      ThreadMode::ASYNC
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_ThreadMode_TC_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "threadModeTESTASYNC";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::ASYNC);
        if (subscribeInfo.GetThreadMode() != CommonEventSubscribeInfo::ThreadMode::ASYNC) {
            printf("ACTS_SubscribeInfo_ThreadMode_TC_0300 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_ThreadMode_TC_0300  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_ThreadMode_TC_0400
 * @tc.name: SetThreadMode and GetThreadMode of ThreadMode::BACKGROUND
 * @tc.desc: Verify the function of CommonEventSubscribeInfo: SetThreadMode and GetThreadMode of
 *                      ThreadMode::BACKGROUND
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_ThreadMode_TC_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "threadModeTESTBACKGROUND";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::BACKGROUND);
        if (subscribeInfo.GetThreadMode() != CommonEventSubscribeInfo::ThreadMode::BACKGROUND) {
            printf("ACTS_SubscribeInfo_ThreadMode_TC_0400 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_ThreadMode_TC_0400  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_DeviceId_TC_0100
 * @tc.name: SetDeviceId and GetDeviceId
 * @tc.desc: Verify the function of CommonEventSubscribeInfo: SetDeviceId and GetDeviceId
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_DeviceId_TC_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "deviceIdTEST1";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetDeviceId(DEVICEIDTEST1);
        if (subscribeInfo.GetDeviceId() != DEVICEIDTEST1) {
            printf("ACTS_SubscribeInfo_DeviceId_TC_0100 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_DeviceId_TC_0100  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_DeviceId_TC_0200
 * @tc.name: SetDeviceId and GetDeviceId
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetDeviceId and GetDeviceId
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_DeviceId_TC_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "deviceIdTEST2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetDeviceId(DEVICEIDTEST2);
        if (subscribeInfo.GetDeviceId() != DEVICEIDTEST2) {
            printf("ACTS_SubscribeInfo_DeviceId_TC_0200 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_DeviceId_TC_0200  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_DeviceId_TC_0300
 * @tc.name: SetDeviceId and GetDeviceId
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetDeviceId and GetDeviceId
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_DeviceId_TC_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "deviceIdTEST3";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetDeviceId(DEVICEIDTEST3);
        if (subscribeInfo.GetDeviceId() != DEVICEIDTEST3) {
            printf("ACTS_SubscribeInfo_DeviceId_TC_0300 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_DeviceId_TC_0300  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_DeviceId_TC_0400
 * @tc.name: SetDeviceId and GetDeviceId
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetDeviceId and GetDeviceId
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_DeviceId_TC_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "deviceIdTEST4";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetDeviceId(DEVICEIDTEST4);
        if (subscribeInfo.GetDeviceId() != DEVICEIDTEST4) {
            printf("ACTS_SubscribeInfo_DeviceId_TC_0400 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_DeviceId_TC_0400  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_DeviceId_TC_0500
 * @tc.name: SetDeviceId and GetDeviceId
 * @tc.desc: Verify the function of CommonEventSubscribeInfo by other event name: SetDeviceId and GetDeviceId
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_DeviceId_TC_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "deviceIdTEST5";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetDeviceId(DEVICEIDTEST5);
        if (subscribeInfo.GetDeviceId() != DEVICEIDTEST5) {
            printf("ACTS_SubscribeInfo_DeviceId_TC_0500 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_DeviceId_TC_0500  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: ACTS_SubscribeInfo_GetMatchingSkills_TC_0100
 * @tc.name: GetMatchingSkills
 * @tc.desc: Verify the function of CommonEventSubscribeInfo: GetMatchingSkills
 */
HWTEST_F(ActsCESSubscribeInfoTest, ACTS_SubscribeInfo_GetMatchingSkills_TC_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "getMatchingSkills";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        if (subscribeInfo.GetMatchingSkills().HasEvent(eventName) == false) {
            printf("ACTS_SubscribeInfo_GetMatchingSkills_TC_0100 faild ,this is %d round\n", i);
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        printf("ACTS_SubscribeInfo_GetMatchingSkills_TC_0100  : %d\n", stLevel_.CESLevel);
    }
    EXPECT_TRUE(result);
}

}  // namespace EventFwk
}  // namespace OHOS