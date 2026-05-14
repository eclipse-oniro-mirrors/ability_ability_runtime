/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "form_runtime/cj_form_extension_object.h"

#include "cj_utils_ffi.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace AbilityRuntime {

struct CJFormExtAbilityFuncs {
    int64_t (*createCjFormExtAbility)(const char* name, FormExtAbilityHandle extAbility);
    void (*releaseCjFormExtAbility)(int64_t id);
    void (*cjFormExtAbilityInit)(int64_t id, FormExtAbilityHandle extAbility);
    CFormBindingData (*cjFormExtAbilityOnAddForm)(int64_t id, WantHandle want);
    void (*cjFormExtAbilityOnCastToNormalForm)(int64_t id, const char* formId);
    void (*cjFormExtAbilityOnUpdateForm)(int64_t id, const char* formId, const char* wantParams);
    void (*cjFormExtAbilityOnChangeFormVisibility)(int64_t id, CRecordI64I32 formEventsMap);
    void (*cjFormExtAbilityOnFormEvent)(int64_t id, const char* formId, const char* message);
    void (*cjFormExtAbilityOnRemoveForm)(int64_t id, const char* formId);
    void (*cjFormExtAbilityOnConfigurationUpdate)(int64_t id, CConfiguration configuration);
    int32_t (*cjFormExtAbilityOnAcquireFormState)(int64_t id, WantHandle want);
    void (*cjFormExtAbilityOnStop)(int64_t id);
    void (*freeCFormBindingData)(CFormBindingData data);
};

struct CJFormExtAbilityFuncsV2 {
    void (*cjFormExtAbilityOnConfigurationUpdateV2)(int64_t id, CConfigurationV2 configuration);
};
} // namespace AbilityRuntime
} // namespace OHOS

namespace {
static OHOS::AbilityRuntime::CJFormExtAbilityFuncs g_cjFuncs {};
static OHOS::AbilityRuntime::CJFormExtAbilityFuncsV2 g_cjFuncsV2 {};
static const int32_t CJ_OBJECT_ERR_CODE = -1;
static const int32_t FORM_STATE_DEFAULT = 0;
} // namespace

namespace OHOS {
namespace AbilityRuntime {

char* CreateCStringFromString(const std::string& source)
{
    if (source.size() == 0) {
        return nullptr;
    }
    size_t length = source.size() + 1;
    auto res = static_cast<char*>(malloc(length));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null res");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::FORM_EXT, "Strcpy failed");
        return nullptr;
    }
    return res;
}

void CJFormExtensionObject::Destroy()
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "Destroy");
    if (cjID_ != 0) {
        if (g_cjFuncs.releaseCjFormExtAbility == nullptr) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "releaseCjFormExtAbility is not registered");
            return;
        }
        g_cjFuncs.releaseCjFormExtAbility(cjID_);
        cjID_ = 0;
    }
}

int32_t CJFormExtensionObject::Init(const std::string& abilityName, FormExtAbilityHandle extAbility)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "called");
    if (g_cjFuncs.createCjFormExtAbility == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "createCjFormExtAbility is not registered");
        return CJ_OBJECT_ERR_CODE;
    }
    cjID_ = g_cjFuncs.createCjFormExtAbility(abilityName.c_str(), extAbility);
    if (cjID_ == 0) {
        TAG_LOGE(
            AAFwkTag::FORM_EXT, "Failed to Init CjFormExtAbility: %{public}s is not registered", abilityName.c_str());
        return CJ_OBJECT_ERR_CODE;
    }
    if (g_cjFuncs.cjFormExtAbilityInit == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityInit is not registered");
        return CJ_OBJECT_ERR_CODE;
    }
    g_cjFuncs.cjFormExtAbilityInit(cjID_, extAbility);
    return 0;
}

CFormBindingData CJFormExtensionObject::OnAddForm(const AAFwk::Want& want)
{
    CFormBindingData cFormBindingData {};
    if (g_cjFuncs.cjFormExtAbilityOnAddForm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnAddForm is not registered");
        return cFormBindingData;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    return g_cjFuncs.cjFormExtAbilityOnAddForm(cjID_, wantHandle);
}

void CJFormExtensionObject::OnCastToNormalForm(const char* formId)
{
    if (g_cjFuncs.cjFormExtAbilityOnCastToNormalForm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnCastToNormalForm is not registered");
        return;
    }
    g_cjFuncs.cjFormExtAbilityOnCastToNormalForm(cjID_, formId);
}

void CJFormExtensionObject::OnUpdateForm(const char* formId, const char* wantParams)
{
    if (g_cjFuncs.cjFormExtAbilityOnUpdateForm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnUpdateForm is not registered");
        return;
    }
    g_cjFuncs.cjFormExtAbilityOnUpdateForm(cjID_, formId, wantParams);
}

void CJFormExtensionObject::OnChangeFormVisibility(const std::map<int64_t, int32_t>& formEventsMap)
{
    if (g_cjFuncs.cjFormExtAbilityOnChangeFormVisibility == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnChangeFormVisibility is not registered");
        return;
    }
    CRecordI64I32 record = {};
    char** keysHead = static_cast<char**>(malloc(sizeof(char*) * formEventsMap.size()));
    if (keysHead == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnChangeFormVisibility malloc failed");
        return;
    }
    int32_t* valuesHead = static_cast<int32_t*>(malloc(sizeof(int32_t) * formEventsMap.size()));
    if (valuesHead == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnChangeFormVisibility malloc failed");
        free(keysHead);
        return;
    }
    int64_t i = 0;
    for (auto& item : formEventsMap) {
        keysHead[i] = const_cast<char*>(std::to_string(item.first).c_str());
        valuesHead[i] = item.second;
        i++;
    }
    record.keys.size = static_cast<int64_t>(formEventsMap.size());
    record.keys.head = keysHead;
    record.values.size = static_cast<int64_t>(formEventsMap.size());
    record.values.head = valuesHead;
    g_cjFuncs.cjFormExtAbilityOnChangeFormVisibility(cjID_, record);
    free(keysHead);
    free(valuesHead);
}

void CJFormExtensionObject::OnFormEvent(const char* formId, const char* message)
{
    if (g_cjFuncs.cjFormExtAbilityOnFormEvent == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnFormEvent is not registered");
        return;
    }
    g_cjFuncs.cjFormExtAbilityOnFormEvent(cjID_, formId, message);
}

void CJFormExtensionObject::OnRemoveForm(const char* formId)
{
    if (g_cjFuncs.cjFormExtAbilityOnRemoveForm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnRemoveForm is not registered");
        return;
    }
    g_cjFuncs.cjFormExtAbilityOnRemoveForm(cjID_, formId);
}

void CJFormExtensionObject::OnConfigurationUpdate(std::shared_ptr<AppExecFwk::Configuration> fullConfig)
{
    if (g_cjFuncsV2.cjFormExtAbilityOnConfigurationUpdateV2 != nullptr) {
        auto cfg = CreateCConfigurationV2(*fullConfig);
        g_cjFuncsV2.cjFormExtAbilityOnConfigurationUpdateV2(cjID_, cfg);
        FreeCConfigurationV2(&cfg);
        return;
    }
    if (g_cjFuncs.cjFormExtAbilityOnConfigurationUpdate == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnConfigurationUpdate is not registered");
        return;
    }
    auto cfg = CreateCConfiguration(*fullConfig);
    g_cjFuncs.cjFormExtAbilityOnConfigurationUpdate(cjID_, cfg);
    FreeCConfiguration(&cfg);
}

int32_t CJFormExtensionObject::OnAcquireFormState(const AAFwk::Want& want)
{
    if (g_cjFuncs.cjFormExtAbilityOnAcquireFormState == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnAcquireFormState is not registered");
        return FORM_STATE_DEFAULT;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    return g_cjFuncs.cjFormExtAbilityOnAcquireFormState(cjID_, wantHandle);
}

void CJFormExtensionObject::OnStop()
{
    if (g_cjFuncs.cjFormExtAbilityOnStop == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "cjFormExtAbilityOnStop is not registered");
        return;
    }
    g_cjFuncs.cjFormExtAbilityOnStop(cjID_);
}

void CJFormExtensionObject::FreeCFormBindingData(CFormBindingData data)
{
    if (g_cjFuncs.freeCFormBindingData == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "freeCFormBindingData is not registered");
        return;
    }
    g_cjFuncs.freeCFormBindingData(data);
}

extern "C" {
CJ_EXPORT void FFIRegisterCJFormExtAbilityFuncs(void (*registerFunc)(CJFormExtAbilityFuncs*))
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "FFIRegisterCJExtAbilityFuncs start");
    if (g_cjFuncs.createCjFormExtAbility != nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Repeated registration for cj functions of createCjFormExtAbility");
        return;
    }

    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FFIRegisterCJFormExtAbilityFuncs failed, registerFunc is nullptr");
        return;
    }

    registerFunc(&g_cjFuncs);
    TAG_LOGD(AAFwkTag::FORM_EXT, "FFIRegisterCJFormExtAbilityFuncs end");
}

CJ_EXPORT void FFIRegisterCJFormExtAbilityFuncsV2(void (*registerFunc)(CJFormExtAbilityFuncsV2*))
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "FFIRegisterCJFormExtAbilityFuncsV2 start");
    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FFIRegisterCJFormExtAbilityFuncsV2 failed, registerFunc is nullptr");
        return;
    }

    registerFunc(&g_cjFuncsV2);
    TAG_LOGD(AAFwkTag::FORM_EXT, "FFIRegisterCJFormExtAbilityFuncsV2 end");
}
} // extern "C"
} // namespace AbilityRuntime
} // namespace OHOS
