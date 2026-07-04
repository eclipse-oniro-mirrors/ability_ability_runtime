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

#include "agent_utils.h"

#include <cerrno>
#include <cstddef>
#include <limits>

#include <sys/random.h>

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int64_t MIN_VERIFICATION_NONCE = 1000000000;
}

int64_t GenerateVerificationNonce()
{
    uint64_t randomValue = 0;
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&randomValue);
    size_t offset = 0;
    while (offset < sizeof(randomValue)) {
        auto readSize = getrandom(buffer + offset, sizeof(randomValue) - offset, 0);
        if (readSize < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        if (readSize == 0) {
            return 0;
        }
        offset += static_cast<size_t>(readSize);
    }
    int64_t nonce =
        static_cast<int64_t>(randomValue & static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
    if (nonce < MIN_VERIFICATION_NONCE) {
        nonce += MIN_VERIFICATION_NONCE;
    }
    return nonce;
}

bool IsMatchedAgentCardTarget(const AAFwk::Want &want, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return false;
    }

    const auto &element = want.GetElement();
    if (element.GetBundleName() != card.appInfo->bundleName ||
        element.GetAbilityName() != card.appInfo->abilityName) {
        return false;
    }

    return element.GetModuleName().empty() || card.appInfo->moduleName.empty() ||
        element.GetModuleName() == card.appInfo->moduleName;
}

void NormalizeAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return;
    }
    auto element = connectWant.GetElement();
    if (element.GetModuleName().empty() && !card.appInfo->moduleName.empty()) {
        element.SetModuleName(card.appInfo->moduleName);
        connectWant.SetElement(element);
    }
}
} // namespace AgentRuntime
} // namespace OHOS
