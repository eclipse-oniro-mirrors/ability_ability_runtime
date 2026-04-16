/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#define private public

#include <gtest/gtest.h>
#include <vector>

#define private public
#include "modular_object_rdb_data_mgr.h"
#include "rdb_data_manager.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace {
const std::string KEY_ONE = "KEY_ONE";
const std::string VALUE_ONE = "VALUE_ONE";
const std::string KEY_TWO = "KEY_TWO";
const std::string VALUE_TWO = "VALUE_TWO";

class ModularObjectRdbDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ModularObjectRdbDataManagerTest::SetUpTestCase()
{}

void ModularObjectRdbDataManagerTest::TearDownTestCase()
{}

void ModularObjectRdbDataManagerTest::SetUp()
{}

void ModularObjectRdbDataManagerTest::TearDown()
{}

/**
 * @tc.number: ModularObjectRdbDataManager_0100
 * @tc.desc: Test init
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectRdbDataManagerTest, ModularObjectRdbDataManager_0100, Function | SmallTest | Level1)
{
    auto res = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->InsertData(KEY_ONE, VALUE_ONE);
    EXPECT_EQ(res, NativeRdb::E_OK);

    std::string value;
    res = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(KEY_ONE, value);
    EXPECT_EQ(res, NativeRdb::E_OK);
    EXPECT_TRUE(value == VALUE_ONE);

    res = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->UpdateData(KEY_ONE, VALUE_TWO);
    EXPECT_EQ(res, NativeRdb::E_OK);

    res = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(KEY_ONE, value);
    EXPECT_EQ(res, NativeRdb::E_OK);
    EXPECT_TRUE(value == VALUE_TWO);

    res = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->DeleteData(KEY_ONE);
    EXPECT_EQ(res, NativeRdb::E_OK);
}

/**
 * @tc.number: ModularObjectRdbDataManager_0200
 * @tc.desc: Validate the retry predicate for transient Rdb errors.
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectRdbDataManagerTest, ModularObjectRdbDataManager_0200, Function | SmallTest | Level1)
{
    auto rdbMgr = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance();
    const std::vector<int32_t> retryErrCodes = {
        NativeRdb::E_DATABASE_BUSY,
        NativeRdb::E_SQLITE_BUSY,
        NativeRdb::E_SQLITE_LOCKED,
        NativeRdb::E_SQLITE_NOMEM,
        NativeRdb::E_SQLITE_IOERR,
    };
    for (auto errCode : retryErrCodes) {
        EXPECT_TRUE(rdbMgr->IsRetryErrCode(errCode));
    }
    EXPECT_FALSE(rdbMgr->IsRetryErrCode(NativeRdb::E_OK));
}
}  // namespace
