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
#include "support.h"

#include <uv.h>

#include "common_event_manager.h"
#include "event_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace EventFwkNapi {
using namespace OHOS::EventFwk;

static const std::int32_t STR_MAX_SIZE = 64;
static const std::int32_t PUBLISH_MAX_PARA = 2;
static const std::int32_t GETSUBSCREBEINFO_MAX_PARA = 1;
static const std::int32_t ISORDEREDCOMMONEVENT_MAX_PARA = 1;
static const std::int32_t ISSTICKYCOMMONEVENT_MAX_PARA = 1;
static const std::int32_t GET_CODE_MAX_PARA = 1;
static const std::int32_t SET_CODE_MAX_PARA = 2;
static const std::int32_t GET_DATA_MAX_PARA = 1;
static const std::int32_t SET_DATA_MAX_PARA = 2;
static const std::int32_t SET_CODE_AND_DATA_MAX_PARA = 3;
static const std::int32_t ABORT_MAX_PARA = 1;
static const std::int32_t CLEAR_ABORT_MAX_PARA = 1;
static const std::int32_t GET_ABORT_MAX_PARA = 1;
static const std::int32_t FINISH_MAX_PARA = 1;
static const std::int32_t ARGS_TWO = 2;
static const std::int32_t PARAM0 = 0;
static const std::int32_t PARAM1 = 1;
static const std::int32_t NO_ERROR = 0;

SubscriberInstance::SubscriberInstance(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
{}

SubscriberInstance::~SubscriberInstance()
{}

void SubscriberInstance::OnReceiveEvent(const CommonEventData &data)
{
    EVENT_LOGI("OnReceiveEvent start");
    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(env_, &loop);
#endif  // NAPI_VERSION >= 2

    uv_work_t *work = new uv_work_t;

    CommonEventDataWorker *commonEventDataWorker = new CommonEventDataWorker();
    commonEventDataWorker->want = data.GetWant();
    EVENT_LOGI("OnReceiveEvent() action = %{public}s", data.GetWant().GetAction().c_str());
    commonEventDataWorker->code = data.GetCode();
    commonEventDataWorker->data = data.GetData();
    commonEventDataWorker->env = env_;
    commonEventDataWorker->ref = ref_;

    work->data = (void *)commonEventDataWorker;

    EVENT_LOGI("OnReceiveEvent this = %{public}p", this);

    if (this->IsOrderedCommonEvent()) {
        for (auto subscriberInstance : subscriberInstances) {
            EVENT_LOGI("OnReceiveEvent get = %{public}p", subscriberInstance.first.get());
            if (subscriberInstance.first.get() == this) {
                subscriberInstances[subscriberInstance.first].commonEventResult = GoAsyncCommonEvent();
                break;
            }
        }
    }

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            EVENT_LOGI("OnReceiveEvent uv_work_t start");
            CommonEventDataWorker *commonEventDataWorkerData = (CommonEventDataWorker *)work->data;
            if (commonEventDataWorkerData == nullptr) {
                return;
            }

            napi_value result;
            napi_create_object(commonEventDataWorkerData->env, &result);
            napi_value value;

            // event
            napi_create_string_utf8(commonEventDataWorkerData->env,
                commonEventDataWorkerData->want.GetAction().c_str(),
                NAPI_AUTO_LENGTH,
                &value);
            napi_set_named_property(commonEventDataWorkerData->env, result, "event", value);

            // bundleName
            napi_create_string_utf8(commonEventDataWorkerData->env,
                commonEventDataWorkerData->want.GetBundle().c_str(),
                NAPI_AUTO_LENGTH,
                &value);
            napi_set_named_property(commonEventDataWorkerData->env, result, "bundleName", value);

            // code
            napi_create_int32(commonEventDataWorkerData->env, commonEventDataWorkerData->code, &value);
            napi_set_named_property(commonEventDataWorkerData->env, result, "code", value);

            // data
            napi_create_string_utf8(
                commonEventDataWorkerData->env, commonEventDataWorkerData->data.c_str(), NAPI_AUTO_LENGTH, &value);
            napi_set_named_property(commonEventDataWorkerData->env, result, "data", value);

            napi_value undefined;
            napi_get_undefined(commonEventDataWorkerData->env, &undefined);

            napi_value callback;
            napi_value resultout;
            napi_get_reference_value(commonEventDataWorkerData->env, commonEventDataWorkerData->ref, &callback);

            napi_value results[ARGS_TWO] = {0};
            results[PARAM0] = GetCallbackErrorValue(commonEventDataWorkerData->env, NO_ERROR);
            results[PARAM1] = result;
            NAPI_CALL_RETURN_VOID(commonEventDataWorkerData->env,
                napi_call_function(
                    commonEventDataWorkerData->env, undefined, callback, ARGS_TWO, &results[PARAM0], &resultout));

            delete commonEventDataWorkerData;
            commonEventDataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });

    EVENT_LOGI("OnReceiveEvent end");
}

void SubscriberInstance::SetEnv(const napi_env &env)
{
    env_ = env;
}

void SubscriberInstance::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = 0;
    napi_get_null(env, &result);

    return result;
}

napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

napi_value ParseParametersByCreateSubscriber(const napi_env &env, const napi_value (&argv)[CREATE_MAX_PARA],
    const size_t &argc, CommonEventSubscribeInfoByjs &subscribeInfo, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:CommonEventSubscribeInfo
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. object expected.");

    // argv[1]:callback
    if (argc >= CREATE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype = napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value &result)
{
    napi_value undefined;
    napi_get_undefined(env, &undefined);

    napi_value callback;
    napi_value resultout;
    napi_get_reference_value(env, callbackIn, &callback);

    napi_value results[ARGS_TWO] = {0};
    results[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    results[PARAM1] = result;

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &results[PARAM0], &resultout));
}

void SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result)
{
    napi_resolve_deferred(env, deferred, result);
}

void ReturnCallbackPromise(const napi_env &env, const bool &isCallback, const napi_ref &callback,
    const napi_deferred &deferred, const napi_value &result)
{
    EVENT_LOGI("ReturnCallbackPromise start");

    if (isCallback) {
        SetCallback(env, callback, result);
    } else {
        SetPromise(env, deferred, result);
    }
}

void PaddingAsyncCallbackInfoCreateSubscriber(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoCreate *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoCreateSubscriber start");

    if (argc >= CREATE_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

napi_value CreateSubscriber(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("CreateSubscriber start");

    size_t argc = CREATE_MAX_PARA;
    napi_value argv[CREATE_MAX_PARA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    CommonEventSubscribeInfoByjs subscribeInfoByjs;
    napi_ref callback = 0;
    if (ParseParametersByCreateSubscriber(env, argv, argc, subscribeInfoByjs, callback) == nullptr) {
        return NapiGetNull(env);
    }

    AsyncCallbackInfoCreate *asynccallbackinfo =
        new AsyncCallbackInfoCreate{.env = env, .asyncWork = nullptr, .subscriberInfo = nullptr};

    napi_value promise = 0;

    PaddingAsyncCallbackInfoCreateSubscriber(env, argc, asynccallbackinfo, callback, promise);

    napi_create_reference(env, argv[0], 1, &asynccallbackinfo->subscriberInfo);

    napi_value resourceName;
    napi_create_string_latin1(env, "CreateSubscriber", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) { EVENT_LOGI("CreateSubscriber napi_create_async_work start"); },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("CreateSubscriber napi_create_async_work end");
            AsyncCallbackInfoCreate *asynccallbackinfo = (AsyncCallbackInfoCreate *)data;
            napi_value subscriberInfoRefValue;
            napi_get_reference_value(env, asynccallbackinfo->subscriberInfo, &subscriberInfoRefValue);

            napi_new_instance(env, g_CommonEventSubscriber, 1, &subscriberInfoRefValue, &asynccallbackinfo->result);

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                asynccallbackinfo->result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByGetSubscribeInfo(
    const napi_env &env, const size_t &argc, const napi_value (&argv)[1], napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= GETSUBSCREBEINFO_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void SetEventsResult(const napi_env &env, const std::vector<std::string> &events, napi_value &commonEventSubscribeInfo)
{
    EVENT_LOGI("SetEventsResult start");

    napi_value value = nullptr;

    if (events.size() > 0) {
        napi_value nEvents;
        if (napi_create_array(env, &nEvents) != napi_ok) {
            return;
        }
        size_t index = 0;
        for (auto event : events) {
            napi_create_string_utf8(env, event.c_str(), NAPI_AUTO_LENGTH, &value);
            napi_set_element(env, nEvents, index, value);
            index++;
        }

        napi_set_named_property(env, commonEventSubscribeInfo, "events", nEvents);
    }

    EVENT_LOGI("SetEventsResult end");
}

void SetPublisherPermissionResult(
    const napi_env &env, const std::string &permission, napi_value &commonEventSubscribeInfo)
{
    EVENT_LOGI("SetPublisherPermissionResult start");

    napi_value value;
    napi_create_string_utf8(env, permission.c_str(), NAPI_AUTO_LENGTH, &value);

    napi_set_named_property(env, commonEventSubscribeInfo, "publisherPermission", value);
}

void SetPublisherDeviceIdResult(const napi_env &env, const std::string &deviceId, napi_value &commonEventSubscribeInfo)
{
    EVENT_LOGI("SetPublisherDeviceIdResult start");

    napi_value value;
    napi_create_string_utf8(env, deviceId.c_str(), NAPI_AUTO_LENGTH, &value);

    napi_set_named_property(env, commonEventSubscribeInfo, "publisherDeviceId", value);
}

void SetPublisherPriorityResult(const napi_env &env, const int32_t &priority, napi_value &commonEventSubscribeInfo)
{
    EVENT_LOGI("SetPublisherPriorityResult start");

    napi_value value;
    napi_create_int32(env, priority, &value);

    napi_set_named_property(env, commonEventSubscribeInfo, "priority", value);
}

void PaddingAsyncCallbackInfoGetSubscribeInfo(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoGetSubscribeInfo start");

    if (argc >= GETSUBSCREBEINFO_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

void PaddingNapiCreateAsyncWorkCallbackInfo(
    AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, SubscriberInstance *&subscriber)
{
    EVENT_LOGI("PaddingNapiCreateAsyncWorkCallbackInfo start");

    asynccallbackinfo->events = subscriber->GetSubscribeInfo().GetMatchingSkills().GetEvents();
    asynccallbackinfo->permission = subscriber->GetSubscribeInfo().GetPermission();
    asynccallbackinfo->deviceId = subscriber->GetSubscribeInfo().GetDeviceId();
    asynccallbackinfo->priority = subscriber->GetSubscribeInfo().GetPriority();
}

void SetNapiResult(const napi_env &env, const AsyncCallbackInfoSubscribeInfo *asynccallbackinfo, napi_value &result)
{
    EVENT_LOGI("SetNapiResult start");

    SetEventsResult(env, asynccallbackinfo->events, result);
    SetPublisherPermissionResult(env, asynccallbackinfo->permission, result);
    SetPublisherDeviceIdResult(env, asynccallbackinfo->deviceId, result);
    SetPublisherPriorityResult(env, asynccallbackinfo->priority, result);
}

napi_value GetSubscribeInfo(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("GetSubscribeInfo start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByGetSubscribeInfo(env, argc, argv, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("GetSubscribeInfo objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoSubscribeInfo *asynccallbackinfo =
        new AsyncCallbackInfoSubscribeInfo{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoGetSubscribeInfo(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "getSubscribeInfo", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("GetSubscribeInfo napi_create_async_work start");
            AsyncCallbackInfoSubscribeInfo *asynccallbackinfo = (AsyncCallbackInfoSubscribeInfo *)data;

            PaddingNapiCreateAsyncWorkCallbackInfo(asynccallbackinfo, asynccallbackinfo->objectInfo);
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("GetSubscribeInfo napi_create_async_work end");
            AsyncCallbackInfoSubscribeInfo *asynccallbackinfo = (AsyncCallbackInfoSubscribeInfo *)data;

            napi_value result = 0;
            napi_create_object(env, &result);
            SetNapiResult(env, asynccallbackinfo, result);

            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByIsOrderedCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= ISORDEREDCOMMONEVENT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoIsOrderedCommonEvent(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoOrderedCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoIsOrderedCommonEvent start");

    if (argc >= ISORDEREDCOMMONEVENT_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

napi_value IsOrderedCommonEvent(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("IsOrderedCommonEvent start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    napi_ref callback = 0;
    if (ParseParametersByIsOrderedCommonEvent(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("IsOrderedCommonEvent objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoOrderedCommonEvent *asynccallbackinfo =
        new AsyncCallbackInfoOrderedCommonEvent{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};
    napi_value promise = 0;
    PaddingAsyncCallbackInfoIsOrderedCommonEvent(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "isOrderedCommonEvent", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("IsOrderedCommonEvent napi_create_async_work start");
            AsyncCallbackInfoOrderedCommonEvent *asynccallbackinfo = (AsyncCallbackInfoOrderedCommonEvent *)data;

            asynccallbackinfo->isOrdered = asynccallbackinfo->objectInfo->IsOrderedCommonEvent();
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("IsOrderedCommonEvent napi_create_async_work end");
            AsyncCallbackInfoOrderedCommonEvent *asynccallbackinfo = (AsyncCallbackInfoOrderedCommonEvent *)data;

            napi_value result = 0;
            napi_get_boolean(env, asynccallbackinfo->isOrdered, &result);

            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByIsStickyCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= ISSTICKYCOMMONEVENT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoIsStickyCommonEvent(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoStickyCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoIsStickyCommonEvent start");

    if (argc >= ISSTICKYCOMMONEVENT_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoIsStickyCommonEvent end");
}

napi_value IsStickyCommonEvent(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("IsStickyCommonEvent start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByIsStickyCommonEvent(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("IsStickyCommonEvent: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoStickyCommonEvent *asynccallbackinfo =
        new AsyncCallbackInfoStickyCommonEvent{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoIsStickyCommonEvent(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "isStickyCommonEvent", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("isStickyCommonEvent napi_create_async_work start");
            AsyncCallbackInfoStickyCommonEvent *asynccallbackinfo = (AsyncCallbackInfoStickyCommonEvent *)data;

            asynccallbackinfo->isSticky = asynccallbackinfo->objectInfo->IsStickyCommonEvent();
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("isStickyCommonEvent napi_create_async_work end");
            AsyncCallbackInfoStickyCommonEvent *asynccallbackinfo = (AsyncCallbackInfoStickyCommonEvent *)data;

            napi_value result = 0;
            napi_get_boolean(env, asynccallbackinfo->isSticky, &result);
            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}
napi_value ParseParametersByGetCode(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= GET_CODE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoGetCode(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetCode *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoGetCode start");

    if (argc >= GET_CODE_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoGetCode end");
}

std::shared_ptr<AsyncCommonEventResult> GetAsyncResult(const SubscriberInstance *objectInfo)
{
    EVENT_LOGI("GetAsyncResult start");
    if (objectInfo) {
        for (auto subscriberInstance : subscriberInstances) {
            EVENT_LOGI("GetAsyncResult SubscriberInstance = %{public}p", subscriberInstance.first.get());
            if (subscriberInstance.first.get() == objectInfo) {
                EVENT_LOGI("GetAsyncResult Result = %{public}p", subscriberInstance.second.commonEventResult.get());
                return subscriberInstance.second.commonEventResult;
            }
        }
    }
    EVENT_LOGI("GetAsyncResult end");
    return nullptr;
}
napi_value GetCode(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("GetCode start");
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByGetCode(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("GetCode: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoGetCode *asynccallbackinfo =
        new AsyncCallbackInfoGetCode{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoGetCode(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "getCode", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("GetCode napi_create_async_work start");
            AsyncCallbackInfoGetCode *asynccallbackinfo = (AsyncCallbackInfoGetCode *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asynccallbackinfo->code = asyncResult->GetCode();
            } else {
                asynccallbackinfo->code = 0;
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("GetCode napi_create_async_work end");
            AsyncCallbackInfoGetCode *asynccallbackinfo = (AsyncCallbackInfoGetCode *)data;

            napi_value result = 0;
            napi_create_int32(env, asynccallbackinfo->code, &result);
            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersBySetCode(
    const napi_env &env, const napi_value (&argv)[2], size_t argc, int &code, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:code
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &code));

    // argv[1]:callback
    if (argc >= SET_CODE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }
    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoSetCode(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetCode *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoSetCode start");

    if (argc >= SET_CODE_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoSetCode end");
}

napi_value SetCode(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("SetCode start");
    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    int code = 0;
    if (ParseParametersBySetCode(env, argv, argc, code, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("SetCode: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoSetCode *asynccallbackinfo =
        new AsyncCallbackInfoSetCode{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo, .code = code};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoSetCode(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "setCode", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("SetCode napi_create_async_work start");
            AsyncCallbackInfoSetCode *asynccallbackinfo = (AsyncCallbackInfoSetCode *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            EVENT_LOGI("SetCode get = %{public}p", asyncResult.get());
            if (asyncResult) {
                EVENT_LOGI("SetCode get2 = %{public}p", asyncResult.get());
                asyncResult->SetCode(asynccallbackinfo->code);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("SetCode napi_create_async_work end");
            AsyncCallbackInfoSetCode *asynccallbackinfo = (AsyncCallbackInfoSetCode *)data;

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                NapiGetNull(env));

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByGetData(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= GET_DATA_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoGetData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoGetData start");

    if (argc >= GET_DATA_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoGetData end");
}

napi_value GetData(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("GetData start");
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByGetData(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("GetData: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoGetData *asynccallbackinfo =
        new AsyncCallbackInfoGetData{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoGetData(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "getData", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("GetData napi_create_async_work start");
            AsyncCallbackInfoGetData *asynccallbackinfo = (AsyncCallbackInfoGetData *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asynccallbackinfo->data = asyncResult->GetData();
            } else {
                asynccallbackinfo->data = std::string();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("GetData napi_create_async_work end");
            AsyncCallbackInfoGetData *asynccallbackinfo = (AsyncCallbackInfoGetData *)data;

            napi_value result = 0;
            napi_create_string_utf8(env, asynccallbackinfo->data.c_str(), NAPI_AUTO_LENGTH, &result);
            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersBySetData(
    const napi_env &env, const napi_value (&argv)[2], size_t argc, std::string &data, napi_ref &callback)
{
    napi_valuetype valuetype;
    size_t strLen = 0;
    char str[STR_MAX_SIZE] = {0};
    // argv[0]:data
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE - 1, &strLen));
    data = str;

    // argv[1]:callback
    if (argc >= SET_CODE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }
    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoSetData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoSetData start");

    if (argc >= SET_DATA_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoSetData end");
}

napi_value SetData(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("SetData start");
    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    std::string data;
    if (ParseParametersBySetData(env, argv, argc, data, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("SetData: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoSetData *asynccallbackinfo =
        new AsyncCallbackInfoSetData{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo, .data = data};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoSetData(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "setData", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("SetData napi_create_async_work start");
            AsyncCallbackInfoSetData *asynccallbackinfo = (AsyncCallbackInfoSetData *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            EVENT_LOGI("SetData get = %{public}p", asyncResult.get());
            if (asyncResult) {
                EVENT_LOGI("SetData get2 = %{public}p", asyncResult.get());
                asyncResult->SetData(asynccallbackinfo->data);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("SetData napi_create_async_work end");
            AsyncCallbackInfoSetData *asynccallbackinfo = (AsyncCallbackInfoSetData *)data;

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                NapiGetNull(env));

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersBySetCodeAndData(
    const napi_env &env, const napi_value (&argv)[3], size_t argc, int &code, std::string &data, napi_ref &callback)
{
    napi_valuetype valuetype;
    size_t strLen = 0;
    char str[STR_MAX_SIZE] = {0};

    // argv[0]:code
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &code));

    // argv[1]:data
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], str, STR_MAX_SIZE - 1, &strLen));
    data = str;

    // argv[2]:callback
    if (argc >= SET_CODE_AND_DATA_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[2], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[2], 1, &callback);
    }
    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoSetCodeAndData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetCodeAndData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoSetCodeAndData start");

    if (argc >= SET_CODE_AND_DATA_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoSetCodeAndData end");
}

napi_value SetCodeAndData(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("SetCodeAndData start");
    size_t argc = 3;
    napi_value argv[3];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    int code = 0;
    std::string data;
    if (ParseParametersBySetCodeAndData(env, argv, argc, code, data, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("SetCodeAndData: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoSetCodeAndData *asynccallbackinfo = new AsyncCallbackInfoSetCodeAndData{
        .env = env, .asyncWork = nullptr, .objectInfo = objectInfo, .code = code, .data = data};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoSetCodeAndData(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "setCodeAndData", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("SetCodeAndData napi_create_async_work start");
            AsyncCallbackInfoSetCodeAndData *asynccallbackinfo = (AsyncCallbackInfoSetCodeAndData *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asyncResult->SetCodeAndData(asynccallbackinfo->code, asynccallbackinfo->data);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("SetCodeAndData napi_create_async_work end");
            AsyncCallbackInfoSetCodeAndData *asynccallbackinfo = (AsyncCallbackInfoSetCodeAndData *)data;

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                NapiGetNull(env));

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByAbort(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= ABORT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoAbort(const napi_env &env, const size_t &argc, AsyncCallbackInfoAbort *&asynccallbackinfo,
    const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoAbort start");

    if (argc >= ABORT_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoAbort end");
}

napi_value AbortCommonEvent(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("Abort start");
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByAbort(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("Abort: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoAbort *asynccallbackinfo =
        new AsyncCallbackInfoAbort{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoAbort(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "abort", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("Abort napi_create_async_work start");
            AsyncCallbackInfoAbort *asynccallbackinfo = (AsyncCallbackInfoAbort *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asyncResult->AbortCommonEvent();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("Abort napi_create_async_work end");
            AsyncCallbackInfoAbort *asynccallbackinfo = (AsyncCallbackInfoAbort *)data;

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                NapiGetNull(env));

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByClearAbort(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= CLEAR_ABORT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoClearAbort(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoClearAbort *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoClearAbort start");

    if (argc >= CLEAR_ABORT_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoClearAbort end");
}

napi_value ClearAbortCommonEvent(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("ClearAbort start");
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByClearAbort(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("ClearAbort: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoClearAbort *asynccallbackinfo =
        new AsyncCallbackInfoClearAbort{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoClearAbort(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "clearAbort", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("ClearAbort napi_create_async_work start");
            AsyncCallbackInfoClearAbort *asynccallbackinfo = (AsyncCallbackInfoClearAbort *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asyncResult->ClearAbortCommonEvent();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("ClearAbort napi_create_async_work end");
            AsyncCallbackInfoClearAbort *asynccallbackinfo = (AsyncCallbackInfoClearAbort *)data;

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                NapiGetNull(env));

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByGetAbort(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= GET_ABORT_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoGetAbort(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetAbort *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoGetAbort start");

    if (argc >= GET_ABORT_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoGetAbort end");
}

napi_value GetAbortCommonEvent(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("GetAbort start");
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByGetAbort(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("GetAbort: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoGetAbort *asynccallbackinfo =
        new AsyncCallbackInfoGetAbort{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoGetAbort(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "getAbort", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("GetAbort napi_create_async_work start");
            AsyncCallbackInfoGetAbort *asynccallbackinfo = (AsyncCallbackInfoGetAbort *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asynccallbackinfo->abortEvent = asyncResult->GetAbortCommonEvent();
            } else {
                asynccallbackinfo->abortEvent = false;
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("GetAbort napi_create_async_work end");
            AsyncCallbackInfoGetAbort *asynccallbackinfo = (AsyncCallbackInfoGetAbort *)data;

            napi_value result = 0;
            napi_get_boolean(env, asynccallbackinfo->abortEvent, &result);
            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByFinish(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= FINISH_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoFinish(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoFinish *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    EVENT_LOGI("PaddingAsyncCallbackInfoFinish start");

    if (argc >= FINISH_MAX_PARA) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    EVENT_LOGI("PaddingAsyncCallbackInfoFinish end");
}

napi_value FinishCommonEvent(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("Finish start");
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    if (ParseParametersByFinish(env, argv, argc, callback) == nullptr) {
        return NapiGetNull(env);
    }

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    EVENT_LOGI("Finish: objectInfo = %{public}p", objectInfo);

    AsyncCallbackInfoFinish *asynccallbackinfo =
        new AsyncCallbackInfoFinish{.env = env, .asyncWork = nullptr, .objectInfo = objectInfo};

    napi_value promise = 0;
    PaddingAsyncCallbackInfoFinish(env, argc, asynccallbackinfo, callback, promise);

    napi_value resourceName;
    napi_create_string_latin1(env, "finish", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("Finish napi_create_async_work start");
            AsyncCallbackInfoFinish *asynccallbackinfo = (AsyncCallbackInfoFinish *)data;
            std::shared_ptr<AsyncCommonEventResult> asyncResult = GetAsyncResult(asynccallbackinfo->objectInfo);
            if (asyncResult) {
                asyncResult->FinishCommonEvent();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("Finish napi_create_async_work end");
            AsyncCallbackInfoFinish *asynccallbackinfo = (AsyncCallbackInfoFinish *)data;

            ReturnCallbackPromise(env,
                asynccallbackinfo->isCallback,
                asynccallbackinfo->callback,
                asynccallbackinfo->deferred,
                NapiGetNull(env));

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value GetSubscriberBySubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber)
{
    EVENT_LOGI("GetSubscriberBySubscribe start");

    SubscriberInstance *commonEventSubscriberPtr = nullptr;
    napi_unwrap(env, value, (void **)&commonEventSubscriberPtr);
    if (!commonEventSubscriberPtr) {
        EVENT_LOGI("GetSubscriberBySubscribe commonEventSubscriberPtr is null");
        return nullptr;
    }
    EVENT_LOGI("GetSubscriberBySubscribe commonEventSubscriberPtr = %{public}p start", commonEventSubscriberPtr);
    std::shared_ptr<SubscriberInstance> subscriberInstance(commonEventSubscriberPtr);
    subscriber = subscriberInstance;

    return NapiGetNull(env);
}

napi_value ParseParametersBySubscribe(const napi_env &env, const napi_value (&argv)[SUBSCRIBE_MAX_PARA],
    std::shared_ptr<SubscriberInstance> &subscriber, napi_ref &callback)
{
    EVENT_LOGI("ParseParametersBySubscribe start");

    napi_valuetype valuetype;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. Subscribe expected.");
    if (GetSubscriberBySubscribe(env, argv[0], subscriber) == nullptr) {
        return nullptr;
    }

    // argv[1]:callback
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
    napi_create_reference(env, argv[1], 1, &callback);

    return NapiGetNull(env);
}

napi_value Subscribe(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("Subscribe start");

    // Argument parsing
    size_t argc = SUBSCRIBE_MAX_PARA;
    napi_value argv[SUBSCRIBE_MAX_PARA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= SUBSCRIBE_MAX_PARA, "Wrong number of arguments");

    napi_ref callback = 0;
    std::shared_ptr<SubscriberInstance> subscriber;

    if (ParseParametersBySubscribe(env, argv, subscriber, callback) == nullptr) {
        return NapiGetNull(env);
    }

    AsyncCallbackInfoSubscribe *asynccallbackinfo =
        new AsyncCallbackInfoSubscribe{.env = env, .asyncWork = nullptr, .subscriber = nullptr};
    EVENT_LOGI("Subscribe new asynccallbackinfo = %{public}p", asynccallbackinfo);
    asynccallbackinfo->subscriber = subscriber;
    asynccallbackinfo->callback = callback;
    subscriberInstances[asynccallbackinfo->subscriber].asyncCallbackInfo.push_back(asynccallbackinfo);
    EVENT_LOGI("Subscribe  AsyncCallbackInfoSubscribe * asynccallbackinfo = %{public}p", asynccallbackinfo);

    napi_value resourceName;
    napi_create_string_latin1(env, "Subscribe", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("Subscribe napi_create_async_work start");
            AsyncCallbackInfoSubscribe *asynccallbackinfo = (AsyncCallbackInfoSubscribe *)data;
            asynccallbackinfo->subscriber->SetEnv(env);
            asynccallbackinfo->subscriber->SetCallbackRef(asynccallbackinfo->callback);
            CommonEventManager::SubscribeCommonEvent(asynccallbackinfo->subscriber);
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("Subscribe napi_create_async_work end");
            AsyncCallbackInfoSubscribe *asynccallbackinfo = (AsyncCallbackInfoSubscribe *)data;
            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));
    return NapiGetNull(env);
}

napi_value GetBundlenameByPublish(const napi_env &env, const napi_value &value, std::string &bundleName)
{
    EVENT_LOGI("GetBundlenameByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "bundleName", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "bundleName", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        bundleName = str;
    }

    return NapiGetNull(env);
}

napi_value GetDataByPublish(const napi_env &env, const napi_value &value, std::string &data)
{
    EVENT_LOGI("GetDataByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "data", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "data", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        data = str;
    }

    return NapiGetNull(env);
}

napi_value GetCodeByPublish(const napi_env &env, const napi_value &value, int &code)
{
    EVENT_LOGI("GetCodeByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "code", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "code", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &code);
    }

    return NapiGetNull(env);
}

napi_value GetSubscriberPermissionsByPublish(
    const napi_env &env, const napi_value &value, std::vector<std::string> &subscriberPermissions)
{
    EVENT_LOGI("GetSubscriberPermissionsByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool isArray = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "subscriberPermissions", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "subscriberPermissions", &result);
        napi_is_array(env, result, &isArray);
        if (isArray) {
            uint32_t length = 0;
            napi_get_array_length(env, result, &length);
            if (length > 0) {
                for (uint32_t i = 0; i < length; ++i) {
                    napi_value nSubscriberPermission;
                    napi_get_element(env, result, i, &nSubscriberPermission);
                    NAPI_CALL(env, napi_typeof(env, nSubscriberPermission, &valuetype));
                    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
                    memset_s(str, STR_MAX_SIZE, 0, STR_MAX_SIZE);
                    NAPI_CALL(
                        env, napi_get_value_string_utf8(env, nSubscriberPermission, str, STR_MAX_SIZE - 1, &strLen));
                    subscriberPermissions.push_back(str);
                }
            }
        }
    }

    return NapiGetNull(env);
}

napi_value GetIsOrderedByPublish(const napi_env &env, const napi_value &value, bool &isOrdered)
{
    EVENT_LOGI("GetIsOrderedByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isOrdered", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isOrdered", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean expected.");
        napi_get_value_bool(env, result, &isOrdered);
    }

    return NapiGetNull(env);
}

napi_value GetIsStickyByPublish(const napi_env &env, const napi_value &value, bool &isSticky)
{
    EVENT_LOGI("GetIsStickyByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isSticky", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isSticky", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean expected.");
        napi_get_value_bool(env, result, &isSticky);
    }

    return NapiGetNull(env);
}

napi_value ParseParametersByPublish(const napi_env &env, const napi_value (&argv)[PUBLISH_MAX_PARA_BY_PUBLISHDATA],
    const size_t &argc, std::string &event, CommonEventPublishDataByjs &commonEventPublishData, napi_ref &callback)
{
    EVENT_LOGI("ParseParametersByPublish start");

    napi_valuetype valuetype;
    // argv[0]: event
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE - 1, &strLen);
    event = str;

    // argv[1]: CommonEventPublishData
    if (argc == PUBLISH_MAX_PARA_BY_PUBLISHDATA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

        // argv[1]: CommonEventPublishData:bundlename
        if (GetBundlenameByPublish(env, argv[1], commonEventPublishData.bundleName) == nullptr) {
            return nullptr;
        }
        // argv[1]: CommonEventPublishData:data
        if (GetDataByPublish(env, argv[1], commonEventPublishData.data) == nullptr) {
            return nullptr;
        }
        // argv[1]: CommonEventPublishData:code
        if (GetCodeByPublish(env, argv[1], commonEventPublishData.code) == nullptr) {
            return nullptr;
        }
        // argv[1]: CommonEventPublishData:permissions
        if (GetSubscriberPermissionsByPublish(env, argv[1], commonEventPublishData.subscriberPermissions) == nullptr) {
            return nullptr;
        }
        // argv[1]: CommonEventPublishData:isOrdered
        if (GetIsOrderedByPublish(env, argv[1], commonEventPublishData.isOrdered) == nullptr) {
            return nullptr;
        }
        // argv[1]: CommonEventPublishData:isSticky
        if (GetIsStickyByPublish(env, argv[1], commonEventPublishData.isSticky) == nullptr) {
            return nullptr;
        }
    }

    // argv[2]: callback
    if (argc == PUBLISH_MAX_PARA_BY_PUBLISHDATA) {
        NAPI_CALL(env, napi_typeof(env, argv[PUBLISH_MAX_PARA], &valuetype));
        NAPI_ASSERT(env, valuetype = napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[PUBLISH_MAX_PARA], 1, &callback);
    } else {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype = napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingCallbackInfoPublish(Want &want, AsyncCallbackInfoPublish *&asynccallbackinfo,
    const CommonEventPublishDataByjs &commonEventPublishDatajs)
{
    EVENT_LOGI("PaddingCallbackInfoPublish start");

    want.SetBundle(commonEventPublishDatajs.bundleName);
    asynccallbackinfo->commonEventData.SetCode(commonEventPublishDatajs.code);
    asynccallbackinfo->commonEventData.SetData(commonEventPublishDatajs.data);
    asynccallbackinfo->commonEventPublishInfo.SetSubscriberPermissions(commonEventPublishDatajs.subscriberPermissions);
    asynccallbackinfo->commonEventPublishInfo.SetOrdered(commonEventPublishDatajs.isOrdered);
    asynccallbackinfo->commonEventPublishInfo.SetSticky(commonEventPublishDatajs.isSticky);
}

napi_value Publish(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("Publish start");

    size_t argc = PUBLISH_MAX_PARA_BY_PUBLISHDATA;
    napi_value argv[PUBLISH_MAX_PARA_BY_PUBLISHDATA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= PUBLISH_MAX_PARA, "Wrong number of arguments");

    std::string event;
    CommonEventPublishDataByjs commonEventPublishDatajs;
    napi_ref callback = 0;

    if (ParseParametersByPublish(env, argv, argc, event, commonEventPublishDatajs, callback) == nullptr) {
        return NapiGetNull(env);
    }

    AsyncCallbackInfoPublish *asynccallbackinfo = new AsyncCallbackInfoPublish{.env = env, .asyncWork = nullptr};

    asynccallbackinfo->callback = callback;

    // CommonEventData::want->action
    Want want;
    want.SetAction(event);
    if (argc == PUBLISH_MAX_PARA_BY_PUBLISHDATA) {
        PaddingCallbackInfoPublish(want, asynccallbackinfo, commonEventPublishDatajs);
    }
    asynccallbackinfo->commonEventData.SetWant(want);

    napi_value resourceName;
    napi_create_string_latin1(env, "Publish", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("Publish napi_create_async_work start");
            AsyncCallbackInfoPublish *asynccallbackinfo = (AsyncCallbackInfoPublish *)data;
            CommonEventManager::PublishCommonEvent(
                asynccallbackinfo->commonEventData, asynccallbackinfo->commonEventPublishInfo);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoPublish *asynccallbackinfo = (AsyncCallbackInfoPublish *)data;

            SetCallback(env, asynccallbackinfo->callback, NapiGetNull(env));
            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    return NapiGetNull(env);
}

napi_value GetSubscriberByUnsubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber, bool &isFind)
{
    EVENT_LOGI("GetSubscriberByUnsubscribe start");

    SubscriberInstance *commonEventSubscriberPtr = nullptr;
    napi_unwrap(env, value, (void **)&commonEventSubscriberPtr);
    if (!commonEventSubscriberPtr) {
        EVENT_LOGI("GetSubscriberByUnsubscribe commonEventSubscriberPtr is null");
        return nullptr;
    }
    EVENT_LOGI("GetSubscriberByUnsubscribe:commonEventSubscriberPtr = %{public}p", commonEventSubscriberPtr);

    for (auto subscriberInstance : subscriberInstances) {
        EVENT_LOGI("GetSubscriberByUnsubscribe get = %{public}p", subscriberInstance.first.get());
        if (subscriberInstance.first.get() == commonEventSubscriberPtr) {
            subscriber = subscriberInstance.first;
            isFind = true;
            break;
        }
    }

    return NapiGetNull(env);
}

napi_value ParseParametersByUnsubscribe(const napi_env &env, const size_t &argc,
    const napi_value (&argv)[UNSUBSCRIBE_MAX_PARA], std::shared_ptr<SubscriberInstance> &subscriber, napi_ref &callback)
{
    EVENT_LOGI("ParseParametersByUnsubscribe start");

    napi_valuetype valuetype;
    napi_value result;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. Subscribe expected.");
    bool isFind = false;
    if (GetSubscriberByUnsubscribe(env, argv[0], subscriber, isFind) == nullptr) {
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= UNSUBSCRIBE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    napi_get_boolean(env, isFind, &result);

    return result;
}

void NapiDeleteSubscribe(std::shared_ptr<SubscriberInstance> &subscriber)
{
    EVENT_LOGI("NapiDeleteSubscribe start");

    auto subscribe = subscriberInstances.find(subscriber);
    if (subscribe != subscriberInstances.end()) {
        EVENT_LOGI("NapiDeleteSubscribe size = %{public}zu", subscribe->second.asyncCallbackInfo.size());
        for (auto asyncCallbackInfoSubscribe : subscribe->second.asyncCallbackInfo) {
            EVENT_LOGI("NapiDeleteSubscribe ptr = %{public}p", asyncCallbackInfoSubscribe);
            delete asyncCallbackInfoSubscribe;
            asyncCallbackInfoSubscribe = nullptr;
        }
        subscriberInstances.erase(subscribe);
    }
}

napi_value Unsubscribe(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("Unsubscribe start");

    // Argument parsing
    size_t argc = UNSUBSCRIBE_MAX_PARA;
    napi_value argv[UNSUBSCRIBE_MAX_PARA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_ref callback = 0;
    std::shared_ptr<SubscriberInstance> subscriber;
    napi_value result;
    result = ParseParametersByUnsubscribe(env, argc, argv, subscriber, callback);
    if (result == nullptr) {
        return NapiGetNull(env);
    }
    bool isFind = false;
    napi_get_value_bool(env, result, &isFind);
    if (!isFind) {
        EVENT_LOGI("Unsubscribe failed. The current subscriber does not exist");
        return NapiGetNull(env);
    }

    AsyncCallbackInfoUnsubscribe *asynccallback =
        new AsyncCallbackInfoUnsubscribe{.env = env, .asyncWork = nullptr, .subscriber = nullptr};

    asynccallback->subscriber = subscriber;
    asynccallback->argc = argc;
    if (argc >= UNSUBSCRIBE_MAX_PARA) {
        asynccallback->callback = callback;
    }

    napi_value resourceName;
    napi_create_string_latin1(env, "Unsubscribe", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            EVENT_LOGI("Unsubscribe napi_create_async_work start");
            AsyncCallbackInfoUnsubscribe *asynccallbackinfo = (AsyncCallbackInfoUnsubscribe *)data;

            CommonEventManager::UnSubscribeCommonEvent(asynccallbackinfo->subscriber);
        },
        [](napi_env env, napi_status status, void *data) {
            EVENT_LOGI("Subscribe napi_create_async_work end");
            AsyncCallbackInfoUnsubscribe *asynccallbackinfo = (AsyncCallbackInfoUnsubscribe *)data;

            if (asynccallbackinfo->argc >= UNSUBSCRIBE_MAX_PARA) {
                napi_value result = 0;
                napi_get_null(env, &result);
                SetCallback(env, asynccallbackinfo->callback, result);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);

            NapiDeleteSubscribe(asynccallbackinfo->subscriber);

            if (asynccallbackinfo) {
                EVENT_LOGI("delete asynccallbackinfo");
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallback,
        &asynccallback->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallback->asyncWork));
    return NapiGetNull(env);
}

napi_value GetEventsByCreateSubscriber(const napi_env &env, const napi_value &argv, std::vector<std::string> &events)
{
    EVENT_LOGI("GetEventsByCreateSubscriber start");
    napi_valuetype valuetype;
    bool hasProperty = false;
    bool isArray = false;
    size_t strLen = 0;
    uint32_t length = 0;
    // events
    NAPI_CALL(env, napi_has_named_property(env, argv, "events", &hasProperty));
    if (hasProperty) {
        napi_value eventsNapi;
        napi_get_named_property(env, argv, "events", &eventsNapi);
        napi_is_array(env, eventsNapi, &isArray);
        if (isArray) {
            napi_get_array_length(env, eventsNapi, &length);
            NAPI_ASSERT(env, length > 0, "The array is empty.");
            for (size_t i = 0; i < length; i++) {
                napi_value event;
                napi_get_element(env, eventsNapi, i, &event);
                NAPI_CALL(env, napi_typeof(env, event, &valuetype));
                NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
                char str[STR_MAX_SIZE] = {0};
                NAPI_CALL(env, napi_get_value_string_utf8(env, event, str, STR_MAX_SIZE - 1, &strLen));
                EVENT_LOGI("GetEventsByCreateSubscriber str = %{public}s", str);
                events.push_back(str);
            }
        }
    }

    return NapiGetNull(env);
}

napi_value GetPublisherPermissionByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &permission, bool &hasProperty)
{
    EVENT_LOGI("GetPublisherPermissionByCreateSubscriber start");

    napi_valuetype valuetype;
    size_t strLen = 0;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};

    // publisherPermission
    NAPI_CALL(env, napi_has_named_property(env, argv, "publisherPermission", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, argv, "publisherPermission", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        permission = str;
    }

    return NapiGetNull(env);
}

napi_value GetPublisherDeviceIdByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &publisherDeviceId, bool &hasProperty)
{
    EVENT_LOGI("GetPublisherDeviceIdByCreateSubscriber start");

    napi_valuetype valuetype;
    size_t strLen = 0;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};

    // publisherDeviceId
    NAPI_CALL(env, napi_has_named_property(env, argv, "publisherDeviceId", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, argv, "publisherDeviceId", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        publisherDeviceId = str;
    }

    return NapiGetNull(env);
}

napi_value GetPriorityByCreateSubscriber(const napi_env &env, const napi_value &argv, int &priority, bool &hasProperty)
{
    EVENT_LOGI("GetPriorityByCreateSubscriber start");

    napi_valuetype valuetype;
    napi_value result;
    int32_t value;

    // priority
    NAPI_CALL(env, napi_has_named_property(env, argv, "priority", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, argv, "priority", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        NAPI_CALL(env, napi_get_value_int32(env, result, &value));
        priority = value;
    }

    return NapiGetNull(env);
}

napi_value CommonEventSubscriberConstructor(napi_env env, napi_callback_info info)
{
    EVENT_LOGI("CommonEventSubscriberConstructor start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    // argv[0]:CommonEventSubscribeInfo:events
    std::vector<std::string> events;
    if (GetEventsByCreateSubscriber(env, argv[0], events) == nullptr) {
        return nullptr;
    }

    std::string permission;
    bool hasPermission = false;
    if (GetPublisherPermissionByCreateSubscriber(env, argv[0], permission, hasPermission) == nullptr) {
        return nullptr;
    }

    std::string publisherDeviceId;
    bool hasPublisherDeviceId = false;
    if (GetPublisherDeviceIdByCreateSubscriber(env, argv[0], publisherDeviceId, hasPublisherDeviceId) == nullptr) {
        return nullptr;
    }

    int priority;
    bool hasPriority = false;
    if (GetPriorityByCreateSubscriber(env, argv[0], priority, hasPriority) == nullptr) {
        return nullptr;
    }

    MatchingSkills matchingSkills;
    for (const auto &event : events) {
        matchingSkills.AddEvent(event);
    }

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (hasPermission) {
        subscriberInfo.SetPermission(permission);
    }

    if (hasPublisherDeviceId) {
        subscriberInfo.SetDeviceId(publisherDeviceId);
    }

    if (hasPriority) {
        subscriberInfo.SetPriority(priority);
    }

    SubscriberInstance *objectInfo = new SubscriberInstance(subscriberInfo);
    EVENT_LOGI("CommonEventSubscriberConstructor  objectInfo = %{public}p", objectInfo);

    napi_wrap(
        env,
        thisVar,
        objectInfo,
        [](napi_env env, void *data, void *hint) {
            SubscriberInstance *objectInfo = (SubscriberInstance *)data;
            EVENT_LOGI("CommonEventSubscriberConstructor this = %{public}p, destruct", objectInfo);

            for (auto subscriberInstance : subscriberInstances) {
                EVENT_LOGI("CommonEventSubscriberConstructor get = %{public}p", subscriberInstance.first.get());
                if (subscriberInstance.first.get() == objectInfo) {
                    for (auto asyncCallbackInfo : subscriberInstance.second.asyncCallbackInfo) {
                        EVENT_LOGI("CommonEventSubscriberConstructor ptr = %{public}p", asyncCallbackInfo);
                        delete asyncCallbackInfo;
                        asyncCallbackInfo = nullptr;
                    }
                    subscriberInstances.erase(subscriberInstance.first);
                    break;
                }
            }

            if (objectInfo) {
                delete objectInfo;
                objectInfo = nullptr;
            }
        },
        nullptr,
        nullptr);

    EVENT_LOGI("CommonEventSubscriberConstructor end");
    return thisVar;
}

napi_value CommonEventSubscriberInit(napi_env env, napi_value exports)
{
    EVENT_LOGI("enter");

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSubscribeInfo", GetSubscribeInfo),
        DECLARE_NAPI_FUNCTION("isOrderedCommonEvent", IsOrderedCommonEvent),
        DECLARE_NAPI_FUNCTION("isStickyCommonEvent", IsStickyCommonEvent),
        DECLARE_NAPI_FUNCTION("getCode", GetCode),
        DECLARE_NAPI_FUNCTION("setCode", SetCode),
        DECLARE_NAPI_FUNCTION("getData", GetData),
        DECLARE_NAPI_FUNCTION("setData", SetData),
        DECLARE_NAPI_FUNCTION("setCodeAndData", SetCodeAndData),
        DECLARE_NAPI_FUNCTION("abortCommonEvent", AbortCommonEvent),
        DECLARE_NAPI_FUNCTION("clearAbortCommonEvent", ClearAbortCommonEvent),
        DECLARE_NAPI_FUNCTION("getAbortCommonEvent", GetAbortCommonEvent),
        DECLARE_NAPI_FUNCTION("finishCommonEvent", FinishCommonEvent),
    };

    NAPI_CALL(env,
        napi_define_class(env,
            "commonEventSubscriber",
            NAPI_AUTO_LENGTH,
            CommonEventSubscriberConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            &g_CommonEventSubscriber));

    return exports;
}

napi_value CommonEventInit(napi_env env, napi_value exports)
{
    EVENT_LOGI("enter");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("publish", Publish),
        DECLARE_NAPI_FUNCTION("createSubscriber", CreateSubscriber),
        DECLARE_NAPI_FUNCTION("subscribe", Subscribe),
        DECLARE_NAPI_FUNCTION("unsubscribe", Unsubscribe),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    OHOS::EventFwkNapi::SupportInit(env, exports);
    return exports;
}
}  // namespace EventFwkNapi
}  // namespace OHOS
