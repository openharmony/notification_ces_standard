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

#include <cinttypes>

#include "event_log_wrapper.h"
#include "system_time.h"

namespace OHOS {
namespace EventFwk {
PublishManager::PublishManager()
{}

PublishManager::~PublishManager()
{}

bool PublishManager::CheckIsFloodAttack(pid_t appUid)
{
    EVENT_LOGI("enter");

    bool isAttacked = false;

    int64_t now = SystemTime::GetNowSysTime();
    EVENT_LOGI("dispatch common event by app (uid = %{publish}d) at now = %{public}" PRId64, appUid, now);

    auto iter = floodAttackAppStatistics_.find(appUid);
    if (iter == floodAttackAppStatistics_.end()) {
        floodAttackAppStatistics_[appUid].emplace_back(now);
        return isAttacked;
    }

    // Remove expired record
    auto iterVec = iter->second.begin();
    while (iterVec != iter->second.end()) {
        if (now - *iterVec > FLOOD_ATTACK_INTERVAL_MAX) {
            iterVec = iter->second.erase(iterVec);
        } else {
            break;
        }
    }

    if (iter->second.size() + 1 > FLOOD_ATTACK_NUMBER_MAX) {
        EVENT_LOGW("CES was maliciously attacked by app (uid = %{publish}d)", appUid);
        isAttacked = true;
    }
    iter->second.emplace_back(now);

    return isAttacked;
}
}  // namespace EventFwk
}  // namespace OHOS