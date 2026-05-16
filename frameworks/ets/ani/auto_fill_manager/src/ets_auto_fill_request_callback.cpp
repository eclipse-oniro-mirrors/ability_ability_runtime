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

#include "ets_auto_fill_request_callback.h"

#include "ani_common_util.h"
#include "ets_auto_fill_manager_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AutoFillManagerEts {
namespace {
constexpr int32_t ARGC_ZERO = 0;
constexpr int32_t ARGC_ONE = 1;
const std::string METHOD_ON_FILL_REQUEST_SUCCESS = "onSuccess";
const std::string METHOD_ON_FILL_REQUEST_FAILED = "onFailure";
} // namespace

EtsAutoFillRequestCallback::EtsAutoFillRequestCallback(ani_vm *vm, int32_t instanceId,
    AutoFillManagerFunc autoFillManagerFunc)
    : vm_(vm), instanceId_(instanceId), autoFillManagerFunc_(autoFillManagerFunc) {}

EtsAutoFillRequestCallback::~EtsAutoFillRequestCallback() {}

void EtsAutoFillRequestCallback::OnFillRequestSuccess(const AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "onSuccess called");
    ani_env *env = GetAniEnv();
    ani_ref argv[ARGC_ONE] = { reinterpret_cast<ani_ref>(WrapViewData(env, viewData)) };
    ETSCallFunction(METHOD_ON_FILL_REQUEST_SUCCESS, argv, ARGC_ONE);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void EtsAutoFillRequestCallback::OnFillRequestFailed(int32_t errCode, const std::string &fillContent, bool isPopup)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "onFailure called");
    ani_env *env = GetAniEnv();
    ani_ref argv[ARGC_ONE] = { reinterpret_cast<ani_ref>(WrapFillFailureResult(env, errCode)) };
    ETSCallFunction(METHOD_ON_FILL_REQUEST_FAILED, argv, ARGC_ONE);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void EtsAutoFillRequestCallback::Register(ani_object object)
{
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null env");
        return;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null object");
        return;
    }

    if (IsEtsCallbackEquals(callback_, object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "callback exist");
        return;
    }

    callback_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null callback_");
        return;
    }

    ani_ref objRef = nullptr;
    ani_status status = ANI_ERROR;
    status = env->GlobalReference_Create(object, &objRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }

    callback_->aniObj = object;
    callback_->aniRef = objRef;
}

void EtsAutoFillRequestCallback::ETSCallFunction(const std::string &methodName, ani_ref *argv, int32_t argc)
{
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null env");
        return;
    }
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null callback_");
        return;
    }

    ani_status status = ANI_ERROR;
    ani_ref funRef;
    status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(callback_->aniRef), methodName.c_str(),
        &funRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object_GetPropertyByName_Ref failed, status: %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::AUTOFILLMGR, "invalid property");
        return;
    }
    ani_ref result;
    status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), argc, argv, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "FunctionalObject_Call failed, status: %{public}d", status);
    }
}

bool EtsAutoFillRequestCallback::IsEtsCallbackEquals(std::shared_ptr<AppExecFwk::ETSNativeReference> callback,
    ani_object object)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Invalid etsCallback");
        return false;
    }

    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null env");
        return false;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null object");
        return false;
    }

    ani_boolean isEquals = false;
    if ((env->Reference_StrictEquals(reinterpret_cast<ani_ref>(object), callback->aniRef, &isEquals)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object not match");
        return false;
    }

    return isEquals;
}

ani_env *EtsAutoFillRequestCallback::GetAniEnv()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "GetAniEnv call");
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null vm_");
        return nullptr;
    }
    ani_env* env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "GetEnv failed");
        return nullptr;
    }
    return env;
}
} // namespace AutoFillManagerEts
} // namespace OHOS