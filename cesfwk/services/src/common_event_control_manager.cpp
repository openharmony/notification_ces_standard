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

#include "common_event_control_manager.h"
#include "bundle_manager_helper.h"
#include "common_event_subscriber_manager.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
#include "ievent_receive.h"
#include "system_time.h"

namespace OHOS {
namespace EventFwk {

#define TENSECONDS 10000
using frozenRecords = std::map<std::shared_ptr<EventSubscriberRecord>, std::vector<std::shared_ptr<CommonEventRecord>>>;

CommonEventControlManager::CommonEventControlManager()
    : handler_(nullptr), handlerOrdered_(nullptr), pendingTimeoutMessage_(false), scheduled_(false), TIMEOUT(TENSECONDS)
{
    EVENT_LOGD("enter");
}

CommonEventControlManager::~CommonEventControlManager()
{
    EVENT_LOGD("enter");
}

bool CommonEventControlManager::PublishCommonEvent(
    const CommonEventRecord &eventRecord, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    bool ret = false;

    if (!eventRecord.publishInfo->IsOrdered()) {
        ret = ProcessUnorderedEvent(eventRecord);
    } else {
        ret = ProcessOrderedEvent(eventRecord, commonEventListener);
    }

    return ret;
}

bool CommonEventControlManager::PublishFreezeCommonEvent(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!GetUnorderedEventHandler()) {
        return false;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    frozenRecords frozenEventRecords = spinstance->GetFrozenEvents(uid);

    EVENT_LOGD("frozenEventRecords size: %{public}d", frozenEventRecords.size());
    for (auto record : frozenEventRecords) {
        EVENT_LOGD("CommonEventRecord size: %{public}d", record.second.size());
        for (auto vec : record.second) {
            EVENT_LOGD("subscriber proxy: %{public}p", &(record.first->commonEventListener));
            std::function<void()> innerCallback =
                std::bind(&CommonEventControlManager::NotifyFreezeEvents, this, *(record.first), *vec);
            handler_->PostImmediateTask(innerCallback);
        }
    }

    return true;
}

bool CommonEventControlManager::NotifyFreezeEvents(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");
    EVENT_LOGD("subscriber proxy: %{public}p", &subscriberRecord.commonEventListener);
    EVENT_LOGD("subscriber uid: %{public}d", subscriberRecord.uid);
    EVENT_LOGD("subscriber isFreeze: %{public}d", subscriberRecord.isFreeze);
    EVENT_LOGD("subscriber freezeTime: %{public}lld", subscriberRecord.freezeTime);
    EVENT_LOGD("CommonEvent Action: %{public}s", eventRecord.commonEventData->GetWant().GetAction().c_str());
    EVENT_LOGD("CommonEvent Type: %{public}s", eventRecord.commonEventData->GetWant().GetType().c_str());

    sptr<IEventReceive> commonEventListenerProxy = iface_cast<IEventReceive>(subscriberRecord.commonEventListener);

    if (!commonEventListenerProxy) {
        EVENT_LOGE("Fail to get IEventReceive proxy");
        return false;
    }

    int ret = CheckPermission(subscriberRecord, eventRecord);
    if (ret != OrderedEventRecord::DELIVERY_DELIVERED) {
        EVENT_LOGE("check permission is failed");
        return false;
    }
    commonEventListenerProxy->NotifyEvent(*(eventRecord.commonEventData), false, eventRecord.publishInfo->IsSticky());
    return true;
}

bool CommonEventControlManager::GetUnorderedEventHandler()
{
    if (!handler_) {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
        if (!handler_) {
            EVENT_LOGE("Failed to create UnorderedEventHandler");
            return false;
        }
    }
    return true;
}

bool CommonEventControlManager::NotifyUnorderedEvent(const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    for (auto vec : spinstance->GetSubscriberRecords(eventRecord.commonEventData->GetWant())) {
        EVENT_LOGD("vec->uid: %{public}d", vec->uid);
        EVENT_LOGD("vec->isFreeze: %{public}d", vec->isFreeze);
        if (vec->isFreeze) {
            DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertFrozenEvents(vec, eventRecord);
        } else {
            sptr<IEventReceive> commonEventListenerProxy = iface_cast<IEventReceive>(vec->commonEventListener);
            if (!commonEventListenerProxy) {
                EVENT_LOGE("Failed to get IEventReceive proxy");
                continue;
            }

            int ret = CheckPermission(*vec, eventRecord);
            if (ret == OrderedEventRecord::DELIVERY_DELIVERED) {
                commonEventListenerProxy->NotifyEvent(
                    *(eventRecord.commonEventData), false, eventRecord.publishInfo->IsSticky());
            }
        }
    }
    return true;
}

bool CommonEventControlManager::ProcessUnorderedEvent(const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    bool ret = false;

    if (!GetUnorderedEventHandler()) {
        return ret;
    }

    std::function<void()> innerCallback =
        std::bind(&CommonEventControlManager::NotifyUnorderedEvent, this, eventRecord);

    if (eventRecord.isSystemEvent) {
        ret = handler_->PostImmediateTask(innerCallback);
    } else {
        ret = handler_->PostTask(innerCallback);
    }

    return ret;
}

std::shared_ptr<OrderedEventRecord> CommonEventControlManager::GetMatchingOrderedReceiver(sptr<IRemoteObject> proxy)
{
    EVENT_LOGI("enter");

    if (orderedEventQueue_.size() > 0) {
        std::shared_ptr<OrderedEventRecord> firstRecord = orderedEventQueue_.front();
        if ((firstRecord != nullptr) && (firstRecord->curReceiver == proxy)) {
            return firstRecord;
        }
    }

    return nullptr;
}

bool CommonEventControlManager::GetOrderedEventHandler()
{
    if (!handlerOrdered_) {
        handlerOrdered_ = std::make_shared<OrderedEventHandler>(EventRunner::Create(), shared_from_this());
        if (!handlerOrdered_) {
            EVENT_LOGE("Failed to create OrderedEventHandler");
            return false;
        }
    }
    return true;
}

bool CommonEventControlManager::ProcessOrderedEvent(
    const CommonEventRecord &commonEventRecord, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    bool ret = false;

    if (!GetOrderedEventHandler()) {
        return ret;
    }

    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    if (eventRecord == nullptr) {
        EVENT_LOGE("eventRecord is null");
        return ret;
    }

    std::shared_ptr<CommonEventSubscriberManager> spinstance =
        DelayedSingleton<CommonEventSubscriberManager>::GetInstance();

    eventRecord->FillCommonEventRecord(commonEventRecord);
    eventRecord->resultTo = commonEventListener;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;
    eventRecord->receivers = spinstance->GetSubscriberRecords(commonEventRecord.commonEventData->GetWant());
    for (auto vec : eventRecord->receivers) {
        eventRecord->deliveryState.emplace_back(OrderedEventRecord::DELIVERY_PENDING);
    }

    EnqueueOrderedRecord(eventRecord);

    ret = ScheduleOrderedCommonEvent();

    return ret;
}

bool CommonEventControlManager::EnqueueOrderedRecord(const std::shared_ptr<OrderedEventRecord> &eventRecordPtr)
{
    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr is null");
        return false;
    }

    orderedEventQueue_.emplace_back(eventRecordPtr);

    return true;
}

bool CommonEventControlManager::ScheduleOrderedCommonEvent()
{
    EVENT_LOGI("enter");

    if (scheduled_) {
        return true;
    }

    scheduled_ = true;

    return handlerOrdered_->SendEvent(InnerEvent::Get(OrderedEventHandler::ORDERED_EVENT_START));
}

bool CommonEventControlManager::NotifyOrderedEvent(std::shared_ptr<OrderedEventRecord> &eventRecordPtr, int index)
{
    EVENT_LOGI("enter with index %{public}d", index);

    if (eventRecordPtr == nullptr) {
        EVENT_LOGE("eventRecordPtr = nullptr");
        return false;
    }

    int receiverNum = eventRecordPtr->receivers.size();
    if ((index < 0) || (index >= receiverNum)) {
        EVENT_LOGE("Invalid index (= %{public}d)", index);
        return false;
    }

    int ret = CheckPermission(*(eventRecordPtr->receivers[index]), *eventRecordPtr);
    if (ret == OrderedEventRecord::DELIVERY_SKIPPED) {
        eventRecordPtr->deliveryState[index] = ret;
    } else if (ret == OrderedEventRecord::DELIVERY_DELIVERED) {
        if (eventRecordPtr->receivers[index]->isFreeze) {
            EVENT_LOGD("vec->uid: %{public}d", eventRecordPtr->receivers[index]->uid);
            EVENT_LOGD("vec->isFreeze: %{public}d", eventRecordPtr->receivers[index]->isFreeze);
            DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertFrozenEvents(
                eventRecordPtr->receivers[index], *eventRecordPtr);
            eventRecordPtr->deliveryState[index] = OrderedEventRecord::DELIVERY_SKIPPED;
            eventRecordPtr->curReceiver = nullptr;
            return true;
        }

        eventRecordPtr->deliveryState[index] = ret;

        eventRecordPtr->curReceiver = eventRecordPtr->receivers[index]->commonEventListener;

        eventRecordPtr->state = OrderedEventRecord::RECEIVEING;

        sptr<IEventReceive> receiver = iface_cast<IEventReceive>(eventRecordPtr->curReceiver);
        if (!receiver) {
            EVENT_LOGE("Failed to get IEventReceive proxy");
            eventRecordPtr->curReceiver = nullptr;
            return false;
        }

        eventRecordPtr->state = OrderedEventRecord::RECEIVED;

        receiver->NotifyEvent(*(eventRecordPtr->commonEventData), true, eventRecordPtr->publishInfo->IsSticky());
    }

    return true;
}

void CommonEventControlManager::ProcessNextOrderedEvent(bool isSendMsg)
{
    EVENT_LOGI("enter");

    if (isSendMsg) {
        scheduled_ = false;
    }

    std::shared_ptr<OrderedEventRecord> sp = nullptr;
    do {
        if (orderedEventQueue_.size() == 0) {
            return;
        }

        sp = orderedEventQueue_.front();
        bool forceReceive = false;
        int numReceivers = sp->receivers.size();
        int64_t nowSysTime = SystemTime::GetNowSysTime();

        if (sp->dispatchTime > 0) {
            if ((numReceivers > 0) && (nowSysTime > sp->dispatchTime + (2 * TIMEOUT * numReceivers))) {
                CurrentOrderedEventTimeout(false);
                forceReceive = true;
                sp->state = OrderedEventRecord::IDLE;
            }
        }

        if (sp->state != OrderedEventRecord::IDLE) {
            return;
        }

        if ((sp->receivers.size() == 0) || (sp->nextReceiver >= numReceivers) || sp->resultAbort || forceReceive) {
            // No more receivers for this ordered common event, then process the final result receiver
            if (sp->resultTo != nullptr) {
                EVENT_LOGI("Process the final subscriber");
                sptr<IEventReceive> receiver = iface_cast<IEventReceive>(sp->resultTo);
                if (!receiver) {
                    EVENT_LOGE("Failed to get IEventReceive proxy");
                    return;
                }
                receiver->NotifyEvent(*(sp->commonEventData), true, sp->publishInfo->IsSticky());

                sp->resultTo = nullptr;
            }

            CancelTimeout();

            orderedEventQueue_.erase(orderedEventQueue_.begin());

            sp = nullptr;
        }
    } while (sp == nullptr);

    int recIdx = sp->nextReceiver++;
    SetTime(recIdx, sp, pendingTimeoutMessage_);

    NotifyOrderedEvent(sp, recIdx);
    if (sp->curReceiver == nullptr) {
        sp->state = OrderedEventRecord::IDLE;
        ScheduleOrderedCommonEvent();
    }

    return;
}

bool CommonEventControlManager::SetTimeout(int64_t timeoutTime)
{
    EVENT_LOGI("enter");

    bool ret = true;

    if (!pendingTimeoutMessage_) {
        pendingTimeoutMessage_ = true;
        ret = handlerOrdered_->SendTimingEvent(OrderedEventHandler::ORDERED_EVENT_TIMEOUT, timeoutTime);
    }

    return ret;
}

bool CommonEventControlManager::CancelTimeout()
{
    EVENT_LOGI("enter");

    if (pendingTimeoutMessage_) {
        pendingTimeoutMessage_ = false;
        handlerOrdered_->RemoveEvent(OrderedEventHandler::ORDERED_EVENT_TIMEOUT);
    }

    return true;
}

void CommonEventControlManager::CurrentOrderedEventTimeout(bool isFromMsg)
{
    EVENT_LOGI("enter");

    if (isFromMsg) {
        pendingTimeoutMessage_ = false;
    }

    if (orderedEventQueue_.size() == 0) {
        return;
    }

    int64_t nowSysTime = SystemTime::GetNowSysTime();
    std::shared_ptr<OrderedEventRecord> sp = orderedEventQueue_.front();

    if (isFromMsg) {
        int64_t timeoutTime = sp->receiverTime + TIMEOUT;
        if (timeoutTime > nowSysTime) {
            SetTimeout(timeoutTime);
            return;
        }
    }

    // The processing of current receiver has timeout
    sp->receiverTime = nowSysTime;

    if (sp->nextReceiver > 0) {
        std::shared_ptr<EventSubscriberRecord> subscriberRecord = sp->receivers[sp->nextReceiver - 1];
        EVENT_LOGW("Timeout: When %{public}s (pid = %{public}d, uid = %{public}d) process common event %{public}s",
            subscriberRecord->bundleName.c_str(),
            subscriberRecord->pid,
            subscriberRecord->uid,
            sp->commonEventData->GetWant().GetAction().c_str());

        sp->deliveryState[sp->nextReceiver - 1] = OrderedEventRecord::DELIVERY_TIMEOUT;
    }

    // Forced to finish the current receiver to process the next receiver
    int code = sp->commonEventData->GetCode();
    const std::string &strRef = sp->commonEventData->GetData();
    bool abort = sp->resultAbort;
    FinishReceiverLocked(sp, code, strRef, abort);

    ScheduleOrderedCommonEvent();

    return;
}

bool CommonEventControlManager::FinishReceiverLocked(std::shared_ptr<OrderedEventRecord> recordPtr, const int &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (recordPtr == nullptr) {
        EVENT_LOGE("recordPtr is null");
        return false;
    }

    EVENT_LOGI("enter recordPtr->state=%{public}d", recordPtr->state);

    int state = recordPtr->state;
    recordPtr->state = OrderedEventRecord::IDLE;
    recordPtr->curReceiver = nullptr;
    recordPtr->commonEventData->SetCode(code);
    recordPtr->commonEventData->SetData(receiverData);
    recordPtr->resultAbort = abortEvent;

    return state == OrderedEventRecord::RECEIVED;
}

bool CommonEventControlManager::FinishReceiverAction(std::shared_ptr<OrderedEventRecord> recordPtr, const int &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (recordPtr == nullptr) {
        EVENT_LOGE("recordPtr is nullptr");
        return false;
    }

    bool doNext = false;
    doNext = FinishReceiverLocked(recordPtr, code, receiverData, abortEvent);
    if (doNext) {
        ProcessNextOrderedEvent(false);
    }

    return true;
}

void CommonEventControlManager::CheckSubscriberRequiredPermission(const std::string &subscriberRequiredPermission,
    const CommonEventRecord &eventRecord, const EventSubscriberRecord &subscriberRecord, bool &skip)
{
    bool ret = false;

    if (!subscriberRequiredPermission.empty()) {
        ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
            eventRecord.bundleName, subscriberRequiredPermission);
        if (!ret) {
            EVENT_LOGW("No permission to send common event %{public}s "
                       "from %{public}s (pid = %{public}d, uid = %{public}d) "
                       "to %{public}s (pid = %{public}d, uid = %{public}d) "
                       "due to registered subscriber require %{public}s",
                eventRecord.commonEventData->GetWant().GetAction().c_str(),
                eventRecord.bundleName.c_str(),
                eventRecord.pid,
                eventRecord.uid,
                subscriberRecord.bundleName.c_str(),
                subscriberRecord.pid,
                subscriberRecord.uid,
                subscriberRequiredPermission.c_str());
            skip = true;
        }
    }
}

void CommonEventControlManager::CheckPublisherRequiredPermissions(
    const std::vector<std::string> &publisherRequiredPermissions, const EventSubscriberRecord &subscriberRecord,
    const CommonEventRecord &eventRecord, bool &skip)
{
    bool ret = false;

    if ((!skip) && (publisherRequiredPermissions.size() > 0)) {
        for (auto publisherRequiredPermission : publisherRequiredPermissions) {
            ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
                subscriberRecord.bundleName, publisherRequiredPermission);
            if (!ret) {
                skip = true;
                EVENT_LOGW("No permission to receive common event %{public}s "
                           "to %{public}s (pid = %{public}d, uid = %{public}d) "
                           "due to publisher %{public}s (pid = %{public}d, uid = %{public}d) requires %{public}s",
                    eventRecord.commonEventData->GetWant().GetAction().c_str(),
                    subscriberRecord.bundleName.c_str(),
                    subscriberRecord.pid,
                    subscriberRecord.uid,
                    eventRecord.bundleName.c_str(),
                    eventRecord.pid,
                    eventRecord.uid,
                    publisherRequiredPermission.c_str());
                break;
            }
        }
    }
}

int CommonEventControlManager::CheckPermission(
    const EventSubscriberRecord &subscriberRecord, const CommonEventRecord &eventRecord)
{
    EVENT_LOGI("enter");

    bool skip = false;
    bool ret = false;
    std::string subscriberRequiredPermission = subscriberRecord.eventSubscriberInfo->GetPermission();
    std::vector<std::string> publisherRequiredPermissions = eventRecord.publishInfo->GetSubscriberPermissions();

    Permission per = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(
        eventRecord.commonEventData->GetWant().GetAction());

    ret = CheckSubcriberPermission(per, subscriberRecord);
    EVENT_LOGI("after CheckSubcriberPermission and ret = %{public}d", ret);
    if (!ret) {
        return OrderedEventRecord::DELIVERY_SKIPPED;
    }

    CheckSubscriberRequiredPermission(subscriberRequiredPermission, eventRecord, subscriberRecord, skip);

    CheckPublisherRequiredPermissions(publisherRequiredPermissions, subscriberRecord, eventRecord, skip);

    if (skip) {
        return OrderedEventRecord::DELIVERY_SKIPPED;
    }

    return OrderedEventRecord::DELIVERY_DELIVERED;
}

bool CommonEventControlManager::CheckSubcriberPermission(
    const Permission &permission, const EventSubscriberRecord &subscriberRecord)
{
    EVENT_LOGI("enter");
    EVENT_LOGI("size = %{public}d", permission.names.size());
    bool ret = false;

    if (permission.names.size() < 1) {
        return true;
    }
    if (permission.names.size() == 1) {
        ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
            subscriberRecord.bundleName, permission.names[0]);
    } else {
        if (permission.state == Permission::AND) {
            for (auto vec : permission.names) {
                ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
                    subscriberRecord.bundleName, vec);
                if (!ret) {
                    break;
                }
            }
        } else if (permission.state == Permission::OR) {
            for (auto vec : permission.names) {
                ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(
                    subscriberRecord.bundleName, vec);
                if (ret) {
                    break;
                }
            }
        }
    }

    return ret;
}

void CommonEventControlManager::SetTime(int recIdx, std::shared_ptr<OrderedEventRecord> &sp, bool timeoutMessage)
{
    EVENT_LOGI("enter");

    sp->receiverTime = SystemTime::GetNowSysTime();

    if (recIdx == 0) {
        sp->dispatchTime = sp->receiverTime;
    }

    if (!timeoutMessage) {
        int64_t timeoutTime = sp->receiverTime + TIMEOUT;
        SetTimeout(timeoutTime);
    }

    return;
}
}  // namespace EventFwk
}  // namespace OHOS