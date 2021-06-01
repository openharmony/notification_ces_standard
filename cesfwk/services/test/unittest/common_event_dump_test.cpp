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

#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager_service.h"
#undef private
#undef protected

#include "inner_common_event_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
const std::string TYPE = "type";
const std::string BUNDLE = "BundleName";
const std::string ABILITY = "AbilityName";
const int PRIORITY = 1;
const int FLAG = 1;
const int CODE = 1;
const std::string DATA = "DATA";

const std::string EVENT2 = "com.ces.test.event2";
const std::string ENTITY2 = "com.ces.test.entity2";
const std::string SCHEME2 = "com.ces.test.scheme2";
const std::string PERMISSION2 = "com.ces.test.permission2";
const std::string DEVICEDID2 = "deviceId2";
const std::string TYPE2 = "type2";
const std::string BUNDLE2 = "BundleName";
const std::string ABILITY2 = "AbilityName";
const int PRIORITY2 = 2;
const int FLAG2 = 2;
const int CODE2 = 2;
const std::string DATA2 = "DATA2";

const std::string EVENT3 = "com.ces.test.event3";
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventDumpTest : public testing::Test {
public:
    CommonEventDumpTest()
    {}
    ~CommonEventDumpTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void dumpInfoCount(const std::vector<std::string> &state, int desSubscribersCount, int desStickyCount)
    {
        int subscribersNum = 0;
        int stickyNum = 0;
        bool subFlag = false;
        for (auto vec : state) {
            GTEST_LOG_(INFO) << vec;
            auto pos = vec.find("Subscribers:");
            if (pos != string::npos) {
                subFlag = true;
            }
            pos = vec.find("Sticky Events:");
            if (pos != string::npos) {
                subFlag = false;
            }
            if (subFlag) {
                subscribersNum++;
            } else {
                stickyNum++;
            }
        }

        EXPECT_EQ(subscribersNum, desSubscribersCount);
        EXPECT_EQ(stickyNum, desStickyCount);
    }
};

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventDumpTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
}

void CommonEventDumpTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

void CommonEventDumpTest::SetUp(void)
{}

void CommonEventDumpTest::TearDown(void)
{}

static void SetMatchingSkills(MatchingSkills& matchingSkills)
{
	matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEvent(EVENT2);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);
    matchingSkills.AddScheme(SCHEME);
    matchingSkills.AddScheme(SCHEME2);
}

static void SetSubscriberInfo1(CommonEventListener *&listener, MatchingSkills& matchingSkills)
{
	CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

	std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
	listener = new CommonEventListener(subscriber);

	OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());
}

static void SetSubscriberInfo2(CommonEventListener *&listener2, MatchingSkills& matchingSkills)
{
	CommonEventSubscribeInfo subscriberInfo2(matchingSkills);
    subscriberInfo2.SetPriority(PRIORITY2);
    subscriberInfo2.SetPermission(PERMISSION2);
    subscriberInfo2.SetDeviceId(DEVICEDID2);
    std::shared_ptr<SubscriberTest> subscriber2 = std::make_shared<SubscriberTest>(subscriberInfo2);
    listener2 = new CommonEventListener(subscriber2);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo2, listener2->AsObject());
}

static void Test0100Publish1()
{
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODE);
    data.SetData(DATA);

    // make publish info
    CommonEventPublishInfo publishInfo;

    publishInfo.SetSticky(false);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);
}

static void Test0100Publish2()
{
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);
    data2.SetCode(CODE2);
    data2.SetData(DATA2);

    // make publish info
    CommonEventPublishInfo publishInfo2;

    publishInfo2.SetSticky(false);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);
}

static void Test0200Publish1()
{
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODE);
    data.SetData(DATA);

    // make publish info
    CommonEventPublishInfo publishInfo;

    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);
}

static void Test0200Publish2()
{
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);
    data2.SetCode(CODE2);
    data2.SetData(DATA2);

    // make publish info
    CommonEventPublishInfo publishInfo2;

    publishInfo2.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);
}


/*
 * @tc.number: CommonEventDumpTest_0100
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
	CommonEventListener *listener = nullptr;
	CommonEventListener *listener2 = nullptr;

    SetMatchingSkills(matchingSkills);

    SetSubscriberInfo1(listener, matchingSkills);

    SetSubscriberInfo2(listener2, matchingSkills);

    /* Publish */
    Test0100Publish1();

    Test0100Publish2();

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 2, 1);
    EXPECT_EQ("Sticky Events:\tNo information", state[2]);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0200
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0200, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    CommonEventListener *listener = nullptr;
	CommonEventListener *listener2 = nullptr;

    SetMatchingSkills(matchingSkills);

    SetSubscriberInfo1(listener, matchingSkills);

    SetSubscriberInfo2(listener2, matchingSkills);

    /* Publish */
    Test0200Publish1();

    Test0200Publish2();

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 2, 2);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0300
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0300, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    // make a want
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);

    // make publish info
    CommonEventPublishInfo publishInfo2;
    publishInfo2.SetSticky(true);

    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
    EXPECT_EQ("Subscribers:\tNo information", state[0]);
}

/*
 * @tc.number: CommonEventDumpTest_0400
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddScheme(SCHEME);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0500
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info  to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0500, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0600
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0600, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0700
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0700, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0800
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0800, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0900
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0900, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState(EVENT3, state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 1);
    EXPECT_EQ("Subscribers:\tNo information", state[0]);
    EXPECT_EQ("Sticky Events:\tNo information", state[1]);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

void Test100Publish1()
{
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);
}

void Test100Publish2()
{
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);

    // make publish info
    CommonEventPublishInfo publishInfo2;

    publishInfo2.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);
}

/*
 * @tc.number: CommonEventDumpTest_1000
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1000, TestSize.Level1)
{
    /* Subscribe */
	MatchingSkills matchingSkills;
	CommonEventListener *listener = nullptr;
	CommonEventListener *listener2 = nullptr;

	SetMatchingSkills(matchingSkills);

	SetSubscriberInfo1(listener, matchingSkills);

	SetSubscriberInfo2(listener2, matchingSkills);

    /* Publish */
    //Publish1
    Test100Publish1();

    //Publish1
    Test100Publish2();

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState(EVENT2, state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 2, 1);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_1100
 * @tc.name: test dump
 * @tc.desc:  Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1100, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;

    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}

/*
 * @tc.number: CommonEventDumpTest_1200
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1200, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}

/*
 * @tc.number: CommonEventDumpTest_1300
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1300, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}

/*
 * @tc.number: CommonEventDumpTest_1400
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1400, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}

/*
 * @tc.number: CommonEventDumpTest_1500
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1500, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    OHOS::AppExecFwk::ElementName element;
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}

/*
 * @tc.number: CommonEventDumpTest_1600
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1600, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}

/*
 * @tc.number: CommonEventDumpTest_1700
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1700, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, 2);
}
