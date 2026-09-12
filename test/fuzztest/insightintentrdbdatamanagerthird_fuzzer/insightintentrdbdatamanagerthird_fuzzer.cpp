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

#include "insightintentrdbdatamanagerthird_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <unordered_map>

#define private public
#include "insight_intent_rdb_data_mgr.h"
#undef private

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto rdbDataMgr = DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance();

    std::string key = fdp.ConsumeRandomLengthString(256);
    std::string value = fdp.ConsumeRandomLengthString(256);

    if (fdp.ConsumeBool()) {
        rdbDataMgr->InsertData(key, value);
    }

    if (fdp.ConsumeBool()) {
        rdbDataMgr->UpdateData(key, value);
    }

    if (fdp.ConsumeBool()) {
        rdbDataMgr->DeleteData(key);
    }

    if (fdp.ConsumeBool()) {
        rdbDataMgr->DeleteDataBeginWithKey(key);
    }

    if (fdp.ConsumeBool()) {
        std::string queryValue;
        rdbDataMgr->QueryData(key, queryValue);
    }

    if (fdp.ConsumeBool()) {
        std::unordered_map<std::string, std::string> datas;
        rdbDataMgr->QueryDataBeginWithKey(key, datas);
    }

    if (fdp.ConsumeBool()) {
        std::unordered_map<std::string, std::string> datas;
        rdbDataMgr->QueryAllData(datas);
    }

    if (fdp.ConsumeBool()) {
        rdbDataMgr->BackupRdb();
    }
    // Merged APIs
    DelayedSingleton<InsightIntentRdbDataMgr>::GetInstance()->IsRetryErrCode(errCode);


    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
