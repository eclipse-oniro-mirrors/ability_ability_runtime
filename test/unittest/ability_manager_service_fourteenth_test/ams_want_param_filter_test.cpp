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

#include <unistd.h>

#include <gtest/gtest.h>

#include "ams_want_param_filter.h"
#include "int_wrapper.h"
#include "mock_my_status.h"
#include "skill_execute_param.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AMSWantParamFilterTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp(void)
    {
        // Default to a cross-process third-party caller (not local, not system).
        MyStatus::GetInstance().ipcGetCallingUid_ = static_cast<int>(getpid()) + 1;
        MyStatus::GetInstance().permPermission_ = 0;
        MyStatus::GetInstance().isSystemAppCall_ = false;
    }
    void TearDown(void)
    {
        // Restore the mock defaults so other test cases are not affected.
        MyStatus::GetInstance().ipcGetCallingUid_ = 1;
        MyStatus::GetInstance().permPermission_ = 1;
        MyStatus::GetInstance().isSystemAppCall_ = true;
    }
};

/**
 * @tc.number: AMSWantParamFilter_ThirdParty_StripsParam
 * @tc.name: third-party caller strips the protected param
 * @tc.desc: a cross-process third-party caller has the protected param stripped.
 */
HWTEST_F(AMSWantParamFilterTest, AMSWantParamFilter_ThirdParty_StripsParam, Function | MediumTest | Level1)
{
    WantParams params;
    params.SetParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, String::Box("forged"));
    AMSWantParamFilter::GetInstance()->OnDeserialized(params);
    EXPECT_FALSE(params.HasParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME));
}

/**
 * @tc.number: AMSWantParamFilter_ThirdParty_StripsSpecifyTokenId
 * @tc.name: third-party caller strips specifyTokenId.
 * @tc.desc: a cross-process third-party caller has specifyTokenId stripped.
 */
HWTEST_F(AMSWantParamFilterTest, AMSWantParamFilter_ThirdParty_StripsSpecifyTokenId, Function | MediumTest | Level1)
{
    WantParams params;
    params.SetParam("specifyTokenId", Integer::Box(1));
    AMSWantParamFilter::GetInstance()->OnDeserialized(params);
    EXPECT_FALSE(params.HasParam("specifyTokenId"));
}

/**
 * @tc.number: AMSWantParamFilter_ThirdParty_StripsCallerAppId
 * @tc.name: third-party caller strips callerAppId
 * @tc.desc: a cross-process third-party caller has callerAppId stripped.
 */
HWTEST_F(AMSWantParamFilterTest, AMSWantParamFilter_ThirdParty_StripsCallerAppId, Function | MediumTest | Level1)
{
    WantParams params;
    params.SetParam(Want::PARAM_RESV_CALLER_APP_ID, String::Box("forged"));
    AMSWantParamFilter::GetInstance()->OnDeserialized(params);
    EXPECT_FALSE(params.HasParam(Want::PARAM_RESV_CALLER_APP_ID));
}

/**
 * @tc.number: AMSWantParamFilter_ThirdParty_CallerAppIdentifier
 * @tc.name: third-party caller strips callerAppIdentifier
 * @tc.desc: a cross-process third-party caller has callerAppIdentifier stripped.
 */
HWTEST_F(AMSWantParamFilterTest, AMSWantParamFilter_ThirdParty_CallerAppIdentifier, Function | MediumTest | Level1)
{
    WantParams params;
    params.SetParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER, String::Box("forged"));
    AMSWantParamFilter::GetInstance()->OnDeserialized(params);
    EXPECT_FALSE(params.HasParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER));
}

/**
 * @tc.number: AMSWantParamFilter_SystemApp_KeepsParam
 * @tc.name: system app caller keeps the protected param
 * @tc.desc: a system app caller keeps the protected param untouched.
 */
HWTEST_F(AMSWantParamFilterTest, AMSWantParamFilter_SystemApp_KeepsParam, Function | MediumTest | Level1)
{
    MyStatus::GetInstance().isSystemAppCall_ = true;
    WantParams params;
    params.SetParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, String::Box("forged"));
    AMSWantParamFilter::GetInstance()->OnDeserialized(params);
    EXPECT_TRUE(params.HasParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME));
}

/**
 * @tc.number: AMSWantParamFilter_LocalDeserialization_KeepsParam
 * @tc.name: local deserialization keeps the protected param
 * @tc.desc: local deserialization (no cross-process caller) keeps the protected param untouched.
 */
HWTEST_F(AMSWantParamFilterTest, AMSWantParamFilter_LocalDeserialization_KeepsParam, Function | MediumTest | Level1)
{
    MyStatus::GetInstance().ipcGetCallingUid_ = static_cast<int>(getpid());
    WantParams params;
    params.SetParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, String::Box("forged"));
    AMSWantParamFilter::GetInstance()->OnDeserialized(params);
    EXPECT_TRUE(params.HasParam(AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME));
}
} // namespace AAFwk
} // namespace OHOS
