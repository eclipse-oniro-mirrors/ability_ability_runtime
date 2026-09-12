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

#include "abilitymanagerstubremaining13_fuzzer.h"
#include "ability_manager_stub.h"
#include "ability_manager_stub_mock.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "attack_vectors.h"
#include "fuzz_util.h"
#include "insight_intent_query_param.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr size_t HANDLE_COUNT = 12;
}

class AbilityManagerStubRemaining13Fuzz : public AbilityManagerStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_APPLICATION);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_RESTART_APP_LIMIT);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UN_PRELOAD_UI_EXTENSION_ABILITY);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_ALL_PRELOAD_UI_EXTENSION_ABILITY);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_PRELOAD_UI_EXTENSION_HOST_CLIENT);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_PRELOAD_UI_EXTENSION_HOST_CLIENT);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_USER_LOCKED_BUNDLE_LIST);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SELF_MODULAR_OBJECT_EXTENSION_INFOS);
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_GAME_PRELAUNCH);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETE_GAME_PRELAUNCH);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 10: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::INSIGHT_INTENT_QUERY_ENTITY);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteRemoteObject(nullptr);
            InsightIntentQueryParam param;
            param.bundleName_ = FuzzUtil::BuildMaliciousBundleName(fdp);
            param.moduleName_ = FuzzUtil::BuildSpecialCharString(fdp);
            param.intentName_ = FuzzUtil::BuildSpecialCharString(fdp);
            param.className_ = FuzzUtil::BuildSpecialCharString(fdp);
            param.queryEntityParam_.queryType_ = FuzzUtil::BuildSpecialCharString(fdp);
            param.queryEntityParam_.parameters_ = std::make_shared<WantParams>();
            param.userId_ = FuzzUtil::BuildIntegerOverflow(fdp);
            param.intentId_ = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteParcelable(&param);
            break;
        }
        case 11: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SELF);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        default:
            break;
    }

}

FUZZ_STUB_ENTRY_IMPL(AbilityManagerStubRemaining13Fuzz, FuzzUtil::Tokens::ABILITY_MGR)
} // namespace OHOS
