/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_AGENT_RUNTIME_MY_FLAG_H
#define MOCK_AGENT_RUNTIME_MY_FLAG_H

#include "agent_card.h"

namespace OHOS {
namespace AgentRuntime {
class MyFlag {
public:
    static int32_t retInsertData;
    static int32_t retDeleteData;
    static int32_t retQueryData;
    static int32_t retQueryAllData;
    static std::vector<AgentCard> queryDataCards;
    static std::vector<AgentCard> queryAllDataCards;
    static bool retGetBundleInfo;
    static bool retGetResConfigFile;
    static bool retFromJson;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif // MOCK_AGENT_RUNTIME_MY_FLAG_H
