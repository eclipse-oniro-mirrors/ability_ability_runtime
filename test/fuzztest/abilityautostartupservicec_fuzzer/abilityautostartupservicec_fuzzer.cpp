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

#include "abilityautostartupservicec_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define private public
#include "ability_auto_startup_service.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
// App index 0 selects the normal (non-clone, non-sandbox) GetBundleInfo branch.
constexpr int32_t DEFAULT_APP_INDEX = 0;

struct AutoStartupFuzzParam {
    std::string bundleName;
    std::string abilityName;
    std::string moduleName;
    int32_t intParam = 0;
    bool boolParam = false;
    AutoStartupInfo info;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
};
} // namespace

void ExerciseStartupHelpers(const std::shared_ptr<AbilityAutoStartupService> &service,
    const AutoStartupFuzzParam &param)
{
    service->GetValidUserId(param.intParam);
    service->GetAbilityTypeName(param.abilityInfo);
    service->GetExtensionTypeName(param.extensionInfo);
    service->IsTargetAbility(param.info, param.abilityInfo);
    service->IsTargetExtension(param.info, param.extensionInfo);
    service->CheckSelfApplication(param.info.bundleName);
    service->GetSelfApplicationBundleName();
    service->GetBundleMgrClient();

    AppExecFwk::BundleInfo bundleInfo;
    service->GetBundleInfo(param.info.bundleName, param.intParam, param.intParam, bundleInfo);
    service->GetBundleInfo(param.info.bundleName, param.intParam, DEFAULT_APP_INDEX, bundleInfo);
    service->CheckPermissionForSystem();
    service->CheckPermissionForEDM();
}

void ExerciseEdmPaths(const std::shared_ptr<AbilityAutoStartupService> &service,
    const AutoStartupFuzzParam &param)
{
    service->InnerApplicationAutoStartupByEDM(param.info, param.boolParam, param.boolParam);

    std::vector<sptr<IRemoteObject>> callbackVector;
    service->GetCallbackVector(callbackVector);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<AbilityAutoStartupService> service = std::make_shared<AbilityAutoStartupService>();
    AutoStartupFuzzParam param;
    param.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.intParam = fdp.ConsumeIntegral<int32_t>();
    param.boolParam = fdp.ConsumeBool();

    param.info.bundleName = param.bundleName;
    param.info.moduleName = param.moduleName;
    param.info.abilityName = param.abilityName;
    param.info.appCloneIndex = param.intParam;
    param.info.accessTokenId = param.bundleName;
    param.info.setterUserId = param.intParam;
    param.info.userId = param.intParam;

    param.abilityInfo.bundleName = param.bundleName;
    param.abilityInfo.name = param.abilityName;
    param.abilityInfo.moduleName = param.moduleName;
    param.abilityInfo.type = static_cast<AppExecFwk::AbilityType>(fdp.ConsumeIntegral<int32_t>());

    param.extensionInfo.bundleName = param.bundleName;
    param.extensionInfo.name = param.abilityName;
    param.extensionInfo.moduleName = param.moduleName;
    param.extensionInfo.type = static_cast<AppExecFwk::ExtensionAbilityType>(fdp.ConsumeIntegral<int32_t>());

    ExerciseStartupHelpers(service, param);
    ExerciseEdmPaths(service, param);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
