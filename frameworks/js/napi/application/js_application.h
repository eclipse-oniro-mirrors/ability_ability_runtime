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
#ifndef OHOS_ABILITY_RUNTIME_JS_APPLICATION_H
#define OHOS_ABILITY_RUNTIME_JS_APPLICATION_H

#include "context.h"
#include "napi/native_api.h"
#include "ipc_skeleton.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
class JsApplication {
public:
    explicit JsApplication()
    {}
    virtual ~JsApplication() = default;

    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value CreateModuleContext(napi_env env, napi_callback_info info);
    static napi_value CreateModuleContextSync(napi_env env, napi_callback_info info);
    static napi_value CreateBundleContext(napi_env env, napi_callback_info info);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value GetApplicationContextInstance(napi_env env, napi_callback_info info);
    static napi_value CreatePluginModuleContext(napi_env env, napi_callback_info info);
    static napi_value PromoteCurrentToCandidateMasterProcess(napi_env env, napi_callback_info info);
    static napi_value DemoteCurrentFromCandidateMasterProcess(napi_env env, napi_callback_info info);
    static napi_value ExitMasterProcessRole(napi_env env, napi_callback_info info);
    static napi_value CreatePluginModuleContextForBundle(napi_env env, napi_callback_info info);
    static napi_value GetAppPreloadType(napi_env env, napi_callback_info info);

private:
    napi_value OnCreateModuleContext(napi_env env, NapiCallbackInfo &info);
    napi_value OnCreateModuleContextSync(napi_env env, NapiCallbackInfo &info);
    napi_value OnCreateBundleContext(napi_env env, NapiCallbackInfo &info);
    napi_value OnCreatePluginModuleContext(napi_env env, NapiCallbackInfo &info);
    napi_value OnCreatePluginModuleContextForBundle(napi_env env, NapiCallbackInfo &info);
    napi_value CreateJsContext(napi_env env, const std::shared_ptr<Context> &context);
    void SetCreateCompleteCallback(std::shared_ptr<std::shared_ptr<Context>> contextPtr,
        NapiAsyncTask::CompleteCallback &complete);
    bool CheckCallerIsSystemApp();
    bool CheckCallerPermission(const std::string &permission);
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo &info);
    napi_value OnGetApplicationContextInstance(napi_env env, NapiCallbackInfo &info);
    napi_value OnPromoteCurrentToCandidateMasterProcess(napi_env env, NapiCallbackInfo& info);
    napi_value OnDemoteCurrentFromCandidateMasterProcess(napi_env env, NapiCallbackInfo& info);
    napi_value OnExitMasterProcessRole(napi_env env, NapiCallbackInfo& info);
    bool VerifyCreatePluginContextParams(napi_env env, NapiCallbackInfo &info, std::string &moduleName,
        std::string &pluginBundleName, std::string &hostBundleName);
    napi_value OnGetAppPreloadType(napi_env env, NapiCallbackInfo &info);
};

napi_value ApplicationInit(napi_env env, napi_value exportObj);
void JsApplicationInitProperty(napi_env env, napi_value exportObj);
} // namespace AbilityRuntime
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_JS_APPLICATION_H