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

#include "bundlemgrhelper_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#define private public
#define protected public
#include "bundle_mgr_helper.h"
#include "ability_keep_alive_service.h"
#include "bundle_info.h"
#include "main_thread.h"
#include "connection_observer.h"
#undef protected
#undef private
#include "ability_fuzz_util.h"
#include "ability_record.h"
#include "app_mgr_client.h"
#include "continuous_task_callback_info.h"
#include "keep_alive_process_manager.h"
#include "parameters.h"
#include "permission_verification.h"


using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr size_t STRING_MAX_LENGTH = 128;
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

void BundleMgrHelperFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<BundleMgrHelper> bmHelper = std::make_shared<BundleMgrHelper>(); // branch constructor
    bmHelper->GetNameForUid(int32Param, stringParam); // branch
    BundleInfo bundleInfo;
    bmHelper->GetBundleInfo(stringParam, boolParam, bundleInfo, int32Param); // branch
    bmHelper->InstallSandboxApp(stringParam, int32Param, int32Param, int32Param); // branch
    bmHelper->UninstallSandboxApp(stringParam, int32Param, int32Param); // branch
    bmHelper->GetUninstalledBundleInfo(stringParam, bundleInfo); // branch
    bmHelper->GetSandboxBundleInfo(stringParam, int32Param, int32Param, bundleInfo); // branch
    Want want;
    AbilityInfo abilityInfo;
    bmHelper->GetSandboxAbilityInfo(want, int32Param, int32Param, int32Param, abilityInfo);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    bmHelper->GetSandboxExtAbilityInfos(want, int32Param, int32Param, int32Param, extensionInfos);
    HapModuleInfo hapModuleInfo;
    bmHelper->GetSandboxHapModuleInfo(abilityInfo, int32Param, int32Param, hapModuleInfo);
    bmHelper->Connect();
    bmHelper->ConnectBundleInstaller();
    bmHelper->OnDeath();
    bmHelper->GetBundleInfo(stringParam, int32Param, bundleInfo, int32Param);
    bmHelper->GetHapModuleInfo(abilityInfo, hapModuleInfo);
    bmHelper->GetAbilityLabel(stringParam, stringParam);
    bmHelper->GetAppType(stringParam);
    std::vector<BaseSharedBundleInfo> baseSharedBundleInfos;
    bmHelper->GetBaseSharedBundleInfos(
        stringParam, baseSharedBundleInfos, static_cast<GetDependentBundleInfoFlag>(int32Param));
    bmHelper->GetBundleInfoForSelf(int32Param, bundleInfo);
    bmHelper->GetDependentBundleInfo(stringParam, bundleInfo, static_cast<GetDependentBundleInfoFlag>(int32Param));
    bmHelper->GetGroupDir(stringParam, stringParam);
    bmHelper->GetOverlayManagerProxy();
    bmHelper->QueryAbilityInfo(want, abilityInfo);
    bmHelper->QueryAbilityInfo(want, int32Param, int32Param, abilityInfo);
    std::vector<BundleInfo> bundleInfos;
    bmHelper->GetBundleInfos(int32Param, bundleInfos, int32Param);
    bmHelper->GetBundleInfos(static_cast<BundleFlag>(int32Param), bundleInfos, int32Param);
    bmHelper->GetQuickFixManagerProxy();
    bmHelper->ProcessPreload(want);
    bmHelper->GetAppControlProxy();
    bmHelper->QueryExtensionAbilityInfos(want, int32Param, int32Param, extensionInfos);
    bmHelper->GetBundleInfoV9(stringParam, int32Param, bundleInfo, int32Param);
    bmHelper->ConnectTillSuccess();
    bmHelper->SetBmsReady(boolParam);
    bmHelper->GetBundleInfoForSelfWithOutCache(int32Param, bundleInfo);
    ExtensionAbilityInfo pluginExtensionInfo;
    bmHelper->GetPluginExtensionInfo(stringParam, want, int32Param, pluginExtensionInfo);
}

void BundleMgrHelperFuzztest2(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<BundleMgrHelper> bmHelper = std::make_shared<BundleMgrHelper>(); // branch constructor
    ApplicationInfo appInfo;
    Want want;
    bmHelper->GetApplicationInfo(stringParam, static_cast<ApplicationFlag>(int32Param), int32Param, appInfo);
    bmHelper->GetApplicationInfo(stringParam, int32Param, int32Param, appInfo);
    bmHelper->GetApplicationInfoWithAppIndex(stringParam, int32Param, int32Param, appInfo);
    bmHelper->UnregisterBundleEventCallback(nullptr); // branch null
    ExtensionAbilityInfo extensionAbilityInfo;
    bmHelper->QueryExtensionAbilityInfoByUri(stringParam, int32Param, extensionAbilityInfo);
    AbilityInfo abilityInfo;
    bmHelper->ImplicitQueryInfoByPriority(want, int32Param, int32Param, abilityInfo, extensionAbilityInfo);
    bmHelper->QueryAbilityInfoByUri(stringParam, int32Param, abilityInfo);
    bmHelper->QueryAbilityInfo(want, int32Param, int32Param, abilityInfo, nullptr);
    bmHelper->UpgradeAtomicService(want, int32Param);
    std::vector<AbilityInfo> abilityInfos;
    std::vector<ExtensionAbilityInfo> extensionInfos;
    bmHelper->ImplicitQueryInfos(want, int32Param, int32Param, boolParam, abilityInfos, extensionInfos, boolParam);
    bmHelper->CleanBundleDataFiles(stringParam, int32Param, int32Param, int32Param);
    std::vector<DataGroupInfo> infos;
    bmHelper->QueryDataGroupInfos(stringParam, int32Param, infos);
    bmHelper->RegisterBundleEventCallback(nullptr);
    HapModuleInfo hapModuleInfo;
    bmHelper->GetHapModuleInfo(abilityInfo, int32Param, hapModuleInfo);
    bmHelper->QueryAppGalleryBundleName(stringParam);
    bmHelper->GetUidByBundleName(stringParam, int32Param, int32Param);
    bmHelper->QueryExtensionAbilityInfosOnlyWithTypeName(stringParam, int32Param, int32Param, extensionInfos);
    bmHelper->GetDefaultAppProxy();
    bmHelper->GetJsonProfile(static_cast<ProfileType>(int32Param), stringParam, stringParam, stringParam, int32Param);
    bmHelper->GetLaunchWantForBundle(stringParam, want, int32Param);
    ElementName element;
    bmHelper->QueryCloneAbilityInfo(element, int32Param, int32Param, abilityInfo, int32Param);
    BundleInfo bundleInfo;
    bmHelper->GetCloneBundleInfo(stringParam, int32Param, int32Param, bundleInfo, int32Param);
    ExtensionAbilityInfo extensionInfo;
    bmHelper->QueryCloneExtensionAbilityInfoWithAppIndex(element, int32Param, int32Param, extensionInfo, int32Param);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    BundleMgrHelperFuzztest1(boolParam, stringParam, int32Param);
    BundleMgrHelperFuzztest2(boolParam, stringParam, int32Param);
    // Merged APIs
    bmHelper->GetTestRunnerTypeAndPath(bundleName, moduleName, testRunner);
    // Merged APIs
    bmHelper->GetPluginInfoForTarget(hostBundleName, pluginBundleName,


    return true;
}


bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::shared_ptr<BundleMgrHelper> bmHelper = std::make_shared<BundleMgrHelper>();
    std::string bundleName;
    int32_t userId;
    bool bmsReady;
    std::string hostBundleName;
    std::string pluginBundleName;
    std::string pluginModuleName;
    BundleInfo bundleInfo;
    int32_t flags;
    std::vector<int32_t> appIndexes;
    SignatureInfo signatureInfo;
    HapModuleInfo hapModuleInfo;
    std::string moduleName;
    int32_t resId;
    int32_t appIndex;
    sptr<IBundleEventCallback> pluginEventCallback;
    std::vector<PluginBundleInfo> pluginBundleInfos;
    FuzzedDataProvider fdp(data, size);
    bundleName = fdp.ConsumeRandomLengthString(256);
    hostBundleName = fdp.ConsumeRandomLengthString(256);
    pluginBundleName = fdp.ConsumeRandomLengthString(128);
    moduleName = fdp.ConsumeRandomLengthString(128);
    pluginModuleName = fdp.ConsumeRandomLengthString(64);
    userId = fdp.ConsumeIntegral<int32_t>();
    flags = fdp.ConsumeIntegral<int32_t>();
    resId = fdp.ConsumeIntegral<int32_t>();
    appIndex = fdp.ConsumeIntegral<int32_t>();
    bmsReady = fdp.ConsumeBool();
    AbilityFuzzUtil::GetRandomBundleInfo(fdp, bundleInfo);
    AbilityFuzzUtil::GenerateSignatureInfo(fdp, signatureInfo);
    bmHelper->PreConnect();
    bmHelper->GetAppIdByBundleName(bundleName, userId);
    bmHelper->ConnectTillSuccess();
    bmHelper->SetBmsReady(bmsReady);
    bmHelper->GetPluginHapModuleInfo(hostBundleName, pluginBundleName, pluginModuleName, userId, hapModuleInfo);
    bmHelper->GetBundleInfoForSelfWithOutCache (flags, bundleInfo);
    bmHelper->GetCloneAppIndexes(bundleName, appIndexes, userId);
    bmHelper->GetSignatureInfoByBundleName(bundleName, signatureInfo);
    bmHelper->GetStringById(bundleName, moduleName, resId, userId);
    bmHelper->GetDataDir(bundleName, appIndex);
    bmHelper->GetPluginInfosForSelf(pluginBundleInfos);
    bmHelper->RegisterPluginEventCallback(pluginEventCallback);
    bmHelper->UnregisterPluginEventCallback(pluginEventCallback);
    return true;
}
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

