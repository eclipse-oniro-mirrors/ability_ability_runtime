/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_child_process.h"

#include "child_process.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_child_process_param.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<ChildProcess> JsChildProcess::Create(const std::unique_ptr<Runtime>& runtime)
{
    return std::make_shared<JsChildProcess>(static_cast<JsRuntime &>(*runtime));
}

JsChildProcess::JsChildProcess(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime) {}
JsChildProcess::~JsChildProcess()
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "Called");
    jsRuntime_.FreeNativeReference(std::move(jsChildProcessObj_));
}

bool JsChildProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null info");
        return false;
    }
    bool ret = ChildProcess::Init(info);
    if (!ret) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "ChildProcess init failed");
        return false;
    }
    if (info->srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Empty info srcEntry");
        return false;
    }
    std::string srcPath = info->srcEntry;
    RemoveFileExtension(srcPath);
    srcPath.append(".abc");
    std::string moduleName(info->moduleName);
    moduleName.append("::").append(info->name);

    HandleScope handleScope(jsRuntime_);
    jsChildProcessObj_ = jsRuntime_.LoadModule(moduleName, srcPath, info->hapPath, info->isEsModule);
    if (jsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null jsChildProcessObj_");
        return false;
    }
    return true;
}

void JsChildProcess::OnStart()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    ChildProcess::OnStart();
    CallObjectMethod("onStart");
}

void JsChildProcess::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "JsChildProcess OnStart called");
    if (!args) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null args");
        return;
    }
    ChildProcess::OnStart(args);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value jsArgs = WrapChildProcessArgs(env, *args);
    napi_value argv[] = { jsArgs };
    CallObjectMethod("onStart", argv, ArraySize(argv));
}

napi_value JsChildProcess::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "called, name:%{public}s", name);
    if (jsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null jsChildProcessObj_");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsChildProcessObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Get ChildProcess obj failed");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Get '%{public}s' failed", name);
        return nullptr;
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    return nullptr;
}
}  // namespace AbilityRuntime
}  // namespace OHOS