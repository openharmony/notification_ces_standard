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

#define private public
#define protected public
#include "bundle_manager_helper.h"
#include "common_event.h"
#include "common_event_manager_service.h"
#undef private
#undef protected

#include "mock_bundle_manager.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
const int PRIORITY = 1;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
class CommonEventSubscribeUnitTest : public testing::Test {
public:
    CommonEventSubscribeUnitTest()
    {}
    ~CommonEventSubscribeUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
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

class CommonEventStubTest : public CommonEventStub {
public:
    CommonEventStubTest()
    {}

    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return false;
    }

    virtual bool SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscribeInfo, const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return false;
    }

    virtual bool UnsubscribeCommonEvent(const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return false;
    }

    virtual bool DumpState(const std::string &event, std::vector<std::string> &state)
    {
        return false;
    }

    virtual ~CommonEventStubTest()
    {}

    virtual bool FinishReceiver(const OHOS::sptr<IRemoteObject> &proxy, const int &code,
        const std::string &receiverData, const bool &abortEvent)
    {
        return false;
    }
};

class EventReceiveStubTest : public EventReceiveStub {
public:
    EventReceiveStubTest()
    {}

    ~EventReceiveStubTest()
    {}

    virtual void NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky)
    {}
};

void CommonEventSubscribeUnitTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventSubscribeUnitTest::TearDownTestCase(void)
{}

void CommonEventSubscribeUnitTest::SetUp(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
}

void CommonEventSubscribeUnitTest::TearDown(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0100
 * @tc.name: test SubscribeCommonEvent
 * @tc.desc: Verify SubscribeCommonEvent success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0100, Function | MediumTest | Level1)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddScheme(SCHEME);

    // make subcriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(PRIORITY);
    subscribeInfo.SetPermission(PERMISSION);
    subscribeInfo.SetDeviceId(DEVICEDID);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listner
    CommonEventListener *listener = new CommonEventListener(subscriber);

    // SubscribeCommonEvent
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject());
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0200
 * @tc.name: test SubscribeCommonEvent
 * @tc.desc: Verify SubscribeCommonEvent fail bacause service stop
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0200, Function | MediumTest | Level1)
{
    // OnStop
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddScheme(SCHEME);

    // make subcriber info
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(PRIORITY);
    subscribeInfo.SetPermission(PERMISSION);
    subscribeInfo.SetDeviceId(DEVICEDID);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listner
    CommonEventListener *listener = new CommonEventListener(subscriber);

    // SubscribeCommonEvent
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, listener->AsObject());
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0300
 * @tc.name: test subscribe event
 * @tc.desc: InnerCommonEventManager subscribe common event success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0300, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listner
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // SubscribeCommonEvent
    struct tm curTime{0};
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_EQ(
        true, innerCommonEventManager.SubscribeCommonEvent(subscribeInfo, commonEventListenerPtr, curTime, 0, 0, ""));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0400
 * @tc.name: test subscribe event
 * @tc.desc: InnerCommonEventManager subscribe common event fail because subscribeInfo has not event
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0400, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listner
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // SubscribeCommonEvent
    InnerCommonEventManager innerCommonEventManager;
    struct tm curTime{0};
    EXPECT_EQ(
        false, innerCommonEventManager.SubscribeCommonEvent(subscribeInfo, commonEventListenerPtr, curTime, 0, 0, ""));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0500
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event, inner common event manager
 *                          common event listener is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0500, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // SubscribeCommonEvent
    InnerCommonEventManager innerCommonEventManager;
    OHOS::sptr<OHOS::IRemoteObject> sp(nullptr);
    struct tm curTime;
    EXPECT_EQ(false, innerCommonEventManager.SubscribeCommonEvent(subscribeInfo, sp, curTime, 0, 0, ""));
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0600
 * @tc.name: test subscribe event
 * @tc.desc: subscribe common event success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0600, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listner
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // InsertSubscriber
    struct tm curTime{0};
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.InsertSubscriber(
        subscribeInfoPtr, commonEventListener, curTime, 0, 0, "bundlename");
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0700
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event , common event subscriber manager
 *                          event subscriber info is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0700, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make common event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // InsertSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    struct tm curTime{0};
    int result =
        commonEventSubscriberManager.InsertSubscriber(nullptr, commonEventListenerPtr, curTime, 0, 0, "bundlename");
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0800
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event , common event subscriber manager
 *                          event common event listener is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0800, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // InsertSubscriber
    struct tm curTime{0};
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.InsertSubscriber(subscribeInfoPtr, nullptr, curTime, 0, 0, "bundlename");
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_0900
 * @tc.name: test subscribe event
 * @tc.desc: 1. subscribe common event
 *                      2. fail subscribe common event , common event subscriber manager
 *                          event size is null
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_0900, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr =
        std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);

    // make subscriber
    CommonEventStubTest CommonEventStubTest;
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // InsertSubscriber
    struct tm curTime{0};
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.InsertSubscriber(
        subscribeInfoPtr, commonEventListenerPtr, curTime, 0, 0, "bundlename");
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_1000
 * @tc.name: ttest subscribe event
 * @tc.desc:  insert subscriber record locked success
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_1000, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEvent(ENTITY);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // make events
    std::vector<std::string> events = subscribeInfo.GetMatchingSkills().GetEvents();

    // make record
    struct tm recordTime{0};
    auto record = std::make_shared<EventSubscriberRecord>();
    record->eventSubscriberInfo = std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    record->commonEventListener = commonEventListenerPtr;
    record->recordTime = recordTime;
    record->pid = 0;
    record->uid = 0;
    record->bundleName = "bundleName";

    // InsertSubscriberRecordLocked
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.InsertSubscriberRecordLocked(events, record);
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_1100
 * @tc.name: test subscribe event
 * @tc.desc:  insert subscriber record locked fail because events size is 0
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_1100, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscribeInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // make events
    std::vector<std::string> events = subscribeInfo.GetMatchingSkills().GetEvents();

    // make record
    struct tm recordTime{0};
    auto record = std::make_shared<EventSubscriberRecord>();
    record->eventSubscriberInfo = std::make_shared<CommonEventSubscribeInfo>(subscribeInfo);
    record->commonEventListener = commonEventListenerPtr;
    record->recordTime = recordTime;
    record->pid = 0;
    record->uid = 0;
    record->bundleName = "bundleName";

    // InsertSubscriberRecordLocked
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.InsertSubscriberRecordLocked(events, record);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * @tc.number: CommonEventSubscribeUnitTest_1200
 * @tc.name: test subscribe event
 * @tc.desc:  insert subscriber record locked fail because record is nullptr
 */
HWTEST_F(CommonEventSubscribeUnitTest, CommonEventSubscribeUnitTest_1200, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    // make events
    std::vector<std::string> events = subscribeInfo.GetMatchingSkills().GetEvents();
    CommonEventSubscriberManager commonEventSubscriberManager;

    // InsertSubscriberRecordLocked
    int result = commonEventSubscriberManager.InsertSubscriberRecordLocked(events, nullptr);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}
