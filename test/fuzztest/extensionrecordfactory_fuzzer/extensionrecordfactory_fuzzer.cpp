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

#include "extensionrecordfactory_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define protected public
#include "extension_record_factory.h"
#undef protected

#include "base_extension_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
// Named constants for extension types used in the config map
constexpr int32_t EXT_TYPE_WORK_SCHEDULER =
    static_cast<int32_t>(ExtensionAbilityType::WORK_SCHEDULER);
constexpr int32_t EXT_TYPE_INPUTMETHOD =
    static_cast<int32_t>(ExtensionAbilityType::INPUTMETHOD);
constexpr int32_t EXT_TYPE_EMBEDDED_UI =
    static_cast<int32_t>(ExtensionAbilityType::EMBEDDED_UI);
constexpr int32_t EXT_TYPE_STATUS_BAR_VIEW =
    static_cast<int32_t>(ExtensionAbilityType::STATUS_BAR_VIEW);
constexpr int32_t EXT_TYPE_AWC_WEBPAGE =
    static_cast<int32_t>(ExtensionAbilityType::AWC_WEBPAGE);
constexpr int32_t EXT_TYPE_AWC_NEWSFEED =
    static_cast<int32_t>(ExtensionAbilityType::AWC_NEWSFEED);
constexpr int32_t EXT_TYPE_LIVE_FORM =
    static_cast<int32_t>(ExtensionAbilityType::LIVE_FORM);
constexpr int32_t EXT_TYPE_SYS_COMMON_UI =
    static_cast<int32_t>(ExtensionAbilityType::SYS_COMMON_UI);
constexpr int32_t EXT_TYPE_AGENT_UI =
    static_cast<int32_t>(ExtensionAbilityType::SYS_VISUAL);
constexpr int32_t EXT_TYPE_SERVICE =
    static_cast<int32_t>(ExtensionAbilityType::SERVICE);
constexpr int32_t EXT_TYPE_FORM =
    static_cast<int32_t>(ExtensionAbilityType::FORM);

// Named constants for fuzz operation selection
constexpr uint8_t OP_NEED_REUSE = 0;
constexpr uint8_t OP_PRE_CHECK = 1;
constexpr uint8_t OP_GET_PROCESS_MODE = 2;
constexpr uint8_t OP_CREATE_RECORD = 3;
constexpr uint8_t OP_COUNT = 4;

// Named constants for ExtensionProcessMode range
constexpr int32_t PROCESS_MODE_MIN =
    static_cast<int32_t>(ExtensionProcessMode::UNDEFINED);
constexpr int32_t PROCESS_MODE_MAX =
    static_cast<int32_t>(ExtensionProcessMode::RUN_WITH_MAIN_PROCESS);

// Named constants for string length
constexpr size_t BUNDLE_NAME_MAX_LEN = 128;

void FillAbilityRequest(FuzzedDataProvider &fdp, AbilityRequest &req)
{
    req.abilityInfo.name =
        fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    req.abilityInfo.bundleName =
        fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    req.abilityInfo.moduleName =
        fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    req.abilityInfo.process =
        fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    req.abilityInfo.isStageBasedModel = fdp.ConsumeBool();
    req.appInfo.bundleName = req.abilityInfo.bundleName;
    req.appInfo.name =
        fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    int32_t extType = fdp.ConsumeIntegralInRange<int32_t>(
        EXT_TYPE_FORM, EXT_TYPE_AGENT_UI);
    req.extensionType =
        static_cast<ExtensionAbilityType>(extType);
}

void SetupWantHostSpecified(AbilityRequest &req)
{
    req.want.SetParam(
        PROCESS_MODE_HOST_SPECIFIED_KEY, true);
}

void SetupWantHostInstance(
    FuzzedDataProvider &fdp, AbilityRequest &req)
{
    bool hostInstance = fdp.ConsumeBool();
    req.want.SetParam(
        PROCESS_MODE_HOST_INSTANCE_KEY, hostInstance);
}

void FuzzNeedReuse(
    FuzzedDataProvider &fdp,
    ExtensionRecordFactory &factory)
{
    AbilityRequest req;
    FillAbilityRequest(fdp, req);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    factory.NeedReuse(req, recordId);
}

void FuzzPreCheck(
    FuzzedDataProvider &fdp,
    ExtensionRecordFactory &factory)
{
    AbilityRequest req;
    FillAbilityRequest(fdp, req);
    std::string hostBundleName =
        fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    factory.PreCheck(req, hostBundleName);
}

void FuzzGetExtensionProcessMode(
    FuzzedDataProvider &fdp,
    ExtensionRecordFactory &factory)
{
    AbilityRequest req;
    FillAbilityRequest(fdp, req);

    // Optionally inject process mode keys into the Want
    uint8_t wantConfig = fdp.ConsumeIntegral<uint8_t>();
    switch (wantConfig % OP_COUNT) {
        case OP_NEED_REUSE:
            SetupWantHostSpecified(req);
            break;
        case OP_PRE_CHECK:
            SetupWantHostInstance(fdp, req);
            break;
        case OP_GET_PROCESS_MODE:
            SetupWantHostSpecified(req);
            SetupWantHostInstance(fdp, req);
            break;
        default:
            break;
    }

    // Optionally set customProcess for PROCESS_MODE_CUSTOM path
    if (fdp.ConsumeBool()) {
        req.customProcess =
            fdp.ConsumeRandomLengthString(BUNDLE_NAME_MAX_LEN);
    }

    // Set extensionProcessMode to a fuzzed value
    int32_t modeVal = fdp.ConsumeIntegralInRange<int32_t>(
        PROCESS_MODE_MIN, PROCESS_MODE_MAX);
    req.extensionProcessMode =
        static_cast<ExtensionProcessMode>(modeVal);

    bool isHostSpecified = false;
    factory.GetExtensionProcessMode(req, isHostSpecified);
}

void FuzzCreateRecord(
    FuzzedDataProvider &fdp,
    ExtensionRecordFactory &factory)
{
    AbilityRequest req;
    FillAbilityRequest(fdp, req);
    std::shared_ptr<ExtensionRecord> extensionRecord;
    factory.CreateRecord(req, extensionRecord);
}

void DispatchFuzzOperation(
    FuzzedDataProvider &fdp,
    ExtensionRecordFactory &factory)
{
    uint8_t op = fdp.ConsumeIntegralInRange<uint8_t>(
        0, OP_COUNT - 1);
    switch (op) {
        case OP_NEED_REUSE:
            FuzzNeedReuse(fdp, factory);
            break;
        case OP_PRE_CHECK:
            FuzzPreCheck(fdp, factory);
            break;
        case OP_GET_PROCESS_MODE:
            FuzzGetExtensionProcessMode(fdp, factory);
            break;
        case OP_CREATE_RECORD:
            FuzzCreateRecord(fdp, factory);
            break;
        default:
            break;
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto factory = std::make_shared<ExtensionRecordFactory>();

    // Consume remaining data across multiple fuzz iterations
    while (fdp.remaining_bytes() > 0) {
        DispatchFuzzOperation(fdp, *factory);
    }

    return true;
}
} // namespace
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
