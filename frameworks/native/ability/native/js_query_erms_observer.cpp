/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_query_erms_observer.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
JsQueryERMSObserver::JsQueryERMSObserver(napi_env env) : env_(env) {}

JsQueryERMSObserver::~JsQueryERMSObserver() = default;

void JsQueryERMSObserver::OnQueryFinished(const std::string &appId, const std::string &startTime,
    const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "call");
    wptr<JsQueryERMSObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, appId, startTime, rule, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            AbilityRuntime::HandleScope handleScope(env);
            sptr<JsQueryERMSObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr == nullptr) {
                TAG_LOGE(AAFwkTag::QUERY_ERMS, "null jsObserverSptr");
                return;
            }
            if (resultCode == AAFwk::ERR_CAPABILITY_NOT_SUPPORT || resultCode == ERR_OK) {
                jsObserverSptr->HandleOnQueryFinished(appId, startTime, rule, resultCode);
                return;
            }
            jsObserverSptr->HandleOnQueryFinished(appId, startTime, rule, AAFwk::INNER_ERR);
    });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsQueryERMSObserver::OnQueryFinished", env_, std::make_unique<NapiAsyncTask>(callback,
        std::move(execute), std::move(complete)));
}

void JsQueryERMSObserver::HandleOnQueryFinished(const std::string &appId, const std::string &startTime,
    const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "call");
    std::vector<napi_deferred> promises;
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
            if (it->appId != appId || it->startTime != startTime || it->deferred == nullptr) {
                it++;
                continue;
            }
            promises.emplace_back(it->deferred);
            it = jsObserverObjectList_.erase(it);
        }
    }

    for (const napi_deferred& promise : promises) {
        CallPromise(promise, rule, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartQueryERMS", atoi(startTime.c_str()));
    }
}

void JsQueryERMSObserver::CallPromise(napi_deferred deferred, const AtomicServiceStartupRule &rule,
    int32_t resultCode)
{
    AbilityRuntime::HandleScope handleScope(env_);
    if (deferred == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "deferred is nullptr");
        return;
    }
    if (resultCode == ERR_OK) {
        napi_value result = CreateJsAtomicServiceStartupRule(env_, rule);
        napi_resolve_deferred(env_, deferred, result);
        return;
    }
    napi_value error = CreateJsErrorByNativeErr(env_, resultCode, "",
        GetInnerErrorMsg(AbilityInnerErrorMsg::QUERY_ATOMIC_SERVICE_STARTUP_RULE_FAILED));
    napi_reject_deferred(env_, deferred, error);
}

void JsQueryERMSObserver::AddJsObserverObject(const std::string &appId, const std::string &startTime,
    napi_value* result)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "call");
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end(); ++it) {
            if (it->appId == appId && it->startTime == startTime) {
                TAG_LOGW(AAFwkTag::QUERY_ERMS, "The jsObject has been added");
                return;
            }
        }
    }

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartQueryERMS", atoi(startTime.c_str()));
    JsQueryERMSObserverObject object;
    object.appId = appId;
    object.startTime = startTime;
    napi_deferred deferred;
    napi_create_promise(env_, &deferred, result);
    object.deferred = deferred;

    std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
    jsObserverObjectList_.emplace_back(object);
}

napi_value JsQueryERMSObserver::CreateJsAtomicServiceStartupRule(napi_env env,
    const AbilityRuntime::AtomicServiceStartupRule &rule)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_set_named_property(env, object, "isOpenAllowed", CreateJsValue(env, rule.isOpenAllowed));
    napi_set_named_property(env, object, "isEmbeddedAllowed", CreateJsValue(env, rule.isEmbeddedAllowed));
    return handleEscape.Escape(object);
}
} // namespace AbilityRuntime
} // namespace OHOS
