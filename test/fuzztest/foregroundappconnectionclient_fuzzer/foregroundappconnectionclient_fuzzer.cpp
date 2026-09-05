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

#include "foregroundappconnectionclient_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <memory>

#include "foreground_app_connection.h"
#include "foreground_app_connection_client.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace {
class ForegroundAppConnectionFuzz : public ForegroundAppConnection {
public:
    void OnForegroundAppConnected(const ForegroundAppConnectionData &data) override {}
    void OnForegroundAppDisconnected(const ForegroundAppConnectionData &data) override {}
    void OnForegroundAppCallerStarted(int32_t callerPid, int32_t callerUid,
        const std::string &bundleName) override {}
};
} // namespace

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    auto observer = std::make_shared<ForegroundAppConnectionFuzz>();
    auto &client = ForegroundAppConnectionClient::GetInstance();

    client.RegisterObserver(observer);
    client.UnregisterObserver(observer);

    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
