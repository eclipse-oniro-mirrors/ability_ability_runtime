/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "js_interop_object.h"

#include "ets_ability_lifecycle_callback.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
JsInteropObject::JsInteropObject(ani_env *env, std::shared_ptr<AppExecFwk::ETSNativeReference> ref)
{
    if (env == nullptr || ref == nullptr || ref->aniRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or ref or ref->aniRef");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetVM failed");
        return;
    }
    vm_ = aniVM;
    hybridgref_create_from_ani(env, ref->aniRef, &ref_);
}

JsInteropObject::~JsInteropObject()
{
    if (ref_ == nullptr) {
        return;
    }
    if (vm_ != nullptr) {
        ani_env *env = GetAniEnv();
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null env");
            return;
        }
        hybridgref_delete_from_ani(env, ref_);
    }
}

ani_env *JsInteropObject::GetAniEnv()
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null vm_");
        return nullptr;
    }
    ani_env* env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return nullptr;
    }
    return env;
}

napi_value JsInteropObject::GetNapiValue(napi_env env)
{
    napi_value result = nullptr;
    if (!hybridgref_get_napi_value(env, ref_, &result)) {
        TAG_LOGE(AAFwkTag::APPKIT, "hybridgref_get_napi_value failed");
        return nullptr;
    }
    return result;
}

bool JsInteropObject::IsFromAni()
{
    return vm_ != nullptr && ref_ != nullptr;
}

extern "C" ETS_EXPORT InteropObject* OHOS_CreateJsInteropObject(const Runtime &runtime,
    const AbilityLifecycleCallbackArgs &arg)
{
    std::shared_ptr<AppExecFwk::ETSNativeReference> ref = static_cast<const EtsAbilityLifecycleCallbackArgs&>(arg).ref_;
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ref");
        return nullptr;
    }
    ani_env *env = const_cast<ETSRuntime&>(static_cast<const ETSRuntime&>(runtime)).GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    return new (std::nothrow) JsInteropObject(env, ref);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
