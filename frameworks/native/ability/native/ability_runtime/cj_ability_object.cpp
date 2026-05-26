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

#include "ability_runtime/cj_ability_object.h"

#include "cj_common_ffi.h"
#include "cj_utils_ffi.h"
#include "hilog_tag_wrapper.h"
#include "want_params_wrapper.h"

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

using WantHandle = void*;

namespace {
// g_cjAbilityFuncs is used to save cj functions.
// It is assigned by the global variable REGISTER_ABILITY on the cj side which invokes RegisterCJAbilityFuncs.
CJAbilityFuncs g_cjAbilityFuncs {};
CJAbilityFuncsV3 g_cjAbilityFuncsV3 {};

const char* CJ_ABILITY_LIBNAME = "libcj_ability_ffi.z.so";
const char* FUNC_CONVERT_CONFIGURATION = "OHOS_ConvertConfiguration";
const char* FUNC_CONVERT_CONFIGURATION_V2 = "OHOS_ConvertConfigurationV2";
const char* FUNC_FREE_CONFIGURATION_V2 = "OHOS_FreeConfigurationV2";
} // namespace

void RegisterCJAbilityFuncs(void (*registerFunc)(CJAbilityFuncs*))
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (g_cjAbilityFuncs.cjAbilityCreate != nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "repeated registration");
        return;
    }

    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null registerFunc");
        return;
    }

    registerFunc(&g_cjAbilityFuncs);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void RegisterCJAbilityFuncsV3(void (*registerFunc)(CJAbilityFuncsV3*))
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null registerFunc");
        return;
    }

    registerFunc(&g_cjAbilityFuncsV3);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

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
        TAG_LOGE(AAFwkTag::DEFAULT, "null res");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::DEFAULT, "Strcpy failed");
        return nullptr;
    }
    return res;
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

CConfiguration CallConvertConfig(std::shared_ptr<AppExecFwk::Configuration> configuration)
{
    CConfiguration cCfg = {};
    void* handle = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return cCfg;
    }
    using ConvertConfigFunc = CConfiguration (*)(void*);
    auto func = reinterpret_cast<ConvertConfigFunc>(dlsym(handle, FUNC_CONVERT_CONFIGURATION));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return cCfg;
    }
    cCfg = func(configuration.get());
    dlclose(handle);
    return cCfg;
}

CConfigurationV2 CallConvertConfigV2(std::shared_ptr<AppExecFwk::Configuration> configuration)
{
    CConfigurationV2 cCfg = {};
    void* handle = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return cCfg;
    }
    using ConvertConfigFunc = CConfigurationV2 (*)(void*);
    auto func = reinterpret_cast<ConvertConfigFunc>(dlsym(handle, FUNC_CONVERT_CONFIGURATION_V2));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return cCfg;
    }
    cCfg = func(configuration.get());
    dlclose(handle);
    return cCfg;
}

void CallFreeConfigV2(CConfigurationV2 cCfg)
{
    void* handle = dlopen(CJ_ABILITY_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null handle");
        return;
    }
    using FreeConfigFunc = void (*)(CConfigurationV2*);
    auto func = reinterpret_cast<FreeConfigFunc>(dlsym(handle, FUNC_FREE_CONFIGURATION_V2));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null func");
        dlclose(handle);
        return;
    }
    func(&cCfg);
    dlclose(handle);
}

std::shared_ptr<CJAbilityObject> CJAbilityObject::LoadModule(const std::string& name)
{
    if (g_cjAbilityFuncs.cjAbilityCreate == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return nullptr;
    }
    auto id = g_cjAbilityFuncs.cjAbilityCreate(name.c_str());
    if (id == 0) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not registered %{public}s", name.c_str());
        return nullptr;
    }
    return std::make_shared<CJAbilityObject>(id);
}

CJAbilityObject::~CJAbilityObject()
{
    if (g_cjAbilityFuncs.cjAbilityRelease != nullptr) {
        g_cjAbilityFuncs.cjAbilityRelease(id_);
    }
    id_ = 0;
}

void CJAbilityObject::OnStart(const AAFwk::Want& want, const AAFwk::LaunchParam& launchParam) const
{
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    if (g_cjAbilityFuncsV3.cjAbilityOnStartV3 != nullptr) {
        CJLaunchParamV3 param;
        param.launchReason = launchParam.launchReason;
        param.lastExitReason = launchParam.lastExitReason;
        param.lastExitMessage = CreateCStringFromString(launchParam.lastExitMessage);
        param.launchReasonMessage = CreateCStringFromString(launchParam.launchReasonMessage);
        param.lastExitDetailInfo = CreateCJLastExitDetailInfo(launchParam.lastExitDetailInfo);
        g_cjAbilityFuncsV3.cjAbilityOnStartV3(id_, wantHandle, param);
        free(param.lastExitMessage);
        free(param.launchReasonMessage);
        FreeCJLastExitDetailInfo(param.lastExitDetailInfo);
        return;
    }
    if (g_cjAbilityFuncs.cjAbilityOnStart == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    param.lastExitMessage = CreateCStringFromString(launchParam.lastExitMessage);
    g_cjAbilityFuncs.cjAbilityOnStart(id_, wantHandle, param);
    free(param.lastExitMessage);
}

void CJAbilityObject::OnStop() const
{
    if (g_cjAbilityFuncs.cjAbilityOnStop == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    g_cjAbilityFuncs.cjAbilityOnStop(id_);
}

void CJAbilityObject::OnSceneCreated(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs.cjAbilityOnSceneCreated == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs.cjAbilityOnSceneCreated(id_, windowStage);
}

void CJAbilityObject::OnSceneRestored(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs.cjAbilityOnSceneRestored == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs.cjAbilityOnSceneRestored(id_, windowStage);
}

void CJAbilityObject::OnSceneWillDestroy(OHOS::Rosen::CJWindowStageImpl* cjWindowStage) const
{
    if (g_cjAbilityFuncs.cjAbilityOnSceneWillDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    g_cjAbilityFuncs.cjAbilityOnSceneWillDestroy(id_, windowStage);
}

void CJAbilityObject::OnSceneDestroyed() const
{
    if (g_cjAbilityFuncs.cjAbilityOnSceneDestroyed == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    g_cjAbilityFuncs.cjAbilityOnSceneDestroyed(id_);
}

void CJAbilityObject::OnForeground(const Want& want) const
{
    if (g_cjAbilityFuncs.cjAbilityOnForeground == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    g_cjAbilityFuncs.cjAbilityOnForeground(id_, wantHandle);
}

void CJAbilityObject::OnBackground() const
{
    if (g_cjAbilityFuncs.cjAbilityOnBackground == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    g_cjAbilityFuncs.cjAbilityOnBackground(id_);
}

bool CJAbilityObject::OnBackPress(bool defaultRet) const
{
    if (g_cjAbilityFuncs.cjAbilityOnBackPress == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return defaultRet;
    }
    return g_cjAbilityFuncs.cjAbilityOnBackPress(id_);
}

bool CJAbilityObject::OnPrepareTerminate() const
{
    if (g_cjAbilityFuncs.cjAbilityOnPrepareTerminate == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return false;
    }
    return g_cjAbilityFuncs.cjAbilityOnPrepareTerminate(id_);
}

void CJAbilityObject::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const
{
    if (g_cjAbilityFuncsV3.cjAbilityOnConfigurationUpdateV3 != nullptr) {
        auto cfg = CallConvertConfigV2(configuration);
        g_cjAbilityFuncsV3.cjAbilityOnConfigurationUpdateV3(id_, cfg);
        CallFreeConfigV2(cfg);
        return;
    }
    if (g_cjAbilityFuncs.cjAbilityOnConfigurationUpdate == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    auto cfg = CallConvertConfig(configuration);
    return g_cjAbilityFuncs.cjAbilityOnConfigurationUpdate(id_, cfg);
}

void CJAbilityObject::OnMemoryLevel(int32_t level) const
{
    if (g_cjAbilityFuncs.cjAbilityOnMemoryLevel == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    g_cjAbilityFuncs.cjAbilityOnMemoryLevel(id_, level);
}

void CJAbilityObject::OnNewWant(const AAFwk::Want& want, const AAFwk::LaunchParam& launchParam) const
{
    WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
    if (g_cjAbilityFuncsV3.cjAbilityOnNewWantV3 != nullptr) {
        CJLaunchParamV3 param;
        param.launchReason = launchParam.launchReason;
        param.lastExitReason = launchParam.lastExitReason;
        param.lastExitMessage = CreateCStringFromString(launchParam.lastExitMessage);
        param.launchReasonMessage = CreateCStringFromString(launchParam.launchReasonMessage);
        param.lastExitDetailInfo = CreateCJLastExitDetailInfo(launchParam.lastExitDetailInfo);
        g_cjAbilityFuncsV3.cjAbilityOnNewWantV3(id_, wantHandle, param);
        free(param.lastExitMessage);
        free(param.launchReasonMessage);
        FreeCJLastExitDetailInfo(param.lastExitDetailInfo);
        return;
    }
    if (g_cjAbilityFuncs.cjAbilityOnNewWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    CJLaunchParam param;
    param.launchReason = launchParam.launchReason;
    param.lastExitReason = launchParam.lastExitReason;
    param.lastExitMessage = CreateCStringFromString(launchParam.lastExitMessage);
    g_cjAbilityFuncs.cjAbilityOnNewWant(id_, wantHandle, param);
    free(param.lastExitMessage);
}

void CJAbilityObject::Dump(const std::vector<std::string>& params, std::vector<std::string>& info) const
{
    if (g_cjAbilityFuncs.cjAbilityDump == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }

    VectorStringHandle paramHandle = const_cast<std::vector<std::string>*>(&params);
    VectorStringHandle cjInfo = g_cjAbilityFuncs.cjAbilityDump(id_, paramHandle);
    if (cjInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cj info");
        return;
    }

    auto infoHandle = reinterpret_cast<std::vector<std::string>*>(cjInfo);
    for (std::string item : *infoHandle) {
        info.push_back(item);
    }
    // infoHandle is created in cj.
    delete infoHandle;
    infoHandle = nullptr;
}

int32_t CJAbilityObject::OnContinue(AAFwk::WantParams& wantParams) const
{
    if (g_cjAbilityFuncs.cjAbilityOnContinueWithParams == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return ContinuationManager::OnContinueResult::ON_CONTINUE_ERR;
    }
    auto params = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(wantParams).ToString());
    auto cjNumberParmas = g_cjAbilityFuncs.cjAbilityOnContinueWithParams(id_, params);
    auto returnParams = std::string(cjNumberParmas.params);
    free(params);
    free(cjNumberParmas.params);
    wantParams = OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(returnParams);
    return cjNumberParmas.numberResult;
}

int32_t CJAbilityObject::OnSaveState(int32_t reason, WantParams &wantParams) const
{
    if (g_cjAbilityFuncs.cjAbilityOnSaveState == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return -1;
    }
    auto params = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(wantParams).ToString());
    auto cjNumberParmas = g_cjAbilityFuncs.cjAbilityOnSaveState(id_, reason, params);
    auto returnParams = std::string(cjNumberParmas.params);
    free(params);
    free(cjNumberParmas.params);
    wantParams = OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(returnParams);
    return cjNumberParmas.numberResult;
}

int32_t CJAbilityObject::OnShare(WantParams &wantParams) const
{
    if (g_cjAbilityFuncs.cjAbilityOnShare == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return -1;
    }
    auto params = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(wantParams).ToString());
    auto cJReturnParams = g_cjAbilityFuncs.cjAbilityOnShare(id_, params);
    auto returnParams = std::string(cJReturnParams);
    free(params);
    free(cJReturnParams);
    wantParams = OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(returnParams);
    return ERR_OK;
}

int64_t CJAbilityObject::OnCallRequest() const
{
    if (g_cjAbilityFuncs.cjAbilityOnCallRequest == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return -1;
    }
    return g_cjAbilityFuncs.cjAbilityOnCallRequest(id_);
}

void CJAbilityObject::OnSetCalleeFlag(bool flag) const
{
    if (g_cjAbilityFuncs.cjAbilityOnSetCalleeFlag == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    return g_cjAbilityFuncs.cjAbilityOnSetCalleeFlag(id_, flag);
}

void CJAbilityObject::Init(AbilityHandle ability) const
{
    if (g_cjAbilityFuncs.cjAbilityInit == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cjAbilityFunc");
        return;
    }
    g_cjAbilityFuncs.cjAbilityInit(id_, ability);
}

int64_t CJAbilityObject::GetId() const
{
    return id_;
}
} // namespace AbilityRuntime
} // namespace OHOS
