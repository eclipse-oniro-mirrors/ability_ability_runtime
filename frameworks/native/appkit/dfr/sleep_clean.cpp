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

#include "sleep_clean.h"

#include "js_runtime.h"
#include "hilog_tag_wrapper.h"
#include "ohos_application.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "app_recovery.h"
#include "recovery_param.h"

namespace OHOS {
namespace AppExecFwk {

SleepClean &SleepClean::GetInstance()
{
    static SleepClean instance_;
    return instance_;
}

bool SleepClean::HandleAppSaveIfHeap(const std::shared_ptr<OHOSApplication> &application)
{
    auto appHeapTotalSize = GetHeapSize(application);
    TAG_LOGI(AAFwkTag::APPDFR, "appHeapTotalSize is %{public}s", std::to_string(appHeapTotalSize).c_str());
    if (appHeapTotalSize < APP_SAVE_HEAP_SIZE) {
        return false;
    }
    AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
    return true;
}

bool SleepClean::HandleSleepClean(const FaultData &faultData, const std::shared_ptr<OHOSApplication> &application)
{
    if (faultData.waitSaveState) {
        return HandleAppSaveIfHeap(application);
    }
    return AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
}

size_t SleepClean::GetHeapSize(const std::shared_ptr<OHOSApplication> &application)
{
    auto &runtime = application->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null runtime");
        return 0;
    }
    AbilityRuntime::JsRuntime *jsRuntime = static_cast<AbilityRuntime::JsRuntime *>(runtime.get());
    if (jsRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null runtime");
        return 0;
    }
    auto vm = jsRuntime->GetEcmaVm();
    if (!vm) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null runtime");
        return 0;
    }
    return DFXJSNApi::GetHeapTotalSize(vm);
}
}   //namespace AppExecFwk
}   //namespace OHOS