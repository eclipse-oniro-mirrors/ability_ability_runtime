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

#include "appstateobservermanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_state_observer_manager.h"
#include "appspawn.h"
#undef private
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << OFFSET_ZERO) | (ptr[1] << OFFSET_ONE) | (ptr[2] << OFFSET_TWO) | ptr[3];
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<AppStateObserverManager> appStateObserverManager = std::make_shared<AppStateObserverManager>();
    if (appStateObserverManager == nullptr) {
        return false;
    }
    sptr<IApplicationStateObserver> observer;
    std::vector<std::string> bundleNameList;
    appStateObserverManager->RegisterApplicationStateObserver(observer, bundleNameList);
    appStateObserverManager->UnregisterApplicationStateObserver(observer);
    std::shared_ptr<AppRunningRecord> appRecord;
    ProcessData processData;
    BundleType bundleType = BundleType::APP;
    appStateObserverManager->HandleOnAppProcessCreated(processData, bundleType);
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = *data % ENABLE;
    bool isFromWindowFocusChanged = *data % ENABLE;
    appStateObserverManager->HandleAppStateChanged(appRecord, state, needNotifyApp, isFromWindowFocusChanged, false);
    appStateObserverManager->HandleOnAppProcessDied(processData, bundleType);
    std::shared_ptr<RenderRecord> renderRecord;
    appStateObserverManager->HandleOnRenderProcessCreated(renderRecord, false);
    appStateObserverManager->HandleOnRenderProcessDied(renderRecord);
    appStateObserverManager->HandleOnProcessCreated(processData);
    appStateObserverManager->HandleOnProcessStateChanged(processData, bundleType, state, false);
    appStateObserverManager->HandleOnProcessDied(processData);
    AbilityStateData abilityStateData;
    bool isAbility = *data % ENABLE;
    appStateObserverManager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, isFromWindowFocusChanged);
    appStateObserverManager->HandleOnKeepAliveStateChanged(appRecord);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
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

