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

#include "gtest/gtest.h"
#include "hilog_tag_wrapper.h"
#include "native_ability_wrapper.h"
#include "ability_native_thread.h"

using namespace testing::ext;

constexpr int32_t UUID_BUFFER_SIZE = 37; // UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx + '\0'

class NativeAbilityWrapperTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        wrapper_ = new AbilityRuntime_NativeAbilityWrapper();
    }

    void TearDown() override
    {
        delete wrapper_;
        wrapper_ = nullptr;
    }

    AbilityRuntime_NativeAbilityWrapper* wrapper_ = nullptr;
};

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_001
 * @tc.desc: Test GetAbilityInstanceId with null wrapper pointer
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_001 begin");
    char buffer[UUID_BUFFER_SIZE] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(nullptr, buffer, UUID_BUFFER_SIZE);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_001 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_002
 * @tc.desc: Test GetAbilityInstanceId with null buffer pointer
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_002 begin");
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, nullptr, UUID_BUFFER_SIZE);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_002 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_003
 * @tc.desc: Test GetAbilityInstanceId with buffer size less than 37
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_003 begin");
    char buffer[UUID_BUFFER_SIZE] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, 36);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);

    result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, 0);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);

    result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, -1);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_003 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_004
 * @tc.desc: Test GetAbilityInstanceId with empty instanceId in wrapper
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_004 begin");
    wrapper_->instanceId = "";
    char buffer[UUID_BUFFER_SIZE] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, UUID_BUFFER_SIZE);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_004 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_005
 * @tc.desc: Test GetAbilityInstanceId with valid parameters and UUID format instanceId
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_005 begin");
    const std::string testInstanceId = "12345678-1234-1234-1234-123456789abc";
    wrapper_->instanceId = testInstanceId;
    char buffer[UUID_BUFFER_SIZE] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, UUID_BUFFER_SIZE);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(testInstanceId, std::string(buffer));
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_005 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_006
 * @tc.desc: Test GetAbilityInstanceId with buffer size exactly 37 (minimum required)
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_006 begin");
    const std::string testInstanceId = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    wrapper_->instanceId = testInstanceId;
    char buffer[UUID_BUFFER_SIZE] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, UUID_BUFFER_SIZE);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(testInstanceId, std::string(buffer));
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_006 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityInstanceId_007
 * @tc.desc: Test GetAbilityInstanceId with buffer size larger than 37
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityInstanceId_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_007 begin");
    const std::string testInstanceId = "11111111-2222-3333-4444-555555555555";
    wrapper_->instanceId = testInstanceId;
    char buffer[100] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityInstanceId(wrapper_, buffer, 100);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(testInstanceId, std::string(buffer));
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityInstanceId_007 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_001
 * @tc.desc: Test GetAbilityName with null wrapper pointer
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_001 begin");
    char buffer[100] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(nullptr, buffer, 100, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_001 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_002
 * @tc.desc: Test GetAbilityName with null writeLength pointer
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_002 begin");
    wrapper_->abilityName = "TestAbility";
    char buffer[100] = {0};
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, 100, nullptr);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_002 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_003
 * @tc.desc: Test GetAbilityName with empty abilityName in wrapper
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_003 begin");
    wrapper_->abilityName = "";
    char buffer[100] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, 100, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_ABILITY_WRAPPER_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_003 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_004
 * @tc.desc: Test GetAbilityName with null buffer (query length mode)
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_004 begin");
    const std::string testAbilityName = "MainAbility";
    wrapper_->abilityName = testAbilityName;
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, nullptr, 0, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(static_cast<int32_t>(testAbilityName.length()), writeLength);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_004 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_005
 * @tc.desc: Test GetAbilityName with bufferSize <= 0
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_005 begin");
    wrapper_->abilityName = "TestAbility";
    char buffer[100] = {0};
    int32_t writeLength = 0;

    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, 0, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);

    result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, -1, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_005 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_006
 * @tc.desc: Test GetAbilityName with bufferSize less than abilityName length
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_006 begin");
    const std::string testAbilityName = "VeryLongAbilityNameForTesting";
    wrapper_->abilityName = testAbilityName;
    char buffer[5] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, 5, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_006 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_007
 * @tc.desc: Test GetAbilityName with bufferSize equal to abilityName length + 1 (minimum required)
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_007 begin");
    const std::string testAbilityName = "ShortName";
    wrapper_->abilityName = testAbilityName;
    int32_t nameLength = static_cast<int32_t>(testAbilityName.length());
    char buffer[10] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, nameLength + 1, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(nameLength, writeLength);
    EXPECT_EQ(testAbilityName, std::string(buffer));
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_007 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_008
 * @tc.desc: Test GetAbilityName with bufferSize exactly equal to name length (no room for '\0', should fail)
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_008 begin");
    const std::string testAbilityName = "Test";
    wrapper_->abilityName = testAbilityName;
    int32_t nameLength = static_cast<int32_t>(testAbilityName.length());
    char buffer[4] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, nameLength, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_008 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_009
 * @tc.desc: Test GetAbilityName with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_009 begin");
    const std::string testAbilityName = "EntryAbility";
    wrapper_->abilityName = testAbilityName;
    char buffer[100] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, 100, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(static_cast<int32_t>(testAbilityName.length()), writeLength);
    EXPECT_EQ(testAbilityName, std::string(buffer));
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_009 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetAbilityName_010
 * @tc.desc: Test GetAbilityName with one character abilityName
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetAbilityName_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_010 begin");
    const std::string testAbilityName = "A";
    wrapper_->abilityName = testAbilityName;
    char buffer[10] = {0};
    int32_t writeLength = 0;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetAbilityName(wrapper_, buffer, 10, &writeLength);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(1, writeLength);
    EXPECT_EQ(testAbilityName, std::string(buffer));
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetAbilityName_010 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetEnv_001
 * @tc.desc: Test GetEnv with null wrapper pointer
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetEnv_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_001 begin");
    napi_env env = nullptr;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetEnv(nullptr, &env);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_001 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetEnv_002
 * @tc.desc: Test GetEnv with null env pointer
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetEnv_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_002 begin");
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetEnv(wrapper_, nullptr);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_002 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetEnv_003
 * @tc.desc: Test GetEnv with null env in wrapper
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetEnv_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_003 begin");
    wrapper_->env = nullptr;
    napi_env env = nullptr;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetEnv(wrapper_, &env);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_ABILITY_WRAPPER_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_003 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetEnv_004
 * @tc.desc: Test GetEnv with valid env in wrapper
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetEnv_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_004 begin");
    napi_env fakeEnv = reinterpret_cast<napi_env>(0x12345678);
    wrapper_->env = fakeEnv;
    napi_env resultEnv = nullptr;
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetEnv(wrapper_, &resultEnv);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_NO_ERROR, result);
    EXPECT_EQ(fakeEnv, resultEnv);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_004 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetEnv_005
 * @tc.desc: Test GetEnv with both wrapper and env param null
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetEnv_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_005 begin");
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetEnv(nullptr, nullptr);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_005 end");
}

/**
 * @tc.name: OH_AbilityRuntime_GetEnv_006
 * @tc.desc: Test GetEnv resets env value when wrapper env is null
 * @tc.type: FUNC
 */
HWTEST_F(NativeAbilityWrapperTest, OH_AbilityRuntime_GetEnv_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_006 begin");
    wrapper_->env = nullptr;
    napi_env env = reinterpret_cast<napi_env>(0x99999999);
    AbilityRuntime_ErrorCode result = OH_AbilityRuntime_GetEnv(wrapper_, &env);
    EXPECT_EQ(ABILITY_RUNTIME_ERROR_CODE_ABILITY_WRAPPER_INVALID, result);
    TAG_LOGI(AAFwkTag::TEST, "OH_AbilityRuntime_GetEnv_006 end");
}