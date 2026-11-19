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

#ifndef OHOS_ABILITY_RUNTIME_DUMP_ARKWEB_HELPER_H
#define OHOS_ABILITY_RUNTIME_DUMP_ARKWEB_HELPER_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AppExecFwk {

class DumpArkWebHelper {
public:
    DumpArkWebHelper();
    ~DumpArkWebHelper();
    
    static int DumpArkWeb(const std::string &customArgs, std::string &result);

    static void DumpArkWebJSHeap(int32_t appPid, int32_t renderPid, bool needSnapshot, bool needGC, bool needRaw);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DUMP_ARKWEB_HELPER_H
