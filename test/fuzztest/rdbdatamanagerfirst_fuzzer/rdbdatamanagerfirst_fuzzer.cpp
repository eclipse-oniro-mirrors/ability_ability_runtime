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

#include "rdbdatamanagerfirst_fuzzer.h"

#include <charconv>
#include <fuzzer/FuzzedDataProvider.h>
#include "hilog_tag_wrapper.h"
#include "parser_util.h"

#define private public
#include "ability_resident_process_rdb.h"
#include "rdb_data_manager.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
const std::string KEY_BUNDLE_NAME = "KEY_BUNDLE_NAME";
const std::string KEY_KEEP_ALIVE_ENABLE = "KEEP_ALIVE_ENABLE";
const std::string KEY_KEEP_ALIVE_CONFIGURED_LIST = "KEEP_ALIVE_CONFIGURED_LIST";
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    struct AmsRdbConfig amsRdbConfig;
    amsRdbConfig.tableName = "resident_process_list";
    AmsResidentProcessRdbCallBack amsCallback(amsRdbConfig);
    std::unique_ptr<RdbDataManager> rdbMgr =
        std::make_unique<RdbDataManager>(amsRdbConfig);
    rdbMgr->Init(amsCallback);
    FuzzedDataProvider fdp(data, size);
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(KEY_BUNDLE_NAME, fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    valuesBucket.PutString(KEY_KEEP_ALIVE_ENABLE, fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    valuesBucket.PutString(KEY_KEEP_ALIVE_CONFIGURED_LIST, fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    rdbMgr->InsertData(amsRdbConfig.tableName, valuesBucket);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

