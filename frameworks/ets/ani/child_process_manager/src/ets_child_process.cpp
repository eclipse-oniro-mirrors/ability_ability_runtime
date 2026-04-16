/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ets_child_process.h"

#include "ani_common_child_process_param.h"
#include "hilog_tag_wrapper.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {

EtsChildProcess* EtsChildProcess::Create(const std::unique_ptr<Runtime> &runtime)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null runtime");
        return nullptr;
    }
    return new (std::nothrow) EtsChildProcess(static_cast<ETSRuntime &>(*runtime));
}

EtsChildProcess::EtsChildProcess(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime) {}

EtsChildProcess::~EtsChildProcess()
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "EtsChildProcess destroy");
    etsChildProcessObj_.reset();
}

bool EtsChildProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess Init called");
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

    std::string srcEntrance = info->srcEntry;
    auto pos = srcEntrance.rfind(".ets");
    if (pos != std::string::npos) {
        srcEntrance.erase(pos);
    }

    std::string srcPath(info->moduleName);
    srcPath.append("/").append(srcEntrance);
    if (pos != std::string::npos) {
        srcPath.append(".abc");
    }

    std::string moduleName(info->moduleName);
    moduleName.append("::").append(info->name);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null env");
        return false;
    }

    etsChildProcessObj_ = etsRuntime_.LoadModule(moduleName, srcPath, info->hapPath, info->isEsModule, false,
        srcEntrance);
    if (etsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null etsChildProcessObj_");
        return false;
    }
    return true;
}

void EtsChildProcess::OnStart()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess OnStart called");
    ChildProcess::OnStart();
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null env");
        return;
    }
    ani_ref undefinedRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetUndefined(&undefinedRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetUndefined failed, status: %{public}d", status);
        return;
    }
    CallObjectMethod(false, "onStart", nullptr, reinterpret_cast<ani_object>(undefinedRef));
}

void EtsChildProcess::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess OnStart with args called");
    if (!args) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null args");
        return;
    }
    ChildProcess::OnStart(args);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null env");
        return;
    }

    if (etsChildProcessObj_ == nullptr || etsChildProcessObj_->aniCls == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null etsChildProcessObj_ or aniCls");
        return;
    }

    // Convert ChildProcessArgs to ANI object
    ani_object aniArgs = AppExecFwk::WrapChildProcessArgs(env, *args);
    if (aniArgs == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "WrapChildProcessArgs failed");
        return;
    }
    CallObjectMethod(false, "onStart", nullptr, aniArgs);
}

ani_ref EtsChildProcess::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "EtsChildProcess CallObjectMethod");
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null env");
        return nullptr;
    }
    if (etsChildProcessObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null etsChildProcessObj_");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(etsChildProcessObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Class_FindMethod failed, status: %{public}d", status);
        return nullptr;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null method");
        return nullptr;
    }
    ani_ref res = nullptr;
    va_list args;
    if (withResult) {
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Ref_V(etsChildProcessObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "status : %{public}d", status);
            va_end(args);
            return nullptr;
        }
        va_end(args);
        return res;
    }
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsChildProcessObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "status : %{public}d", status);
    }
    va_end(args);
    return nullptr;
}
}  // namespace AbilityRuntime
}  // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::ChildProcess* OHOS_ETS_Child_Process_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AbilityRuntime::EtsChildProcess::Create(runtime);
}