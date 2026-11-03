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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H
#define MOCK_OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace OHOS {
namespace AbilityRuntime {
enum RdbResult : int32_t {
    Rdb_OK = 0,
    /* Representative database initialization failed */
    Rdb_Init_Err,
    /* Failed to parse initialization file */
    Rdb_Parse_File_Err,
    /* Parameter check failed */
    Rdb_Parameter_Err,
    /* Failed to query permission settings for resident processes */
    Rdb_Permissions_Err,
    /* Database query failed, key may not exist */
    Rdb_Search_Record_Err
};

class AmsResidentProcessRdb final {
public:
    AmsResidentProcessRdb() {}
    ~AmsResidentProcessRdb() {}

    static AmsResidentProcessRdb &GetInstance()
    {
        static AmsResidentProcessRdb instance;
        return instance;
    }

    MOCK_METHOD0(Init, int32_t());
    MOCK_METHOD2(VerifyConfigurationPermissions, int32_t(const std::string &bundleName, const std::string &callerName));
    MOCK_METHOD2(VerifySaConfigurationPermissions, int32_t(const std::string &bundleName, int32_t callerUid));
    MOCK_METHOD2(GetResidentProcessEnable, int32_t(const std::string &bundleName, bool &enable));
    MOCK_METHOD2(UpdateResidentProcessEnable, int32_t(const std::string &bundleName, bool enable));
    MOCK_METHOD1(RemoveData, int32_t(const std::string &bundleName));
    MOCK_METHOD2(GetResidentProcessRawData, int32_t(const std::string &bundleName, const std::string &callerName));
    MOCK_METHOD2(GetSaResidentProcessRawData, int32_t(const std::string &bundleName, int32_t callerUid));
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H