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

#include "appstatecallbackhost_fuzzer.h"
#include "app_process_data.h"
#include "app_state_callback_host.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "bundle_info.h"
#include "configuration.h"
#include "fuzz_util.h"
#include "iapp_state_callback.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
}

class AppStateCallbackHostFuzz : public AppStateCallbackHost {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 11) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED);
            AppProcessData processData;
            parcel.WriteParcelable(&processData);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_ABILITY_REQUEST_DONE);
            bool flag = fdp.ConsumeBool();
            parcel.WriteBool(flag);
            if (flag) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_CONFIG_CHANGE);
            Configuration config;
            parcel.WriteParcelable(&config);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_RESIDENT_PROCESS);
            int32_t infoSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(infoSize);
            for (int32_t i = 0; i < infoSize; i++) {
                BundleInfo bundleInfo;
                parcel.WriteParcelable(&bundleInfo);
            }
            break;
        }
        case 4: {
            actualCode =
                static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_KEEP_ALIVE_PROCESS);
            int32_t infoSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(infoSize);
            for (int32_t i = 0; i < infoSize; i++) {
                BundleInfo bundleInfo;
                parcel.WriteParcelable(&bundleInfo);
            }
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_REMOTE_DIED);
            int32_t tokenSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(tokenSize);
            for (int32_t i = 0; i < tokenSize; i++) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_START_PROCESS_FAILED);
            int32_t tokenSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(tokenSize);
            for (int32_t i = 0; i < tokenSize; i++) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_TERMINATE_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_CACHE_EXIT_INFO);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            RunningProcessInfo exitInfo;
            parcel.WriteParcelable(&exitInfo);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            std::vector<std::string> abilityNames = FuzzUtil::BuildMaliciousStringVector(fdp);
            parcel.WriteStringVector(abilityNames);
            std::vector<std::string> uiExtensionNames = FuzzUtil::BuildMaliciousStringVector(fdp);
            parcel.WriteStringVector(uiExtensionNames);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_PRE_CACHE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_RECORD_APP_EXIT_SIGNAL_REASON);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AppStateCallbackHostFuzz, FuzzUtil::Tokens::APP_STATE_CALLBACK)
} // namespace OHOS
