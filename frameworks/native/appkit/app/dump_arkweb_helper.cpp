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

#include "dump_arkweb_helper.h"

#include <cstdint>
#include <ctime>
#include <cstring>
#include <unistd.h>

#include "dfx_socket_request.h"
#include "faultloggerd_client.h"

#if defined(NWEB)
#include "nweb_helper.h"
#endif

namespace OHOS {
namespace AppExecFwk {
constexpr const uint32_t MAX_BUF_SIZE = (1U << 19);
int DumpArkWebHelper::DumpArkWeb(const std::string &customArgs, std::string &result)
{
    result.resize(MAX_BUF_SIZE);
#if defined(NWEB)
    OHOS::NWeb::NWebHelper &nWebHelper = OHOS::NWeb::NWebHelper::Instance();
    result = nWebHelper.DumpArkWebInfo(customArgs);
#endif
    auto size = result.size();
    if (size > 0) {
        result.resize(size);
        return 0;
    }
    result.resize(0);
    return -1;
}

// static
void DumpArkWebHelper::DumpArkWebJSHeap(int32_t appPid, int32_t renderPid, bool needSnapshot, bool needGC, bool needRaw)
{
    int32_t fd = -1;
    if (needSnapshot) {
        int32_t type = needRaw ? static_cast<int32_t>(FaultLoggerType::ARKWEB_JS_RAW_SNAPSHOT) :
                                 static_cast<int32_t>(FaultLoggerType::ARKWEB_JS_HEAP_SNAPSHOT);
        struct FaultLoggerdRequest request = {};
        request.pid = appPid;
        request.type = type;
        request.tid = renderPid;
        request.time = static_cast<uint64_t>(time(nullptr));
        fd = RequestFileDescriptorEx(&request);
        if (fd < 0) {
            return;
        }
    }
#if defined(NWEB)
    OHOS::NWeb::NWebHelper &nWebHelper = OHOS::NWeb::NWebHelper::Instance();
    nWebHelper.DumpArkWebJSHeap(fd, renderPid, needSnapshot, needGC, needRaw);
#endif
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
