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

#include "common_event.h"

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "bundle_manager_helper.h"
#include "common_event_control_manager.h"
#include "common_event_manager_service.h"
#undef private
#undef protected

#include "common_event_manager.h"
#include "common_event_record.h"
#include "common_event_support.h"
#include "mock_bundle_manager.h"
#include "publish_manager.h"
#include "system_time.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
#define FLOOD_ATTACH_MAX 20

class CommonEventPublishManagerEventUnitTest : public testing::Test {
public:
    CommonEventPublishManagerEventUnitTest()
    {}
    ~CommonEventPublishManagerEventUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CommonEventPublishManagerEventUnitTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::DelayedSingleton<BundleManagerHelper>::GetInstance()->sptrBundleMgr_ =
        OHOS::iface_cast<OHOS::AppExecFwk::IBundleMgr>(bundleObject);
}

void CommonEventPublishManagerEventUnitTest::TearDownTestCase(void)
{}

void CommonEventPublishManagerEventUnitTest::SetUp(void)
{}

void CommonEventPublishManagerEventUnitTest::TearDown(void)
{}

/*
 * @tc.number: CommonEventPublishManagerEventUnitTest_0100
 * @tc.name: test event floodAttach
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 */
HWTEST_F(CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishManagerEventUnitTest, CommonEventPublishManagerEventUnitTestt_0100, TestSize.Level1";
    int i = 0;
    bool result = false;
    for (; i < 1000; ++i) {
        result = DelayedSingleton<PublishManager>::GetInstance()->CheckIsFloodAttack(100);
        if (!result) {
            GTEST_LOG_(INFO) << "after CheckIsFloodAttack---------------- i = " << i;
            EXPECT_EQ(true, i >= FLOOD_ATTACH_MAX);
            break;
        }
    }
    EXPECT_EQ(false, result);
}
