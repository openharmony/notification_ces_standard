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
#include <map>
#include <string>
#include <uv.h>
#include <unistd.h>

#include "common_event_manager.h"
#include "hilog_wrapper.h"
#include "securec.h"
#include "want.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

using namespace OHOS::EventFwk;

static const std::int32_t STR_MAX_SIZE = 64;
static const std::int32_t CREATE_MAX_PARA = 2;
static const std::int32_t SUBSCRIBE_MAX_PARA = 2;
static const std::int32_t UNSUBSCRIBE_MAX_PARA = 2;
static const std::int32_t PUBLISH_MAX_PARA_BY_PUBLISHDATA = 3;
static const std::int32_t PUBLISH_MAX_PARA = 2;
static const std::int32_t SUBSCRIBER_AND_CALLBACK_EQUAL = 0;
static const std::int32_t GETSUBSCREBEINFO_MAX_PARA = 1;
static const std::int32_t ISORDEREDCOMMONEVENT_MAX_PARA = 1;
static const std::int32_t ISSTICKYCOMMONEVENT_MAX_PARA = 1;
static const std::int32_t ARGS_TWO = 2;
static const std::int32_t PARAM0 = 0;
static const std::int32_t PARAM1 = 1;
static const std::int32_t NO_ERROR = 0;

class SubscriberInstance;
struct AsyncCallbackInfoSubscribe;

static napi_value g_CommonEventSubscriber;
static std::map<std::shared_ptr<SubscriberInstance>, napi_ref> subscriberInstances;
static std::map<std::shared_ptr<SubscriberInstance>, std::vector<AsyncCallbackInfoSubscribe *>>
    commonEventSubscriberInstances;

struct AsyncCallbackInfoSubscribe
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    std::shared_ptr<SubscriberInstance> subscriber;
};

struct AsyncCallbackInfoUnsubscribe
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    size_t argc;
    std::shared_ptr<SubscriberInstance> subscriber;
};

struct AsyncCallbackInfoSubscribeInfo
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    napi_deferred deferred;
    SubscriberInstance *objectInfo;
    std::vector<std::string> events;
    std::string permission;
    std::string deviceId;
    int32_t priority = 0;
    bool isCallback = false;
};

struct AsyncCallbackInfoOrderedCommonEvent
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    napi_deferred deferred;
    SubscriberInstance *objectInfo;
    bool isCallback = false;
    bool isOrdered = false;
};

struct AsyncCallbackInfoStickyCommonEvent
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    napi_deferred deferred;
    SubscriberInstance *objectInfo;
    bool isSticky = false;
    bool isCallback = false;
};

struct AsyncCallbackInfoCreate
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    napi_deferred deferred;
    napi_ref subscriberInfo;
    bool isCallback = false;
    napi_value result = 0;
};

struct AsyncCallbackInfoPublish
{
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    CommonEventData commonEventData;
    CommonEventPublishInfo commonEventPublishInfo;
};

struct CommonEventSubscribeInfoByjs
{
    std::vector<std::string> events;
    std::string publisherPermission;
    std::string publisherDeviceId;
    int userId;
    int priority;
};

struct CommonEventPublishDataByjs
{
    std::string bundleName;
    std::string data;
    int code;
    std::vector<std::string> subscriberPermissions;
    bool isOrdered;
    bool isSticky;
};

struct CommonEventDataWorker
{
    Want want;
    int code;
    std::string data;
    napi_env env;
    napi_ref ref;
};

static napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

class SubscriberInstance : public CommonEventSubscriber {
public:
    SubscriberInstance(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
    }

    ~SubscriberInstance()
    {
    }

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        HILOG_INFO("OnReceiveEvent start");
        uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
        napi_get_uv_event_loop(env_, &loop);
#endif // NAPI_VERSION >= 2

        uv_work_t *work = new uv_work_t;

        CommonEventDataWorker *commonEventDataWorker = new CommonEventDataWorker();
        commonEventDataWorker->want = data.GetWant();
        HILOG_INFO("OnReceiveEvent() action = %{public}s", data.GetWant().GetAction().c_str());
        commonEventDataWorker->code = data.GetCode();
        commonEventDataWorker->data = data.GetData();
        commonEventDataWorker->env = env_;
        commonEventDataWorker->ref = ref_;

        work->data = (void *)commonEventDataWorker;

        uv_queue_work(
            loop,
            work,
            [](uv_work_t *work) {},
            [](uv_work_t *work, int status)
            {
                HILOG_INFO("OnReceiveEvent uv_work_t start");
                CommonEventDataWorker *commonEventDataWorkerData = (CommonEventDataWorker *)work->data;
                if (commonEventDataWorkerData == nullptr)
                {
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

        HILOG_INFO("OnReceiveEvent end");
    }

public:
    void SetEnv(const napi_env &env)
    {
        env_ = env;
    }

    void SetCallbackRef(const napi_ref &ref)
    {
        ref_ = ref;
    }

private:
    napi_env env_;
    napi_ref ref_;
};

static napi_value ParseParametersByCreateSubscriber(const napi_env &env, const napi_value (&argv)[CREATE_MAX_PARA],
                                                    const size_t &argc, CommonEventSubscribeInfoByjs &subscribeInfo, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:CommonEventSubscribeInfo
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. object expected.");

    // argv[1]:callback
    if (argc >= CREATE_MAX_PARA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype = napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return nullptr;
}

static void SetCallback(const napi_env &env, const napi_ref &callbackIn, napi_value &result)
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

static void SetPromise(const napi_env &env, const napi_deferred &deferred, napi_value &result)
{
    napi_resolve_deferred(env, deferred, result);
}

static void ReturnCallbackPromise(const napi_env &env, const bool &isCallback, const napi_ref &callback,
                                  const napi_deferred &deferred, napi_value &result)
{
    HILOG_INFO("ReturnCallbackPromise start");

    if (isCallback)
    {
        SetCallback(env, callback, result);
    }
    else
    {
        SetPromise(env, deferred, result);
    }
}

static napi_value PaddingAsyncCallbackInfoCreateSubscriber(const napi_env &env, const size_t &argc,
                                                           AsyncCallbackInfoCreate *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    HILOG_INFO("PaddingAsyncCallbackInfoCreateSubscriber start");

    if (argc >= CREATE_MAX_PARA)
    {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    }
    else
    {
        napi_deferred deferred = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    return nullptr;
}

static napi_value CreateSubscriber(napi_env env, napi_callback_info info)
{
    HILOG_INFO("CreateSubscriber start");

    size_t argc = CREATE_MAX_PARA;
    napi_value argv[CREATE_MAX_PARA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));

    CommonEventSubscribeInfoByjs subscribeInfoByjs;
    napi_ref callback = 0;
    ParseParametersByCreateSubscriber(env, argv, argc, subscribeInfoByjs, callback);

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
        [](napi_env env, void *data)
        { HILOG_INFO("CreateSubscriber napi_create_async_work start"); },
        [](napi_env env, napi_status status, void *data)
        {
            HILOG_INFO("CreateSubscriber napi_create_async_work end");
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
            if (asynccallbackinfo)
            {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return nullptr;
    }
    else
    {
        return promise;
    }
}

static napi_value ParseParametersByGetSubscribeInfo(
    const napi_env &env, const size_t &argc, const napi_value (&argv)[1], napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= GETSUBSCREBEINFO_MAX_PARA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return nullptr;
}

static void SetEventsResult(
    const napi_env &env, const std::vector<std::string> &events, napi_value &commonEventSubscribeInfo)
{
    HILOG_INFO("SetEventsResult start");

    napi_value value = nullptr;

    if (events.size() > 0)
    {
        napi_value nEvents;
        if (napi_create_array(env, &nEvents) != napi_ok)
        {
            return;
        }
        size_t index = 0;
        for (auto event : events)
        {
            napi_create_string_utf8(env, event.c_str(), NAPI_AUTO_LENGTH, &value);
            napi_set_element(env, nEvents, index, value);
            index++;
        }

        napi_set_named_property(env, commonEventSubscribeInfo, "events", nEvents);
    }

    HILOG_INFO("SetEventsResult end");
}

static void SetPublisherPermissionResult(
    const napi_env &env, const std::string &permission, napi_value &commonEventSubscribeInfo)
{
    HILOG_INFO("SetPublisherPermissionResult start");

    napi_value value;
    napi_create_string_utf8(env, permission.c_str(), NAPI_AUTO_LENGTH, &value);

    napi_set_named_property(env, commonEventSubscribeInfo, "publisherPermission", value);
}

static void SetPublisherDeviceIdResult(
    const napi_env &env, const std::string &deviceId, napi_value &commonEventSubscribeInfo)
{
    HILOG_INFO("SetPublisherDeviceIdResult start");

    napi_value value;
    napi_create_string_utf8(env, deviceId.c_str(), NAPI_AUTO_LENGTH, &value);

    napi_set_named_property(env, commonEventSubscribeInfo, "publisherDeviceId", value);
}

static void SetPublisherPriorityResult(
    const napi_env &env, const int32_t &priority, napi_value &commonEventSubscribeInfo)
{
    HILOG_INFO("SetPublisherPriorityResult start");

    napi_value value;
    napi_create_int32(env, priority, &value);

    napi_set_named_property(env, commonEventSubscribeInfo, "priority", value);
}

static napi_value PaddingAsyncCallbackInfoGetSubscribeInfo(const napi_env &env, const size_t &argc,
                                                           AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    HILOG_INFO("PaddingAsyncCallbackInfoGetSubscribeInfo start");

    if (argc >= GETSUBSCREBEINFO_MAX_PARA)
    {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    }
    else
    {
        napi_deferred deferred = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    return nullptr;
}

static void PaddingNapiCreateAsyncWorkCallbackInfo(
    AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, SubscriberInstance *&subscriber)
{
    HILOG_INFO("PaddingNapiCreateAsyncWorkCallbackInfo start");

    asynccallbackinfo->events = subscriber->GetSubscribeInfo().GetMatchingSkills().GetEvents();
    asynccallbackinfo->permission = subscriber->GetSubscribeInfo().GetPermission();
    asynccallbackinfo->deviceId = subscriber->GetSubscribeInfo().GetDeviceId();
    asynccallbackinfo->priority = subscriber->GetSubscribeInfo().GetPriority();
}

static void SetNapiResult(
    const napi_env &env, const AsyncCallbackInfoSubscribeInfo *asynccallbackinfo, napi_value &result)
{
    HILOG_INFO("SetNapiResult start");

    SetEventsResult(env, asynccallbackinfo->events, result);
    SetPublisherPermissionResult(env, asynccallbackinfo->permission, result);
    SetPublisherDeviceIdResult(env, asynccallbackinfo->deviceId, result);
    SetPublisherPriorityResult(env, asynccallbackinfo->priority, result);
}

static napi_value GetSubscribeInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("GetSubscribeInfo start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    ParseParametersByGetSubscribeInfo(env, argc, argv, callback);

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("GetSubscribeInfo objectInfo = %{public}p", objectInfo);

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
        [](napi_env env, void *data)
        {
            HILOG_INFO("GetSubscribeInfo napi_create_async_work start");
            AsyncCallbackInfoSubscribeInfo *asynccallbackinfo = (AsyncCallbackInfoSubscribeInfo *)data;

            PaddingNapiCreateAsyncWorkCallbackInfo(asynccallbackinfo, asynccallbackinfo->objectInfo);
        },
        [](napi_env env, napi_status status, void *data)
        {
            HILOG_INFO("GetSubscribeInfo napi_create_async_work end");
            AsyncCallbackInfoSubscribeInfo *asynccallbackinfo = (AsyncCallbackInfoSubscribeInfo *)data;

            napi_value result = 0;
            napi_create_object(env, &result);
            SetNapiResult(env, asynccallbackinfo, result);

            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo)
            {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return nullptr;
    }
    else
    {
        return promise;
    }
}

static napi_value ParseParametersByIsOrderedCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= ISORDEREDCOMMONEVENT_MAX_PARA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return nullptr;
}

static napi_value PaddingAsyncCallbackInfoIsOrderedCommonEvent(const napi_env &env, const size_t &argc,
                                                               AsyncCallbackInfoOrderedCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    HILOG_INFO("PaddingAsyncCallbackInfoIsOrderedCommonEvent start");

    if (argc >= ISORDEREDCOMMONEVENT_MAX_PARA)
    {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    }
    else
    {
        napi_deferred deferred = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    return nullptr;
}

static napi_value IsOrderedCommonEvent(napi_env env, napi_callback_info info)
{
    HILOG_INFO("IsOrderedCommonEvent start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    napi_ref callback = 0;
    ParseParametersByIsOrderedCommonEvent(env, argv, argc, callback);

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("IsOrderedCommonEvent objectInfo = %{public}p", objectInfo);

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
        [](napi_env env, void *data)
        {
            HILOG_INFO("IsOrderedCommonEvent napi_create_async_work start");
            AsyncCallbackInfoOrderedCommonEvent *asynccallbackinfo = (AsyncCallbackInfoOrderedCommonEvent *)data;

            asynccallbackinfo->isOrdered = asynccallbackinfo->objectInfo->IsOrderedCommonEvent();
        },
        [](napi_env env, napi_status status, void *data)
        {
            HILOG_INFO("IsOrderedCommonEvent napi_create_async_work end");
            AsyncCallbackInfoOrderedCommonEvent *asynccallbackinfo = (AsyncCallbackInfoOrderedCommonEvent *)data;

            napi_value result = 0;
            napi_create_object(env, &result);
            napi_get_boolean(env, asynccallbackinfo->isOrdered, &result);

            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo)
            {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback)
    {
        return nullptr;
    }
    else
    {
        return promise;
    }
}

static napi_value ParseParametersByIsStickyCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback)
{
    napi_valuetype valuetype;

    // argv[0]:callback
    if (argc >= ISSTICKYCOMMONEVENT_MAX_PARA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return nullptr;
}

static napi_value PaddingAsyncCallbackInfoIsStickyCommonEvent(const napi_env &env, const size_t &argc,
                                                              AsyncCallbackInfoStickyCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    HILOG_INFO("PaddingAsyncCallbackInfoIsStickyCommonEvent start");

    if (argc >= ISSTICKYCOMMONEVENT_MAX_PARA)
    {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    }
    else
    {
        napi_deferred deferred = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }

    HILOG_INFO("PaddingAsyncCallbackInfoIsStickyCommonEvent end");
    return nullptr;
}

static napi_value IsStickyCommonEvent(napi_env env, napi_callback_info info)
{
    HILOG_INFO("IsStickyCommonEvent start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_ref callback = 0;
    ParseParametersByIsStickyCommonEvent(env, argv, argc, callback);

    SubscriberInstance *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("IsStickyCommonEvent: objectInfo = %{public}p", objectInfo);

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
        [](napi_env env, void *data)
        {
            HILOG_INFO("isStickyCommonEvent napi_create_async_work start");
            AsyncCallbackInfoStickyCommonEvent *asynccallbackinfo = (AsyncCallbackInfoStickyCommonEvent *)data;

            asynccallbackinfo->isSticky = asynccallbackinfo->objectInfo->IsStickyCommonEvent();
        },
        [](napi_env env, napi_status status, void *data)
        {
            HILOG_INFO("isStickyCommonEvent napi_create_async_work end");
            AsyncCallbackInfoStickyCommonEvent *asynccallbackinfo = (AsyncCallbackInfoStickyCommonEvent *)data;

            napi_value result = 0;
            napi_create_object(env, &result);
            napi_get_boolean(env, asynccallbackinfo->isSticky, &result);
            ReturnCallbackPromise(
                env, asynccallbackinfo->isCallback, asynccallbackinfo->callback, asynccallbackinfo->deferred, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo)
            {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return nullptr;
    }
    else
    {
        return promise;
    }
}

static napi_value GetSubscriberBySubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber)
{
    HILOG_INFO("GetSubscriberBySubscribe start");

    SubscriberInstance *commonEventSubscriberPtr = nullptr;
    napi_unwrap(env, value, (void **)&commonEventSubscriberPtr);
    if (!commonEventSubscriberPtr) {
        HILOG_INFO("GetSubscriberBySubscribe commonEventSubscriberPtr is null");
        return nullptr;
    }
    HILOG_INFO("GetSubscriberBySubscribe commonEventSubscriberPtr = %{public}p start", commonEventSubscriberPtr);
    std::shared_ptr<SubscriberInstance> subscriberInstance(commonEventSubscriberPtr);
    subscriber = subscriberInstance;

    return nullptr;
}

static napi_value ParseParametersBySubscribe(const napi_env &env, const napi_value (&argv)[SUBSCRIBE_MAX_PARA],
                                             std::shared_ptr<SubscriberInstance> &subscriber, napi_ref &callback)
{
    HILOG_INFO("ParseParametersBySubscribe start");

    napi_valuetype valuetype;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. Subscribe expected.");
    GetSubscriberBySubscribe(env, argv[0], subscriber);

    // argv[1]:callback
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
    napi_create_reference(env, argv[1], 1, &callback);

    napi_value result = nullptr;
    auto subscriberInstance = subscriberInstances.find(subscriber);
    if (subscriberInstance != subscriberInstances.end())
    {
        if (subscriberInstance->second == callback)
        {
            HILOG_INFO("subcribe and callback is equal");
            napi_create_int32(env, 0, &result);
        }
        else
        {
            HILOG_INFO("subcribe is equal and callback is not");
            subscriberInstances[subscriber] = callback;
            napi_create_int32(env, 1, &result);
        }
    }
    else
    {
        HILOG_INFO("subcribe and callback is not equal");
        subscriberInstances[subscriber] = callback;
        napi_create_int32(env, 2, &result);
    }

    return result;
}

static napi_value Subscribe(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Subscribe start");

    // Argument parsing
    size_t argc = SUBSCRIBE_MAX_PARA;
    napi_value argv[SUBSCRIBE_MAX_PARA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= SUBSCRIBE_MAX_PARA, "Wrong number of arguments");

    napi_ref callback = 0;
    std::shared_ptr<SubscriberInstance> subscriber;
    napi_value subscribeResult;
    subscribeResult = ParseParametersBySubscribe(env, argv, subscriber, callback);
    std::int32_t code;
    napi_get_value_int32(env, subscribeResult, &code);

    if (code == SUBSCRIBER_AND_CALLBACK_EQUAL)
    {
        return nullptr;
    }

    AsyncCallbackInfoSubscribe *asynccallbackinfo = new AsyncCallbackInfoSubscribe{.env = env, .asyncWork = nullptr};
    HILOG_INFO("Subscribe new asynccallbackinfo = %{public}p", asynccallbackinfo);
    asynccallbackinfo->subscriber = subscriber;
    asynccallbackinfo->callback = callback;
    commonEventSubscriberInstances[asynccallbackinfo->subscriber].push_back(asynccallbackinfo);
    HILOG_INFO("Subscribe  AsyncCallbackInfoSubscribe * asynccallbackinfo = %{public}p", asynccallbackinfo);

    napi_value resourceName;
    napi_create_string_latin1(env, "Subscribe", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data)
        {
            HILOG_INFO("Subscribe napi_create_async_work start");
            AsyncCallbackInfoSubscribe *asynccallbackinfo = (AsyncCallbackInfoSubscribe *)data;

            asynccallbackinfo->subscriber->SetEnv(env);
            asynccallbackinfo->subscriber->SetCallbackRef(asynccallbackinfo->callback);
            CommonEventManager::SubscribeCommonEvent(asynccallbackinfo->subscriber);
        },
        [](napi_env env, napi_status status, void *data)
        {
            HILOG_INFO("Subscribe napi_create_async_work end");
            AsyncCallbackInfoSubscribe *asynccallbackinfo = (AsyncCallbackInfoSubscribe *)data;
            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));
    return nullptr;
}

static napi_value GetBundlenameByPublish(const napi_env &env, const napi_value &value, std::string &bundleName)
{
    HILOG_INFO("GetBundlenameByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "bundleName", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, value, "bundleName", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        bundleName = str;
    }

    return nullptr;
}

static napi_value GetDataByPublish(const napi_env &env, const napi_value &value, std::string &data)
{
    HILOG_INFO("GetDataByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "data", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, value, "data", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        data = str;
    }

    return nullptr;
}

static napi_value GetCodeByPublish(const napi_env &env, const napi_value &value, int &code)
{
    HILOG_INFO("GetCodeByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "code", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, value, "code", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &code);
    }

    return nullptr;
}

static napi_value GetSubscriberPermissionsByPublish(
    const napi_env &env, const napi_value &value, std::vector<std::string> &subscriberPermissions)
{
    HILOG_INFO("GetSubscriberPermissionsByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool isArray = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "subscriberPermissions", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, value, "subscriberPermissions", &result);
        napi_is_array(env, result, &isArray);
        if (isArray)
        {
            uint32_t length = 0;
            napi_get_array_length(env, result, &length);
            if (length > 0)
            {
                for (uint32_t i = 0; i < length; ++i)
                {
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

    return nullptr;
}

static napi_value GetIsOrderedByPublish(const napi_env &env, const napi_value &value, bool &isOrdered)
{
    HILOG_INFO("GetIsOrderedByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isOrdered", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, value, "isOrdered", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean expected.");
        napi_get_value_bool(env, result, &isOrdered);
    }

    return nullptr;
}

static napi_value GetIsStickyByPublish(const napi_env &env, const napi_value &value, bool &isSticky)
{
    HILOG_INFO("GetIsStickyByPublish start");

    napi_valuetype valuetype;
    napi_value result;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isSticky", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, value, "isSticky", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean expected.");
        napi_get_value_bool(env, result, &isSticky);
    }

    return nullptr;
}

static napi_value ParseParametersByPublish(const napi_env &env,
                                           const napi_value (&argv)[PUBLISH_MAX_PARA_BY_PUBLISHDATA], const size_t &argc, std::string &event,
                                           CommonEventPublishDataByjs &commonEventPublishData, napi_ref &callback)
{
    HILOG_INFO("ParseParametersByPublish start");

    napi_valuetype valuetype;
    // argv[0]: event
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE - 1, &strLen);
    event = str;

    // argv[1]: CommonEventPublishData
    if (argc == PUBLISH_MAX_PARA_BY_PUBLISHDATA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

        // argv[1]: CommonEventPublishData:bundlename
        GetBundlenameByPublish(env, argv[1], commonEventPublishData.bundleName);
        // argv[1]: CommonEventPublishData:data
        GetDataByPublish(env, argv[1], commonEventPublishData.data);
        // argv[1]: CommonEventPublishData:code
        GetCodeByPublish(env, argv[1], commonEventPublishData.code);
        // argv[1]: CommonEventPublishData:permissions
        GetSubscriberPermissionsByPublish(env, argv[1], commonEventPublishData.subscriberPermissions);
        // argv[1]: CommonEventPublishData:isOrdered
        GetIsOrderedByPublish(env, argv[1], commonEventPublishData.isOrdered);
        // argv[1]: CommonEventPublishData:isSticky
        GetIsStickyByPublish(env, argv[1], commonEventPublishData.isSticky);
    }

    // argv[2]: callback
    if (argc == PUBLISH_MAX_PARA_BY_PUBLISHDATA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[PUBLISH_MAX_PARA], &valuetype));
        NAPI_ASSERT(env, valuetype = napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[PUBLISH_MAX_PARA], 1, &callback);
    }
    else
    {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype = napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return nullptr;
}

static void PaddingCallbackInfoPublish(Want &want, AsyncCallbackInfoPublish *&asynccallbackinfo,
                                       const CommonEventPublishDataByjs &commonEventPublishDatajs)
{
    HILOG_INFO("PaddingCallbackInfoPublish start");

    want.SetBundle(commonEventPublishDatajs.bundleName);
    asynccallbackinfo->commonEventData.SetCode(commonEventPublishDatajs.code);
    asynccallbackinfo->commonEventData.SetData(commonEventPublishDatajs.data);
    asynccallbackinfo->commonEventPublishInfo.SetSubscriberPermissions(commonEventPublishDatajs.subscriberPermissions);
    asynccallbackinfo->commonEventPublishInfo.SetOrdered(commonEventPublishDatajs.isOrdered);
    asynccallbackinfo->commonEventPublishInfo.SetSticky(commonEventPublishDatajs.isSticky);
}

static napi_value Publish(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Publish start");

    size_t argc = PUBLISH_MAX_PARA_BY_PUBLISHDATA;
    napi_value argv[PUBLISH_MAX_PARA_BY_PUBLISHDATA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= PUBLISH_MAX_PARA, "Wrong number of arguments");

    std::string event;
    CommonEventPublishDataByjs commonEventPublishDatajs;
    napi_ref callback = 0;

    ParseParametersByPublish(env, argv, argc, event, commonEventPublishDatajs, callback);

    AsyncCallbackInfoPublish *asynccallbackinfo = new AsyncCallbackInfoPublish{.env = env, .asyncWork = nullptr};

    asynccallbackinfo->callback = callback;

    // CommonEventData::want->action
    Want want;
    want.SetAction(event);
    if (argc == PUBLISH_MAX_PARA_BY_PUBLISHDATA)
    {
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
        [](napi_env env, void *data)
        {
            HILOG_INFO("Publish napi_create_async_work start");
            AsyncCallbackInfoPublish *asynccallbackinfo = (AsyncCallbackInfoPublish *)data;
            sleep(3);
            CommonEventManager::PublishCommonEvent(
                asynccallbackinfo->commonEventData, asynccallbackinfo->commonEventPublishInfo);
        },
        [](napi_env env, napi_status status, void *data)
        {
            AsyncCallbackInfoPublish *asynccallbackinfo = (AsyncCallbackInfoPublish *)data;
            napi_value result = 0;
            napi_get_null(env, &result);
            SetCallback(env, asynccallbackinfo->callback, result);
            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo)
            {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    return nullptr;
}

static napi_value GetSubscriberByUnsubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber, bool &isFind)
{
    HILOG_INFO("GetSubscriberByUnsubscribe start");

    SubscriberInstance *commonEventSubscriberPtr = nullptr;
    napi_unwrap(env, value, (void **)&commonEventSubscriberPtr);
    if (!commonEventSubscriberPtr)
    {
        HILOG_INFO("GetSubscriberByUnsubscribe commonEventSubscriberPtr is null");
        return nullptr;
    }
    HILOG_INFO("GetSubscriberByUnsubscribe:commonEventSubscriberPtr = %{public}p", commonEventSubscriberPtr);

    for (auto commonEventSubscriberInstance : commonEventSubscriberInstances)
    {
        HILOG_INFO("GetSubscriberByUnsubscribe get = %{public}p", commonEventSubscriberInstance.first.get());
        if (commonEventSubscriberInstance.first.get() == commonEventSubscriberPtr)
        {
            subscriber = commonEventSubscriberInstance.first;
            isFind = true;
            break;
        }
    }

    return nullptr;
}

static napi_value ParseParametersByUnsubscribe(const napi_env &env, const size_t &argc,
                                               const napi_value (&argv)[UNSUBSCRIBE_MAX_PARA], std::shared_ptr<SubscriberInstance> &subscriber, napi_ref &callback)
{
    HILOG_INFO("ParseParametersByUnsubscribe start");

    napi_valuetype valuetype;
    napi_value result;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. Subscribe expected.");
    bool isFind = false;
    GetSubscriberByUnsubscribe(env, argv[0], subscriber, isFind);

    // argv[1]:callback
    if (argc >= UNSUBSCRIBE_MAX_PARA)
    {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    napi_get_boolean(env, isFind, &result);

    return result;
}

static void NapiDeleteSubscribe(std::shared_ptr<SubscriberInstance> &subscriber)
{
    HILOG_INFO("NapiDeleteSubscribe start");

    auto subscribe = commonEventSubscriberInstances.find(subscriber);
    if (subscribe != commonEventSubscriberInstances.end())
    {
        HILOG_INFO("NapiDeleteSubscribe size = %{public}d", subscribe->second.size());
        for (auto asyncCallbackInfoSubscribe : subscribe->second)
        {
            HILOG_INFO("NapiDeleteSubscribe ptr = %{public}p", asyncCallbackInfoSubscribe);
            delete asyncCallbackInfoSubscribe;
            asyncCallbackInfoSubscribe = nullptr;
        }
        commonEventSubscriberInstances.erase(subscribe);
    }

    auto it = subscriberInstances.find(subscriber);
    if (it != subscriberInstances.end()) {
        subscriberInstances.erase(it);
    }
}

static napi_value Unsubscribe(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Unsubscribe start");

    // Argument parsing
    size_t argc = UNSUBSCRIBE_MAX_PARA;
    napi_value argv[UNSUBSCRIBE_MAX_PARA];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_ref callback = 0;
    std::shared_ptr<SubscriberInstance> subscriber;
    napi_value result;
    result = ParseParametersByUnsubscribe(env, argc, argv, subscriber, callback);
    bool isFind = false;
    napi_get_value_bool(env, result, &isFind);
    if (!isFind)
    {
        return nullptr;
    }

    AsyncCallbackInfoUnsubscribe *asynccallback = new AsyncCallbackInfoUnsubscribe{.env = env, .asyncWork = nullptr};

    asynccallback->subscriber = subscriber;
    asynccallback->argc = argc;
    if (argc >= UNSUBSCRIBE_MAX_PARA)
    {
        asynccallback->callback = callback;
    }

    napi_value resourceName;
    napi_create_string_latin1(env, "Unsubscribe", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data)
        {
            HILOG_INFO("Unsubscribe napi_create_async_work start");
            AsyncCallbackInfoUnsubscribe *asynccallbackinfo = (AsyncCallbackInfoUnsubscribe *)data;

            CommonEventManager::UnSubscribeCommonEvent(asynccallbackinfo->subscriber);
        },
        [](napi_env env, napi_status status, void *data)
        {
            HILOG_INFO("Subscribe napi_create_async_work end");
            AsyncCallbackInfoUnsubscribe *asynccallbackinfo = (AsyncCallbackInfoUnsubscribe *)data;

            if (asynccallbackinfo->argc >= UNSUBSCRIBE_MAX_PARA)
            {
                napi_value result = 0;
                napi_get_null(env, &result);
                SetCallback(env, asynccallbackinfo->callback, result);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);

            NapiDeleteSubscribe(asynccallbackinfo->subscriber);

            if (asynccallbackinfo)
            {
                HILOG_INFO("delete asynccallbackinfo");
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallback,
        &asynccallback->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallback->asyncWork));
    return nullptr;
}

static napi_value GetEventsByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::vector<std::string> &events)
{
    HILOG_INFO("GetEventsByCreateSubscriber start");
    napi_valuetype valuetype;
    bool hasProperty = false;
    bool isArray = false;
    size_t strLen = 0;
    uint32_t length = 0;
    // events
    NAPI_CALL(env, napi_has_named_property(env, argv, "events", &hasProperty));
    if (hasProperty)
    {
        napi_value eventsNapi;
        napi_get_named_property(env, argv, "events", &eventsNapi);
        napi_is_array(env, eventsNapi, &isArray);
        if (isArray)
        {
            napi_get_array_length(env, eventsNapi, &length);
            NAPI_ASSERT(env, length > 0, "The array is empty.");
            for (size_t i = 0; i < length; i++)
            {
                napi_value event;
                napi_get_element(env, eventsNapi, i, &event);
                NAPI_CALL(env, napi_typeof(env, event, &valuetype));
                NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
                char str[STR_MAX_SIZE] = {0};
                NAPI_CALL(env, napi_get_value_string_utf8(env, event, str, STR_MAX_SIZE - 1, &strLen));
                HILOG_INFO("GetEventsByCreateSubscriber str = %{public}s", str);
                events.push_back(str);
            }
        }
    }

    return nullptr;
}

static napi_value GetPublisherPermissionByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &permission, bool &hasProperty)
{
    HILOG_INFO("GetPublisherPermissionByCreateSubscriber start");

    napi_valuetype valuetype;
    size_t strLen = 0;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};

    // publisherPermission
    NAPI_CALL(env, napi_has_named_property(env, argv, "publisherPermission", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, argv, "publisherPermission", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        permission = str;
    }

    return nullptr;
}

static napi_value GetPublisherDeviceIdByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &publisherDeviceId, bool &hasProperty)
{
    HILOG_INFO("GetPublisherDeviceIdByCreateSubscriber start");

    napi_valuetype valuetype;
    size_t strLen = 0;
    napi_value result;
    char str[STR_MAX_SIZE] = {0};

    // publisherDeviceId
    NAPI_CALL(env, napi_has_named_property(env, argv, "publisherDeviceId", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, argv, "publisherDeviceId", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        publisherDeviceId = str;
    }

    return nullptr;
}

static napi_value GetPriorityByCreateSubscriber(
    const napi_env &env, const napi_value &argv, int &priority, bool &hasProperty)
{
    HILOG_INFO("GetPriorityByCreateSubscriber start");

    napi_valuetype valuetype;
    napi_value result;
    int32_t value;

    // priority
    NAPI_CALL(env, napi_has_named_property(env, argv, "priority", &hasProperty));
    if (hasProperty)
    {
        napi_get_named_property(env, argv, "priority", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        NAPI_CALL(env, napi_get_value_int32(env, result, &value));
        priority = value;
    }

    return nullptr;
}

static napi_value CommonEventSubscriberConstructor(napi_env env, napi_callback_info info)
{
    HILOG_INFO("CommonEventSubscriberConstructor start");

    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    // argv[0]:CommonEventSubscribeInfo:events
    std::vector<std::string> events;
    GetEventsByCreateSubscriber(env, argv[0], events);

    std::string permission;
    bool hasPermission = false;
    GetPublisherPermissionByCreateSubscriber(env, argv[0], permission, hasPermission);

    std::string publisherDeviceId;
    bool hasPublisherDeviceId = false;
    GetPublisherDeviceIdByCreateSubscriber(env, argv[0], publisherDeviceId, hasPublisherDeviceId);

    int priority;
    bool hasPriority = false;
    GetPriorityByCreateSubscriber(env, argv[0], priority, hasPriority);

    MatchingSkills matchingSkills;
    for (const auto &event : events)
    {
        matchingSkills.AddEvent(event);
    }

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (hasPermission)
    {
        subscriberInfo.SetPermission(permission);
    }

    if (hasPublisherDeviceId)
    {
        subscriberInfo.SetDeviceId(publisherDeviceId);
    }

    if (hasPriority)
    {
        subscriberInfo.SetPriority(priority);
    }

    SubscriberInstance *objectInfo = new SubscriberInstance(subscriberInfo);
    HILOG_INFO("CommonEventSubscriberConstructor  objectInfo = %{public}p", objectInfo);

    napi_wrap(
        env,
        thisVar,
        objectInfo,
        [](napi_env env, void *data, void *hint)
        {
            SubscriberInstance *objectInfo = (SubscriberInstance *)data;
            HILOG_INFO("CommonEventSubscriberConstructor this = %{public}p, destruct", objectInfo);

            for (auto commonEventSubscriberInstance : commonEventSubscriberInstances) {
                HILOG_INFO(
                    "CommonEventSubscriberConstructor get = %{public}p", commonEventSubscriberInstance.first.get());
                if (commonEventSubscriberInstance.first.get() == objectInfo) {
                    for (auto asyncCallbackInfoSubscribe : commonEventSubscriberInstance.second) {
                        HILOG_INFO("CommonEventSubscriberConstructor ptr = %{public}p", asyncCallbackInfoSubscribe);
                        delete asyncCallbackInfoSubscribe;
                        asyncCallbackInfoSubscribe = nullptr;
                    }
                    commonEventSubscriberInstances.erase(commonEventSubscriberInstance.first);
                    break;
                }
            }

            for (auto subscriberInstance : subscriberInstances) {
                if (subscriberInstance.first.get() == objectInfo) {
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

    HILOG_INFO("CommonEventSubscriberConstructor end");
    return thisVar;
}

static napi_value CommonEventSubscriberInit(napi_env env, napi_value exports)
{
    HILOG_INFO("enter");

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSubscribeInfo", GetSubscribeInfo),
        DECLARE_NAPI_FUNCTION("isOrderedCommonEvent", IsOrderedCommonEvent),
        DECLARE_NAPI_FUNCTION("isStickyCommonEvent", IsStickyCommonEvent),
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

static napi_value CommonEventInit(napi_env env, napi_value exports)
{
    HILOG_INFO("enter");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("publish", Publish),
        DECLARE_NAPI_FUNCTION("createSubscriber", CreateSubscriber),
        DECLARE_NAPI_FUNCTION("subscribe", Subscribe),
        DECLARE_NAPI_FUNCTION("unsubscribe", Unsubscribe),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

EXTERN_C_START

/*
 * Module export function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Propertise define
     */
    CommonEventSubscriberInit(env, exports);
    CommonEventInit(env, exports);

    return exports;
}
EXTERN_C_END

/*
 * Module define
 */
static napi_module _module = {

    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "commonevent",
    .nm_priv = ((void *)0),
    .reserved = {0}

};

/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
