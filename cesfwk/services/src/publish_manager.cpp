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

#include "publish_manager.h"

#include "event_log_wrapper.h"
#include "system_time.h"

namespace OHOS {
namespace EventFwk {

#define FLOOD_ATTACH_MAX 20
#define FLOOD_ATTACH_INTERVAL_MAX 5

PublishManager::PublishManager()
    : lastPublishTime_(0),
      publishNum_(0),
      floodAttachMax_(FLOOD_ATTACH_MAX),
      floodAttachIntervalMax_(FLOOD_ATTACH_INTERVAL_MAX)
{}

PublishManager::~PublishManager()
{}

bool PublishManager::CheckValid()
{
    EVENT_LOGI("enter");

    bool isValid = false;
    int64_t nowSystemTime = SystemTime::GetNowSysTime();

    EVENT_LOGI("nowSystemTime = %{public}lld, lastPublishTime_ = %{public}lld, interval = %{public}lld",
        nowSystemTime,
        lastPublishTime_,
        nowSystemTime - lastPublishTime_);
    if (nowSystemTime - lastPublishTime_ < floodAttachIntervalMax_) {
        publishNum_++;
    } else {
        publishNum_ = 0;
    }

    lastPublishTime_ = nowSystemTime;

    if (publishNum_ <= floodAttachMax_) {
        isValid = true;
    }

    return isValid;
}
}  // namespace EventFwk
}  // namespace OHOS