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
#undef private
#undef protected
#include "common_event.h"
#include "common_event_support.h"
#include "inner_common_event_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
const int PID = 0;
const int UID = 1000;  // system app
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventPublishSystemEventTest : public testing::Test {
public:
    CommonEventPublishSystemEventTest()
    {}
    ~CommonEventPublishSystemEventTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    InnerCommonEventManager innerCommonEventManager;
};

void CommonEventPublishSystemEventTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
    OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);

    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->bmsDeath_ = new BMSDeathRecipient();
    if (!OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->bmsDeath_) {
        GTEST_LOG_(INFO) << "Failed to create death Recipient ptr BMSDeathRecipient";
        return;
    }
    if (!OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_->AsObject()->AddDeathRecipient(
        OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->bmsDeath_)) {
        GTEST_LOG_(INFO) << "Failed to add death recipient";
        return;
    }
}

void CommonEventPublishSystemEventTest::TearDownTestCase(void)
{}

void CommonEventPublishSystemEventTest::SetUp(void)
{}

void CommonEventPublishSystemEventTest::TearDown(void)
{}

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

/*
 * @tc.number: CommonEventPublishSystemEventTest_0100
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify Publish Systme CommonEvent success
 */
HWTEST_F(CommonEventPublishSystemEventTest, CommonEventPublishSystemEventTest_0100, Function | MediumTest | Level1)
{
    /* Publish */

    // make a want
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    struct tm curTime;
    // publish system event
    bool publishResult =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, nullptr, curTime, PID, UID, "bundlename");
    EXPECT_EQ(true, publishResult);
}

/*
 * @tc.number: CommonEventPublishSystemEventTest_0200
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify Publish Systme CommonEvent fail because is not systemapp
 */
HWTEST_F(CommonEventPublishSystemEventTest, CommonEventPublishSystemEventTest_0200, Function | MediumTest | Level1)
{
    /* Publish */

    // make a want
    Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    struct tm curTime;
    // publish system event
    bool publishResult =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, nullptr, curTime, PID, 0, "bundlename");
    EXPECT_EQ(false, publishResult);
}
