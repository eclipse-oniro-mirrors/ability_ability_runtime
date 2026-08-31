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
#include <gmock/gmock.h>

#include "access_token.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_lite.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi/native_api.h"
#include "token_setproc.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {

class NapiCommonWantTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NapiCommonWantTest::SetUpTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest SetUpTestCase start");
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest SetUpTestCase end");
}

void NapiCommonWantTest::TearDownTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest TearDownTestCase start");
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest TearDownTestCase end");
}

void NapiCommonWantTest::SetUp()
{
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest SetUp start");
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest SetUp end");
}

void NapiCommonWantTest::TearDown()
{
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest TearDown start");
    TAG_LOGI(AAFwkTag::TEST, "NapiCommonWantTest TearDown end");
}

/**
 * @tc.name: BlackListFilter_NotFilter_0100
 * @tc.desc: proNameNotFilter matches the property name, should not be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_NotFilter_0100, Function | MediumTest | Level1)
{
    std::string strProName = "action";
    std::string proNameNotFilter = "action";
    bool isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_FALSE(isFiltered);

    strProName = "parameters";
    proNameNotFilter = "parameters";
    isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_FALSE(isFiltered);
}

/**
 * @tc.name: BlackListFilter_WindowMode_0100
 * @tc.desc: PARAM_RESV_WINDOW_MODE is reserved and should be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_WindowMode_0100, Function | MediumTest | Level1)
{
    std::string strProName = Want::PARAM_RESV_WINDOW_MODE;
    std::string proNameNotFilter = "";
    bool isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_TRUE(isFiltered);

    proNameNotFilter = "proNameNotFilter";
    isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_TRUE(isFiltered);
}

/**
 * @tc.name: BlackListFilter_DisplayId_0100
 * @tc.desc: PARAM_RESV_DISPLAY_ID is reserved and should be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_DisplayId_0100, Function | MediumTest | Level1)
{
    std::string strProName = Want::PARAM_RESV_DISPLAY_ID;
    std::string proNameNotFilter = "";
    bool isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_TRUE(isFiltered);

    proNameNotFilter = "proNameNotFilter";
    isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_TRUE(isFiltered);
}

/**
 * @tc.name: BlackListFilter_NonReserved_0100
 * @tc.desc: A normal property name should not be filtered.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_NonReserved_0100, Function | MediumTest | Level1)
{
    std::string strProName = "normalKey";
    std::string proNameNotFilter = "";
    bool isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_FALSE(isFiltered);

    strProName = "otherKey";
    isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_FALSE(isFiltered);
}

/**
 * @tc.name: BlackListFilter_OriginStringNotFiltered_0100
 * @tc.desc: PARAM_SET_URI_WITH_ORIGIN_STRING must NOT be blacklisted so that the
 *          NAPI unwrap path can carry the pass-through flag to the server side.
 */
HWTEST_F(NapiCommonWantTest, BlackListFilter_OriginStringNotFiltered_0100, Function | MediumTest | Level1)
{
    std::string strProName = Want::PARAM_SET_URI_WITH_ORIGIN_STRING;
    std::string proNameNotFilter = "";
    bool isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_FALSE(isFiltered);

    proNameNotFilter = "proNameNotFilter";
    isFiltered = BlackListFilter(strProName, proNameNotFilter);
    EXPECT_FALSE(isFiltered);
}

/**
 * @tc.name: SetUriWithPassThroughFlag_NoFlag_0100
 * @tc.desc: No pass-through flag set, uri is set directly.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_NoFlag_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    std::string uriString = "https://www.example.com/path";
    SetUriWithPassThroughFlag(uriString, want);
    EXPECT_EQ(want.GetUriString(), uriString);

    uriString = "https://www.example.com/another";
    SetUriWithPassThroughFlag(uriString, want);
    EXPECT_EQ(want.GetUriString(), uriString);
}

/**
 * @tc.name: SetUriWithPassThroughFlag_NonSystemApp_0100
 * @tc.desc: Pass-through flag set but caller is not a system app, uri is set directly.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_NonSystemApp_0100, Function | MediumTest | Level1)
{
    uint64_t originalToken = GetSelfTokenID();
    SetSelfTokenID(0);

    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, true);
    SetUriWithPassThroughFlag("https://www.example.com/path", want);
    EXPECT_EQ(want.GetUriString(), "https://www.example.com/path");

    SetSelfTokenID(originalToken);
}

/**
 * @tc.name: SetUriWithPassThroughFlag_SystemApp_0100
 * @tc.desc: Pass-through flag set and caller is a system app, raw uri restored.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_SystemApp_0100, Function | MediumTest | Level1)
{
    uint64_t originalToken = GetSelfTokenID();

    uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_HAP;
    uint64_t fullTokenId = systemAppMask | tokenID;
    SetSelfTokenID(fullTokenId);

    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, true);
    SetUriWithPassThroughFlag("https://www.example.com/path", want);
    EXPECT_EQ(want.GetUriString(), "https://www.example.com/path");

    SetSelfTokenID(originalToken);
}

/**
 * @tc.name: SetUriWithPassThroughFlag_FlagNotSetOnWant_0100
 * @tc.desc: Flag param absent on the want, direct SetUri regardless of caller identity.
 */
HWTEST_F(NapiCommonWantTest, SetUriWithPassThroughFlag_FlagNotSetOnWant_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    std::string uriString = "https://www.example.com/path";
    bool passThroughFlag = want.GetBoolParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, false);
    EXPECT_FALSE(passThroughFlag);

    SetUriWithPassThroughFlag(uriString, want);
    EXPECT_EQ(want.GetUriString(), uriString);
}

/**
 * @tc.name: UnwrapWant_NullParam_0100
 * @tc.desc: Null param is rejected by the type guard.
 */
HWTEST_F(NapiCommonWantTest, UnwrapWant_NullParam_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    napi_env env = nullptr;
    napi_value param = nullptr;
    bool result = UnwrapWant(env, param, want);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: UnwrapWantParams_NullParam_0100
 * @tc.desc: Null param is rejected by the type guard.
 */
HWTEST_F(NapiCommonWantTest, UnwrapWantParams_NullParam_0100, Function | MediumTest | Level1)
{
    AAFwk::WantParams wantParams;
    napi_env env = nullptr;
    napi_value param = nullptr;
    bool result = UnwrapWantParams(env, param, wantParams);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: WrapWant_NonSystemApp_StripsPassThroughFlag_0100
 * @tc.desc: Non-system app is the final hop, the pass-through flag is stripped
 *          when the want is wrapped to the JS layer.
 */
HWTEST_F(NapiCommonWantTest, WrapWant_NonSystemApp_StripsPassThroughFlag_0100, Function | MediumTest | Level1)
{
    uint64_t originalToken = GetSelfTokenID();
    SetSelfTokenID(0);

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    AbilityRuntime::JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ASSERT_NE(env, nullptr);

    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, true);
    napi_value jsWant = WrapWant(env, want);
    ASSERT_NE(jsWant, nullptr);

    napi_value jsParams = GetPropertyValueByPropertyName(env, jsWant, "parameters", napi_object);
    ASSERT_NE(jsParams, nullptr);
    EXPECT_FALSE(IsExistsByPropertyName(env, jsParams,
        AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING.c_str()));

    AbilityRuntime::JsRuntimeLite::GetInstance().RemoveJsEnv(env);
    SetSelfTokenID(originalToken);
}

/**
 * @tc.name: WrapWant_SystemApp_KeepsPassThroughFlag_0100
 * @tc.desc: System app may keep forwarding, the pass-through flag is preserved
 *          when the want is wrapped to the JS layer.
 */
HWTEST_F(NapiCommonWantTest, WrapWant_SystemApp_KeepsPassThroughFlag_0100, Function | MediumTest | Level1)
{
    uint64_t originalToken = GetSelfTokenID();

    uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_HAP;
    uint64_t fullTokenId = systemAppMask | tokenID;
    SetSelfTokenID(fullTokenId);

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv = nullptr;
    AbilityRuntime::JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ASSERT_NE(env, nullptr);

    AAFwk::Want want;
    want.SetParam(AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING, true);
    napi_value jsWant = WrapWant(env, want);
    ASSERT_NE(jsWant, nullptr);

    napi_value jsParams = GetPropertyValueByPropertyName(env, jsWant, "parameters", napi_object);
    ASSERT_NE(jsParams, nullptr);
    EXPECT_TRUE(IsExistsByPropertyName(env, jsParams,
        AAFwk::Want::PARAM_SET_URI_WITH_ORIGIN_STRING.c_str()));

    AbilityRuntime::JsRuntimeLite::GetInstance().RemoveJsEnv(env);
    SetSelfTokenID(originalToken);
}
} // namespace AppExecFwk
} // namespace OHOS
