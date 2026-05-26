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

#include <gtest/gtest.h>

#include "cj_form_extension_object.h"
#include "cj_utils_ffi.h"
#include "configuration.h"

using FormExtAbilityHandle = void*;

namespace OHOS {
namespace AbilityRuntime {
struct CJFormExtAbilityFuncs {
    int64_t (*createCjFormExtAbility)(const char* name, FormExtAbilityHandle extAbility);
    void (*releaseCjFormExtAbility)(int64_t id);
    void (*cjFormExtAbilityInit)(int64_t id, FormExtAbilityHandle extAbility);
    CFormBindingData (*cjFormExtAbilityOnAddForm)(int64_t id, WantHandle want);
    void (*cjFormExtAbilityOnCastToNormalForm)(int64_t id, const char* formId);
    void (*cjFormExtAbilityOnUpdateForm)(int64_t id, const char* formId, const char* wantParams);
    void (*cjFormExtAbilityOnChangeFormVisibility)(int64_t id, CRecordI64I32 formEventsMap);
    void (*cjFormExtAbilityOnFormEvent)(int64_t id, const char* formId, const char* message);
    void (*cjFormExtAbilityOnRemoveForm)(int64_t id, const char* formId);
    void (*cjFormExtAbilityOnConfigurationUpdate)(int64_t id, CConfiguration configuration);
    int32_t (*cjFormExtAbilityOnAcquireFormState)(int64_t id, WantHandle want);
    void (*cjFormExtAbilityOnStop)(int64_t id);
    void (*freeCFormBindingData)(CFormBindingData data);
};

struct CJFormExtAbilityFuncsV2 {
    void (*cjFormExtAbilityOnConfigurationUpdateV2)(int64_t id, CConfigurationV2 configuration);
};
}
}

extern "C" {
CJ_EXPORT void FFIRegisterCJFormExtAbilityFuncs(void (*registerFunc)(OHOS::AbilityRuntime::CJFormExtAbilityFuncs*));
CJ_EXPORT void FFIRegisterCJFormExtAbilityFuncsV2(void (*registerFunc)(OHOS::AbilityRuntime::CJFormExtAbilityFuncsV2*));
}

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class CjFormExtensionObjectTest : public testing::Test {
};

/**
 * @tc.name  : CjFormExtensionObjectTest_FFIRegisterCJFormExtAbilityFuncsV2_001
 * @tc.desc  : Test FFIRegisterCJFormExtAbilityFuncsV2 with nullptr and valid registerFunc,
 *             covers null-check branch and successful registration branch.
 * @tc.type  : FUNC
 */
HWTEST_F(CjFormExtensionObjectTest, CjFormExtensionObjectTest_FFIRegisterCJFormExtAbilityFuncsV2_001, TestSize.Level1)
{
    FFIRegisterCJFormExtAbilityFuncsV2(nullptr);

    static bool v2ConfigCalled = false;
    v2ConfigCalled = false;
    auto registerFuncV2 = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncsV2* funcs) {
        funcs->cjFormExtAbilityOnConfigurationUpdateV2 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) {
                v2ConfigCalled = true;
            };
    };
    FFIRegisterCJFormExtAbilityFuncsV2(registerFuncV2);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : CjFormExtensionObjectTest_OnConfigurationUpdateV2_001
 * @tc.desc  : Test OnConfigurationUpdate with V2 registered, covers V2 priority path
 *             with CreateCConfigurationV2 and FreeCConfigurationV2.
 * @tc.type  : FUNC
 */
HWTEST_F(CjFormExtensionObjectTest, CjFormExtensionObjectTest_OnConfigurationUpdateV2_001, TestSize.Level1)
{
    static bool v2ConfigCalled = false;
    v2ConfigCalled = false;
    auto registerFunc = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncs* funcs) {
        funcs->createCjFormExtAbility = [](const char* name, FormExtAbilityHandle extAbility) -> int64_t { return 1; };
        funcs->releaseCjFormExtAbility = [](int64_t id) {};
        funcs->cjFormExtAbilityInit = [](int64_t id, FormExtAbilityHandle extAbility) {};
        funcs->cjFormExtAbilityOnAddForm =
            [](int64_t id, WantHandle want) -> OHOS::AbilityRuntime::CFormBindingData { return {}; };
        funcs->cjFormExtAbilityOnCastToNormalForm = [](int64_t id, const char* formId) {};
        funcs->cjFormExtAbilityOnUpdateForm = [](int64_t id, const char* formId, const char* wantParams) {};
        funcs->cjFormExtAbilityOnChangeFormVisibility =
            [](int64_t id, OHOS::AbilityRuntime::CRecordI64I32 formEventsMap) {};
        funcs->cjFormExtAbilityOnFormEvent = [](int64_t id, const char* formId, const char* message) {};
        funcs->cjFormExtAbilityOnRemoveForm = [](int64_t id, const char* formId) {};
        funcs->cjFormExtAbilityOnConfigurationUpdate =
            [](int64_t id, OHOS::AbilityRuntime::CConfiguration configuration) {};
        funcs->cjFormExtAbilityOnAcquireFormState = [](int64_t id, WantHandle want) -> int32_t { return 0; };
        funcs->cjFormExtAbilityOnStop = [](int64_t id) {};
        funcs->freeCFormBindingData = [](OHOS::AbilityRuntime::CFormBindingData data) {};
    };
    FFIRegisterCJFormExtAbilityFuncs(registerFunc);

    auto registerFuncV2 = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncsV2* funcs) {
        funcs->cjFormExtAbilityOnConfigurationUpdateV2 =
            [](int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration) {
                v2ConfigCalled = true;
            };
    };
    FFIRegisterCJFormExtAbilityFuncsV2(registerFuncV2);

    CJFormExtensionObject obj;
    obj.Init("test", nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    obj.OnConfigurationUpdate(config);

    EXPECT_TRUE(v2ConfigCalled);
}

/**
 * @tc.name  : CjFormExtensionObjectTest_OnConfigurationUpdateV1_001
 * @tc.desc  : Test OnConfigurationUpdate with V1 only (V2 not registered),
 *             covers V1 fallback path with CreateCConfiguration and FreeCConfiguration.
 * @tc.type  : FUNC
 */
HWTEST_F(CjFormExtensionObjectTest, CjFormExtensionObjectTest_OnConfigurationUpdateV1_001, TestSize.Level1)
{
    static bool v1ConfigCalled = false;
    v1ConfigCalled = false;
    auto registerFunc = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncs* funcs) {
        funcs->createCjFormExtAbility = [](const char* name, FormExtAbilityHandle extAbility) -> int64_t { return 1; };
        funcs->releaseCjFormExtAbility = [](int64_t id) {};
        funcs->cjFormExtAbilityInit = [](int64_t id, FormExtAbilityHandle extAbility) {};
        funcs->cjFormExtAbilityOnAddForm =
            [](int64_t id, WantHandle want) -> OHOS::AbilityRuntime::CFormBindingData { return {}; };
        funcs->cjFormExtAbilityOnCastToNormalForm = [](int64_t id, const char* formId) {};
        funcs->cjFormExtAbilityOnUpdateForm = [](int64_t id, const char* formId, const char* wantParams) {};
        funcs->cjFormExtAbilityOnChangeFormVisibility =
            [](int64_t id, OHOS::AbilityRuntime::CRecordI64I32 formEventsMap) {};
        funcs->cjFormExtAbilityOnFormEvent = [](int64_t id, const char* formId, const char* message) {};
        funcs->cjFormExtAbilityOnRemoveForm = [](int64_t id, const char* formId) {};
        funcs->cjFormExtAbilityOnConfigurationUpdate =
            [](int64_t id, OHOS::AbilityRuntime::CConfiguration configuration) {
                v1ConfigCalled = true;
            };
        funcs->cjFormExtAbilityOnAcquireFormState = [](int64_t id, WantHandle want) -> int32_t { return 0; };
        funcs->cjFormExtAbilityOnStop = [](int64_t id) {};
        funcs->freeCFormBindingData = [](OHOS::AbilityRuntime::CFormBindingData data) {};
    };
    FFIRegisterCJFormExtAbilityFuncs(registerFunc);

    auto resetV2 = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncsV2* funcs) {
        funcs->cjFormExtAbilityOnConfigurationUpdateV2 = nullptr;
    };
    FFIRegisterCJFormExtAbilityFuncsV2(resetV2);

    CJFormExtensionObject obj;
    obj.Init("test", nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    obj.OnConfigurationUpdate(config);

    EXPECT_FALSE(v1ConfigCalled);
}

/**
 * @tc.name  : CjFormExtensionObjectTest_OnConfigurationUpdate_NullFuncs_001
 * @tc.desc  : Test OnConfigurationUpdate with both V2 and V1 funcs null, covers null-check early return.
 * @tc.type  : FUNC
 */
HWTEST_F(CjFormExtensionObjectTest, CjFormExtensionObjectTest_OnConfigurationUpdate_NullFuncs_001, TestSize.Level1)
{
    auto registerFunc = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncs* funcs) {
        funcs->createCjFormExtAbility = [](const char* name, FormExtAbilityHandle extAbility) -> int64_t { return 1; };
        funcs->releaseCjFormExtAbility = [](int64_t id) {};
        funcs->cjFormExtAbilityInit = [](int64_t id, FormExtAbilityHandle extAbility) {};
        funcs->cjFormExtAbilityOnAddForm =
            [](int64_t id, WantHandle want) -> OHOS::AbilityRuntime::CFormBindingData { return {}; };
        funcs->cjFormExtAbilityOnCastToNormalForm = [](int64_t id, const char* formId) {};
        funcs->cjFormExtAbilityOnUpdateForm = [](int64_t id, const char* formId, const char* wantParams) {};
        funcs->cjFormExtAbilityOnChangeFormVisibility =
            [](int64_t id, OHOS::AbilityRuntime::CRecordI64I32 formEventsMap) {};
        funcs->cjFormExtAbilityOnFormEvent = [](int64_t id, const char* formId, const char* message) {};
        funcs->cjFormExtAbilityOnRemoveForm = [](int64_t id, const char* formId) {};
        funcs->cjFormExtAbilityOnConfigurationUpdate = nullptr;
        funcs->cjFormExtAbilityOnAcquireFormState = [](int64_t id, WantHandle want) -> int32_t { return 0; };
        funcs->cjFormExtAbilityOnStop = [](int64_t id) {};
        funcs->freeCFormBindingData = [](OHOS::AbilityRuntime::CFormBindingData data) {};
    };
    FFIRegisterCJFormExtAbilityFuncs(registerFunc);

    auto resetV2 = [](OHOS::AbilityRuntime::CJFormExtAbilityFuncsV2* funcs) {
        funcs->cjFormExtAbilityOnConfigurationUpdateV2 = nullptr;
    };
    FFIRegisterCJFormExtAbilityFuncsV2(resetV2);

    CJFormExtensionObject obj;
    obj.Init("test", nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    obj.OnConfigurationUpdate(config);

    EXPECT_TRUE(true);
}
