/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "cj_want_ffi.h"
#include <gtest/gtest.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>

#include "cj_utils_ffi.h"
#include "want.h"
#include "want_params_wrapper.h"
#include "securec.h"

#define URI_SIZE 9
#define ACTION_SIZE 11
#define WANT_TYPE_SIZE 12
#define PARAMETERS_SIZE 11

using namespace testing;
using namespace testing::ext;
using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::ElementName;

class CjWantFfiTest : public testing::Test {
public:
    CjWantFfiTest()
    {}
    ~CjWantFfiTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjWantFfiTest::SetUpTestCase()
{}

void CjWantFfiTest::TearDownTestCase()
{}

void CjWantFfiTest::SetUp()
{}

void CjWantFfiTest::TearDown()
{}

/**
 * @tc.name: CjWantFfiTestFFICJWantCreateWithWantInfo_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantCreateWithWantInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantCreateWithWantInfo_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");

    WantHandle want = FFICJWantCreateWithWantInfo(params);
    EXPECT_NE(want, nullptr);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantDelete_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantDelete.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantDelete_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");
    WantHandle want = FFICJWantCreateWithWantInfo(params);
    FFICJWantDelete(want);
    EXPECT_NE(want, nullptr);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantGetWantInfo_0100
 * @tc.desc: CjWantFfiTest test for OnCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantGetWantInfo_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");
    WantHandle want = FFICJWantCreateWithWantInfo(params);

    CJWantParams* paramsResult = FFICJWantGetWantInfo(want);
    EXPECT_NE(paramsResult, nullptr);
    FFICJWantParamsDelete(paramsResult);
    FFICJWantDelete(reinterpret_cast<WantHandle>(want));
}

/**
 * @tc.name: CjWantFfiTestFFICJWantAddEntity_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantAddEntity.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantAddEntity_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");
    WantHandle want = FFICJWantCreateWithWantInfo(params);

    const char* entity = "test_entity";
    FFICJWantAddEntity(want, entity);
    EXPECT_NE(want, nullptr);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantParseUri_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantParseUri.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantParseUri_0100, TestSize.Level1)
{
    const char* uri = "test_uri";
    WantHandle want = FFICJWantParseUri(uri);
    EXPECT_EQ(want, nullptr);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantCreateWithWantInfoV2_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantCreateWithWantInfoV2.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantCreateWithWantInfoV2_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParamsV2 params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");

    params.fds.head = nullptr;
    params.fds.size = 0;

    WantHandle want = FFICJWantCreateWithWantInfoV2(params);
    EXPECT_NE(want, nullptr);
    FFICJWantDelete(want);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantCreateWithWantInfoV2_0200
 * @tc.desc: CjWantFfiTest test for FFICJWantCreateWithWantInfoV2 with fds.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantCreateWithWantInfoV2_0200, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParamsV2 params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");

    CJFdParam* fdParams = new CJFdParam[2];
    fdParams[0].key = new char[4];
    strcpy_s(fdParams[0].key, 4, "fd1");
    fdParams[0].value = 10;
    fdParams[1].key = new char[4];
    strcpy_s(fdParams[1].key, 4, "fd2");
    fdParams[1].value = 20;

    params.fds.head = fdParams;
    params.fds.size = 2;

    WantHandle want = FFICJWantCreateWithWantInfoV2(params);
    EXPECT_NE(want, nullptr);
    FFICJWantDelete(want);

    for (int i = 0; i < 2; i++) {
        delete[] fdParams[i].key;
    }
    delete[] fdParams;
}

/**
 * @tc.name: CjWantFfiTestFFICJWantGetWantInfoV2_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantGetWantInfoV2.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantGetWantInfoV2_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParamsV2 params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");

    params.fds.head = nullptr;
    params.fds.size = 0;

    WantHandle want = FFICJWantCreateWithWantInfoV2(params);

    CJWantParamsV2* paramsResult = FFICJWantGetWantInfoV2(want);
    EXPECT_NE(paramsResult, nullptr);
    FFICJWantParamsDeleteV2(paramsResult);
    FFICJWantDelete(reinterpret_cast<WantHandle>(want));
}

/**
 * @tc.name: CjWantFfiTestFFICJWantGetWantInfoV2_0200
 * @tc.desc: CjWantFfiTest test for FFICJWantGetWantInfoV2 with fds.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantGetWantInfoV2_0200, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParamsV2 params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");

    CJFdParam* fdParams = new CJFdParam[2];
    fdParams[0].key = new char[4];
    strcpy_s(fdParams[0].key, 4, "fd1");
    fdParams[0].value = 10;
    fdParams[1].key = new char[4];
    strcpy_s(fdParams[1].key, 4, "fd2");
    fdParams[1].value = 20;

    params.fds.head = fdParams;
    params.fds.size = 2;

    WantHandle want = FFICJWantCreateWithWantInfoV2(params);

    CJWantParamsV2* paramsResult = FFICJWantGetWantInfoV2(want);
    EXPECT_NE(paramsResult, nullptr);
    FFICJWantParamsDeleteV2(paramsResult);
    FFICJWantDelete(reinterpret_cast<WantHandle>(want));

    for (int i = 0; i < 2; i++) {
        delete[] fdParams[i].key;
    }
    delete[] fdParams;
}

/**
 * @tc.name: CjWantFfiTestFFICJWantParamsDeleteV2_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantParamsDeleteV2.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantParamsDeleteV2_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParamsV2 params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[URI_SIZE];
    strcpy_s(params.uri, URI_SIZE, "deviceId");

    params.action = new char[ACTION_SIZE];
    strcpy_s(params.action, ACTION_SIZE, "bundleName");

    params.wantType = new char[WANT_TYPE_SIZE];
    strcpy_s(params.wantType, WANT_TYPE_SIZE, "abilityName");

    params.parameters = new char[PARAMETERS_SIZE];
    strcpy_s(params.parameters, PARAMETERS_SIZE, "moduleName");

    params.fds.head = nullptr;
    params.fds.size = 0;

    WantHandle want = FFICJWantCreateWithWantInfoV2(params);

    CJWantParamsV2* paramsResult = FFICJWantGetWantInfoV2(want);
    EXPECT_NE(paramsResult, nullptr);
    FFICJWantParamsDeleteV2(paramsResult);
    FFICJWantDelete(reinterpret_cast<WantHandle>(want));
}