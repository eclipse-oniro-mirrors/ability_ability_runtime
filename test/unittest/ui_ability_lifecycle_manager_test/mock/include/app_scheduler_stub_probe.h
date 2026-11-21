/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_STUB_PROBE_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_STUB_PROBE_H

#include <cstdint>

namespace OHOS {
namespace AAFwk {
/*
 * Probe into the link-time AppScheduler stub. The AppScheduler singleton is
 * shared with libabilityms.z.so via ELF symbol interposition, so the mock
 * header must stay a byte-identical copy of the real one (single definition,
 * matching layout) and the probe state stays OUT of the class.
 */
struct AppSchedulerStubState {
    // true: registration succeeds with incrementing handles (default: fail).
    bool attributeSucceeds = false;
    int32_t attributeCalls = 0;
    int32_t unregisterCalls = 0;
    int64_t nextAttributionId = 4701;
};

AppSchedulerStubState &AppSchedulerStub();
void AppSchedulerStubReset();
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_STUB_PROBE_H
