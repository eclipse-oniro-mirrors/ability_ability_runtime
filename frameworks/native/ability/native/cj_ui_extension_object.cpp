/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "cj_ui_extension_object.h"
#include "configuration_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "cj_utils_ffi.h"
#include "last_exit_detail_info.h"

namespace OHOS {
namespace AbilityRuntime {
struct CJLastExitDetailInfo {
    int32_t pid;
    char* processName;
    int32_t uid;
    int32_t exitSubReason;
    char* exitMsg;
    int32_t rss;
    int32_t pss;
    int64_t timestamp;
    int32_t processState;
    char* killReason;
    bool hasKillReason;
};

struct CJLaunchParam {
    int32_t launchReason;
    int32_t lastExitReason;
    char* lastExitMessage;
};

struct CJLaunchParamV3 {
    int32_t launchReason;
    int32_t lastExitReason;
    char* lastExitMessage;
    char* launchReasonMessage;
    CJLastExitDetailInfo lastExitDetailInfo;
};

struct CJExtAbilityFuncs {
    int64_t (*createCjExtAbility)(const char* name, int32_t type);
    void (*releaseCjExtAbility)(int64_t id, int32_t type);
    void (*cjExtAbilityInit)(int64_t id, int32_t type, ExtAbilityHandle extAbility);
    void (*cjExtAbilityOnCreate)(int64_t id, int32_t type, WantHandle want, CJLaunchParam launchParam);
    void (*cjExtAbilityOnDestroy)(int64_t id, int32_t type);
    void (*cjExtAbilityOnSessionCreate)(int64_t id, int32_t type, WantHandle want, int64_t sessionId);
    void (*cjExtAbilityOnSessionDestroy)(int64_t id, int32_t type, int64_t sessionId);
    void (*cjExtAbilityOnForeground)(int64_t id, int32_t type);
    void (*cjExtAbilityOnBackground)(int64_t id, int32_t type);
    void (*cjExtAbilityOnConfigurationUpdate)(int64_t id, int32_t type, CConfiguration configuration);
    void (*cjExtAbilityOnMemoryLevel)(int64_t id, int32_t type, int32_t level);
    void (*cjExtAbilityOnStartContentEditing)(int64_t id, int32_t type, const char* imageUri, WantHandle want,
        int64_t sessionId);
};

struct CJExtAbilityFuncsV2 {
    void (*cjExtAbilityOnCreateV3)(int64_t id, int32_t type, WantHandle want, CJLaunchParamV3 launchParam);
    void (*cjExtAbilityOnConfigurationUpdateV2)(int64_t id, int32_t type, CConfigurationV2 configuration);
};
} // namespace AbilityRuntime
} // namespace OHOS

namespace {
static OHOS::AbilityRuntime::CJExtAbilityFuncs g_cjFuncs {};
static OHOS::AbilityRuntime::CJExtAbilityFuncsV2 g_cjFuncsV2 {};
static const int32_t CJ_OBJECT_ERR_CODE = -1;
} // namespace

namespace OHOS {
namespace AbilityRuntime {
void CJUIExtensionObject::Destroy()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "Destroy");
    if (cjID_ != 0) {
        if (g_cjFuncs.releaseCjExtAbility == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "releaseCjExtAbility is not registered");
            return;
        }
        g_cjFuncs.releaseCjExtAbility(cjID_, GetType());
        cjID_ = 0;
    }
}

int32_t CJUIExtensionObject::Init(const std::string& abilityName, CJExtensionAbilityType type,
    ExtAbilityHandle extAbility)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");

    type_ = type;
    if (g_cjFuncs.createCjExtAbility == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "createCjExtAbility is not registered");
        return CJ_OBJECT_ERR_CODE;
    }

    cjID_ = g_cjFuncs.createCjExtAbility(abilityName.c_str(), GetType());
    if (cjID_ == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT,
            "Failed to Init CJUIExtensionObject. CJExtAbility: %{public}s is not registered", abilityName.c_str());
        return CJ_OBJECT_ERR_CODE;
    }

    if (g_cjFuncs.cjExtAbilityInit == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityInit is not registered");
        return CJ_OBJECT_ERR_CODE;
    }

    g_cjFuncs.cjExtAbilityInit(cjID_, GetType(), extAbility);

    return 0;
}

CJLastExitDetailInfo CreateCJLastExitDetailInfo(const AAFwk::LastExitDetailInfo& lastExitDetailInfo)
{
    CJLastExitDetailInfo detailInfo = {};
    detailInfo.pid = lastExitDetailInfo.pid;
    detailInfo.processName = CreateCStringFromString(lastExitDetailInfo.processName);
    detailInfo.uid = lastExitDetailInfo.uid;
    detailInfo.exitSubReason = lastExitDetailInfo.exitSubReason;
    detailInfo.exitMsg = CreateCStringFromString(lastExitDetailInfo.exitMsg);
    detailInfo.rss = lastExitDetailInfo.rss;
    detailInfo.pss = lastExitDetailInfo.pss;
    detailInfo.timestamp = lastExitDetailInfo.timestamp;
    detailInfo.processState = lastExitDetailInfo.processState;
    if (!lastExitDetailInfo.killReason.empty()) {
        detailInfo.killReason = CreateCStringFromString(lastExitDetailInfo.killReason);
        detailInfo.hasKillReason = true;
    } else {
        detailInfo.killReason = nullptr;
        detailInfo.hasKillReason = false;
    }
    return detailInfo;
}

void FreeCJLastExitDetailInfo(CJLastExitDetailInfo& detailInfo)
{
    if (detailInfo.processName != nullptr) {
        free(detailInfo.processName);
        detailInfo.processName = nullptr;
    }
    if (detailInfo.exitMsg != nullptr) {
        free(detailInfo.exitMsg);
        detailInfo.exitMsg = nullptr;
    }
    if (detailInfo.killReason != nullptr) {
        free(detailInfo.killReason);
        detailInfo.killReason = nullptr;
    }
}

void CJUIExtensionObject::OnCreate(const AAFwk::Want &want, AAFwk::LaunchParam &launchParam)
{
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    if (g_cjFuncsV2.cjExtAbilityOnCreateV3 != nullptr) {
        CJLaunchParamV3 param;
        param.launchReason = launchParam.launchReason;
        param.lastExitReason = launchParam.lastExitReason;
        param.lastExitMessage = CreateCStringFromString(launchParam.lastExitMessage);
        param.launchReasonMessage = CreateCStringFromString(launchParam.launchReasonMessage);
        param.lastExitDetailInfo = CreateCJLastExitDetailInfo(launchParam.lastExitDetailInfo);

        g_cjFuncsV2.cjExtAbilityOnCreateV3(cjID_, GetType(), wantHandle, param);
        free(param.lastExitMessage);
        free(param.launchReasonMessage);
        FreeCJLastExitDetailInfo(param.lastExitDetailInfo);
        return;
    }
    if (g_cjFuncs.cjExtAbilityOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnCreate is not registered");
        return;
    }

    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    param.lastExitMessage = CreateCStringFromString(launchParam.lastExitMessage);

    g_cjFuncs.cjExtAbilityOnCreate(cjID_, GetType(), wantHandle, param);
}

void CJUIExtensionObject::OnDestroy()
{
    if (g_cjFuncs.cjExtAbilityOnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnDestroy is not registered");
        return;
    }

    g_cjFuncs.cjExtAbilityOnDestroy(cjID_, GetType());
}

void CJUIExtensionObject::OnSessionCreate(const AAFwk::Want &want, int64_t sessionId)
{
    if (g_cjFuncs.cjExtAbilityOnSessionCreate == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnSessionCreate is not registered");
        return;
    }

    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);

    g_cjFuncs.cjExtAbilityOnSessionCreate(cjID_, GetType(), wantHandle, sessionId);
}

void CJUIExtensionObject::OnSessionDestroy(int64_t sessionId)
{
    if (g_cjFuncs.cjExtAbilityOnSessionDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnSessionDestroy is not registered");
        return;
    }

    g_cjFuncs.cjExtAbilityOnSessionDestroy(cjID_, GetType(), sessionId);
}

void CJUIExtensionObject::OnForeground()
{
    if (g_cjFuncs.cjExtAbilityOnForeground == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnForeground is not registered");
        return;
    }

    g_cjFuncs.cjExtAbilityOnForeground(cjID_, GetType());
}

void CJUIExtensionObject::OnBackground()
{
    if (g_cjFuncs.cjExtAbilityOnBackground == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnBackground is not registered");
        return;
    }

    g_cjFuncs.cjExtAbilityOnBackground(cjID_, GetType());
}

void CJUIExtensionObject::OnConfigurationUpdate(std::shared_ptr<AppExecFwk::Configuration> fullConfig)
{
    if (g_cjFuncsV2.cjExtAbilityOnConfigurationUpdateV2 != nullptr) {
        auto cfg = CreateCConfigurationV2(*fullConfig);
        g_cjFuncsV2.cjExtAbilityOnConfigurationUpdateV2(cjID_, GetType(), cfg);
        FreeCConfigurationV2(&cfg);
        return;
    }
    if (g_cjFuncs.cjExtAbilityOnConfigurationUpdate == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnConfigurationUpdate is not registered");
        return;
    }

    auto cfg = CreateCConfiguration(*fullConfig);
    g_cjFuncs.cjExtAbilityOnConfigurationUpdate(cjID_, GetType(), cfg);
    FreeCConfiguration(&cfg);
}

void CJUIExtensionObject::OnMemoryLevel(int level)
{
    if (g_cjFuncs.cjExtAbilityOnMemoryLevel == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnMemoryLevel is not registered");
        return;
    }

    g_cjFuncs.cjExtAbilityOnMemoryLevel(cjID_, GetType(), level);
}

void CJUIExtensionObject::OnStartContentEditing(const std::string& imageUri, const AAFwk::Want &want, int64_t sessionId)
{
    if (g_cjFuncs.cjExtAbilityOnStartContentEditing == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjExtAbilityOnStartContentEditing is not registered");
        return;
    }

    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);

    char* cstr = CreateCStringFromString(imageUri);
    if (cstr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnStartContentEditing failed");
        return;
    }

    g_cjFuncs.cjExtAbilityOnStartContentEditing(cjID_, GetType(), cstr, wantHandle, sessionId);
}

extern "C" {
CJ_EXPORT void FFIRegisterCJExtAbilityFuncs(void (*registerFunc)(CJExtAbilityFuncs*))
{
    TAG_LOGD(AAFwkTag::UI_EXT, "FFIRegisterCJExtAbilityFuncs start");
    if (g_cjFuncs.createCjExtAbility != nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Repeated registration for cj functions of CJExtAbility");
        return;
    }

    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FFIRegisterCJExtAbilityFuncs failed, registerFunc is nullptr");
        return;
    }

    registerFunc(&g_cjFuncs);
    TAG_LOGD(AAFwkTag::UI_EXT, "FFIRegisterCJExtAbilityFuncs end");
}

CJ_EXPORT void FFIRegisterCJExtAbilityFuncsV2(void (*registerFunc)(CJExtAbilityFuncsV2*))
{
    TAG_LOGD(AAFwkTag::UI_EXT, "FFIRegisterCJExtAbilityFuncsV2 start");
    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FFIRegisterCJExtAbilityFuncsV2 failed, registerFunc is nullptr");
        return;
    }

    registerFunc(&g_cjFuncsV2);
    TAG_LOGD(AAFwkTag::UI_EXT, "FFIRegisterCJExtAbilityFuncsV2 end");
}
} // extern "C"
} // namespace AbilityRuntime
} // namespace OHOS
