/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_process.h"

#include <dlfcn.h>

#include "accesstoken_kit.h"
#include "hilog_tag_wrapper.h"
#include "permission_list_state.h"

using OHOS::Security::AccessToken::AccessTokenKit;
using OHOS::Security::AccessToken::PermissionListState;
using OHOS::Security::AccessToken::TypePermissionOper;

namespace OHOS {
namespace AppExecFwk {
static void *g_handle = nullptr;
#ifdef SUPPORT_GRAPHICS
#ifdef APP_USE_ARM
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib/module/ability/libfeatureability.z.so";
#elif defined(APP_USE_X86_64)
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib64/module/ability/libfeatureability.z.so";
#else
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib64/module/ability/libfeatureability.z.so";
#endif
#endif
constexpr char FUNC_CALL_ON_ABILITY_RESULT[] = "CallOnAbilityResult";
using NAPICallOnAbilityResult = void (*)(int requestCode, int resultCode, const Want &resultData, CallbackInfo cb);
constexpr char FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT[] = "CallOnRequestPermissionsFromUserResult";
using NAPICallOnRequestPermissionsFromUserResult = void (*)(int requestCode,
    const std::vector<std::string> &permissions, const std::vector<int> &grantResults, CallbackInfo callbackInfo);

std::shared_ptr<AbilityProcess> AbilityProcess::instance_ = nullptr;
std::map<Ability *, std::map<int, CallbackInfo>> AbilityProcess::abilityResultMap_;
std::mutex AbilityProcess::mutex_;
std::shared_ptr<AbilityProcess> AbilityProcess::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityProcess>();
        }
    }
    return instance_;
}

AbilityProcess::AbilityProcess()
{}

AbilityProcess::~AbilityProcess()
{}

ErrCode AbilityProcess::StartAbility(Ability *ability, CallAbilityParam param, CallbackInfo callback)
{
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability");
        return ERR_NULL_OBJECT;
    }
#ifdef SUPPORT_SCREEN
    // inherit split mode
    auto windowMode = ability->GetCurrentWindowMode();
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        param.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    TAG_LOGI(AAFwkTag::ABILITY, "window mode:%{public}d", windowMode);
#endif
    ErrCode err = ERR_OK;
    if (param.forResultOption) {
        if (param.setting == nullptr) {
            TAG_LOGI(AAFwkTag::ABILITY, "null param.setting");
            param.want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
            err = ability->StartAbilityForResult(param.want, param.requestCode);
        } else {
            TAG_LOGI(AAFwkTag::ABILITY, "param.setting");
            err = ability->StartAbilityForResult(param.want, param.requestCode, *(param.setting));
        }

        std::lock_guard<std::mutex> lock_l(mutex_);

        std::map<int, CallbackInfo> map;
        auto it = abilityResultMap_.find(ability);
        if (it == abilityResultMap_.end()) {
            TAG_LOGI(AAFwkTag::ABILITY, "not in abilityResultMap_");
        } else {
            TAG_LOGI(AAFwkTag::ABILITY, "in abilityResultMap_");
            map = it->second;
        }
        callback.errCode = err;
        map[param.requestCode] = callback;
        abilityResultMap_[ability] = map;
    } else {
        if (param.setting == nullptr) {
            TAG_LOGI(AAFwkTag::ABILITY, "null param.settingcall");
            err = ability->StartAbility(param.want);
        } else {
            TAG_LOGI(AAFwkTag::ABILITY, "param.settingcall");
            err = ability->StartAbility(param.want, *(param.setting));
        }
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return err;
}

void AbilityProcess::AddAbilityResultCallback(Ability *ability, CallAbilityParam &param, int32_t errCode,
                                              CallbackInfo &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::map<int, CallbackInfo> map;
    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        TAG_LOGI(AAFwkTag::ABILITY, "not in abilityResultMap_");
    } else {
        TAG_LOGI(AAFwkTag::ABILITY, "in abilityResultMap_");
        map = it->second;
    }
    callback.errCode = errCode;
    map[param.requestCode] = callback;
    abilityResultMap_[ability] = map;
}

void AbilityProcess::OnAbilityResult(Ability *ability, int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::ABILITY, "begin");

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITY, "not in abilityResultMap");
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        TAG_LOGE(AAFwkTag::ABILITY, "%{public}d not in map", requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;
#ifdef SUPPORT_GRAPHICS
    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "dlopen failed %{public}s. %{public}s",
                SHARED_LIBRARY_FEATURE_ABILITY,
                dlerror());
            return;
        }
    }
#endif
    // get function
    auto func = reinterpret_cast<NAPICallOnAbilityResult>(dlsym(g_handle, FUNC_CALL_ON_ABILITY_RESULT));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlsym failed %{public}s. %{public}s", FUNC_CALL_ON_ABILITY_RESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return;
    }
    func(requestCode, resultCode, resultData, callbackInfo);

    map.erase(requestCode);

    abilityResultMap_[ability] = map;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityProcess::RequestPermissionsFromUser(
    Ability *ability, CallAbilityPermissionParam &param, CallbackInfo callbackInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null ability");
        return;
    }

    std::vector<PermissionListState> permList;
    for (auto permission : param.permission_list) {
        TAG_LOGD(AAFwkTag::ABILITY, "permission:%{public}s", permission.c_str());
        PermissionListState permState;
        permState.permissionName = permission;
        permState.state = Security::AccessToken::SETTING_OPER;
        permList.emplace_back(permState);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "permList size:%{public}zu, permissions size:%{public}zu",
        permList.size(), param.permission_list.size());

    Security::AccessToken::PermissionGrantInfo grantInfo;
    auto ret = AccessTokenKit::GetSelfPermissionsState(permList, grantInfo);
    if (permList.size() != param.permission_list.size()) {
        TAG_LOGE(AAFwkTag::ABILITY, "permList size:%{public}zu", permList.size());
        return;
    }

    std::vector<int> permissionsState;
    for (auto permState : permList) {
        TAG_LOGD(AAFwkTag::ABILITY, "permissions:%{public}s. permissionsState:%{public}u",
            permState.permissionName.c_str(), permState.state);
        permissionsState.emplace_back(permState.state);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "permissions size:%{public}zu. permissionsState size:%{public}zu",
        param.permission_list.size(), permissionsState.size());

    auto requestCode = param.requestCode;
    if (ret != TypePermissionOper::DYNAMIC_OPER) {
        TAG_LOGD(AAFwkTag::ABILITY, "need not dynamic popup");
        (void)CallFunc(requestCode, param.permission_list, permissionsState, callbackInfo);
        return;
    }

    auto task = [self = GetInstance(), requestCode, callbackInfo]
        (const std::vector<std::string> &permissions, const std::vector<int> &grantResults) mutable {
        if (!self) {
            TAG_LOGE(AAFwkTag::ABILITY, "null self");
            return;
        }
        if (!self->CallFunc(requestCode, permissions, grantResults, callbackInfo)) {
            TAG_LOGE(AAFwkTag::ABILITY, "call function failed");
            return;
        }
    };

    ability->RequestPermissionsFromUser(param.permission_list, permissionsState, std::move(task));
}

bool AbilityProcess::CallFunc(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &permissionsState, CallbackInfo &callbackInfo)
{
#ifdef SUPPORT_GRAPHICS
    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "dlopen failed %{public}s. %{public}s",
                SHARED_LIBRARY_FEATURE_ABILITY, dlerror());
            return false;
        }
    }
#endif
    // get function
    auto func = reinterpret_cast<NAPICallOnRequestPermissionsFromUserResult>(
        dlsym(g_handle, FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlsym failed %{public}s. %{public}s",
            FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return false;
    }
    func(requestCode, permissions, permissionsState, callbackInfo);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
