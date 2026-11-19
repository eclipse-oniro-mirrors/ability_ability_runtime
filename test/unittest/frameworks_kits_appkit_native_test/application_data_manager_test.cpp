/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <thread>
#define private public
#define protected public
#include "application_data_manager.h"
#undef private
#undef protected
#include "ierror_observer.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class ApplicationDataManagerTest : public testing::Test {
public:
    ApplicationDataManagerTest()
    {}
    ~ApplicationDataManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool Flag;
    void SetUp();
    void TearDown();
};

class MyObserver : public IErrorObserver {
    void OnUnhandledException(std::string errMsg) override;
    void OnExceptionObject(const AppExecFwk::ErrorObject &errorObj) override;
};

bool ApplicationDataManagerTest::Flag = false;

void ApplicationDataManagerTest::SetUpTestCase(void)
{}

void ApplicationDataManagerTest::TearDownTestCase(void)
{}

void ApplicationDataManagerTest::SetUp(void)
{
}

void ApplicationDataManagerTest::TearDown(void)
{}

void MyObserver::OnUnhandledException(std::string errMsg)
{
    GTEST_LOG_(INFO) << "OnUnhandledException come, errMsg is" << errMsg;
    ApplicationDataManagerTest::Flag = true;
}

void MyObserver::OnExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    GTEST_LOG_(INFO) << "OnExceptionObject come, errorObj.name is " << errorObj.name <<
                        " errorObj.message is " << errorObj.message << " errorObj.stack is " << errorObj.stack;
    EXPECT_STREQ("errorName", errorObj.name.c_str());
    EXPECT_STREQ("errorMessage", errorObj.message.c_str());
    EXPECT_STREQ("errorStack", errorObj.stack.c_str());
}

/**
 * @tc.number: ApplicationDataManager_AddErrorObservers_001
 * @tc.name: RegisterAbilityLifecycleCallbacks
 * @tc.desc: Test whether registerabilitylifecyclecallbacks and are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_AddErrorObservers_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_AddErrorObservers_001 start";

    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    ApplicationDataManager::GetInstance().NotifyUnhandledException("test");
    EXPECT_EQ(true, ApplicationDataManagerTest::Flag);
    GTEST_LOG_(INFO) << "ApplicationDataManager_AddErrorObservers_001 end";
}

/**
 * @tc.number: ApplicationDataManager_RemoveErrorObserver_001
 * @tc.name: ApplicationDataManager RemoveErrorObserver
 * @tc.desc: Test whether remove Registerabilitylifecyclecallbacks and are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_RemoveErrorObserver_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_RemoveErrorObserver_001 start";
    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    ApplicationDataManager::GetInstance().NotifyUnhandledException("test");
    EXPECT_EQ(true, ApplicationDataManagerTest::Flag);
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    GTEST_LOG_(INFO) << "ApplicationDataManager_RemoveErrorObserver_001 end";
}

/**
 * @tc.number: ApplicationDataManager_RemoveErrorObserver_002
 * @tc.name: ApplicationDataManager RemoveErrorObserver
 * @tc.desc: Test whether remove Registerabilitylifecyclecallbacks and are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_RemoveErrorObserver_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_RemoveErrorObserver_002 start";
    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    ApplicationDataManager::GetInstance().NotifyETSUnhandledException("test");
    EXPECT_EQ(true, ApplicationDataManagerTest::Flag);
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    GTEST_LOG_(INFO) << "ApplicationDataManager_RemoveErrorObserver_002 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyExceptionObject_001
 * @tc.name: ApplicationDataManager NotifyExceptionObject
 * @tc.desc: Test whether NotifyExceptionObject are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyExceptionObject_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_001 start";
    AppExecFwk::ErrorObject errorObj;
    errorObj.name = "errorName";
    errorObj.message = "errorMessage";
    errorObj.stack = "errorStack";
    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    EXPECT_NE(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    EXPECT_TRUE(ApplicationDataManager::GetInstance().NotifyExceptionObject(errorObj));
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_001 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyExceptionObject_002
 * @tc.name: ApplicationDataManager NotifyExceptionObject
 * @tc.desc: Test whether NotifyExceptionObject are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyExceptionObject_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_002 start";
    AppExecFwk::ErrorObject errorObj;
    errorObj.name = "errorName";
    errorObj.message = "errorMessage";
    errorObj.stack = "errorStack";
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    EXPECT_FALSE(ApplicationDataManager::GetInstance().NotifyExceptionObject(errorObj));
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_002 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyETSExceptionObject_001
 * @tc.name: ApplicationDataManager NotifyExceptionObject
 * @tc.desc: Test whether NotifyETSExceptionObject are called normally.
 */
HWTEST_F(ApplicationDataManagerTest,
    ApplicationDataManager_NotifyETSExceptionObject_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyETSExceptionObject_001 start";
    AppExecFwk::ErrorObject errorObj;
    errorObj.name = "errorName";
    errorObj.message = "errorMessage";
    errorObj.stack = "errorStack";
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    EXPECT_FALSE(ApplicationDataManager::GetInstance().NotifyETSExceptionObject(errorObj));
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyETSExceptionObject_001 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyETSErrorObject_001
 * @tc.name: ApplicationDataManager NotifyExceptionObject
 * @tc.desc: Test whether NotifyETSErrorObject are called normally.
 */
HWTEST_F(ApplicationDataManagerTest,
    ApplicationDataManager_NotifyETSErrorObject_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyETSErrorObject_001 start";
    AppExecFwk::ErrorObject errorObj;
    ApplicationDataManager::GetInstance().NotifyETSErrorObject(errorObj);
    errorObj.name = "errorName";
    errorObj.message = "errorMessage";
    errorObj.stack = "errorStack";
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorCallback_);
    ApplicationDataManager::GetInstance().NotifyETSErrorObject(errorObj);
    auto errorCallback = [](const AppExecFwk::ErrorObject &errorObj) {
        printf("test\n");
    };
    ApplicationDataManager::GetInstance().SetErrorHandlerCallback(errorCallback);
    EXPECT_NE(nullptr, ApplicationDataManager::GetInstance().errorCallback_);
    ApplicationDataManager::GetInstance().NotifyETSErrorObject(errorObj);
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyETSErrorObject_001 end";
}
 
/**
 * @tc.number: ApplicationDataManager_IsUncatchable_001
 * @tc.name: ApplicationDataManager IsUncatchable
 * @tc.desc: Test whether IsUncatchable are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_IsUncatchable_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_IsUncatchable_001 start";
    ApplicationDataManager::GetInstance().SetIsUncatchable(true);
    EXPECT_TRUE(ApplicationDataManager::GetInstance().GetIsUncatchable());
    ApplicationDataManager::GetInstance().SetIsUncatchable(false);
    EXPECT_TRUE(!ApplicationDataManager::GetInstance().GetIsUncatchable());
    GTEST_LOG_(INFO) << "ApplicationDataManager_IsUncatchable_001 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyLeakObject_001
 * @tc.name: ApplicationDataManager NotifyLeakObject
 * @tc.desc: Test whether NotifyLeakObject returns false when no leak observer is set.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyLeakObject_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyLeakObject_001 start";
    LeakObject testObj{
        .leakType = LeakType::PSS_MEMORY,
        .leakSize = 1024 * 1024,  // 1MB
        .detailInfo = {
            .arktsSize = 512 * 1024,
            .nativeSize = 512 * 1024,
            .ionSize = 0,
            .gpuSize = 0,
            .ashmemSize = 0,
            .otherSize = 0
        }
    };
    bool result = ApplicationDataManager::GetInstance().NotifyLeakObject(testObj);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyLeakObject_001 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyLeakObject_002
 * @tc.name: ApplicationDataManager NotifyLeakObject WithObserver
 * @tc.desc: Test whether NotifyLeakObject executes callback normally
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyLeakObject_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyLeakObject_002 start";
    LeakObject testObj{
        .leakType = LeakType::ION_MEMORY,
        .leakSize = 2048 * 1024,  // 2MB
        .detailInfo = {
            .arktsSize = 0,
            .nativeSize = 0,
            .ionSize = 2048 * 1024,
            .gpuSize = 0,
            .ashmemSize = 0,
            .otherSize = 0
        }
    };
    bool callbackExecuted = false;
    const bool expectedCallbackResult = true;
    auto testCallback = [&callbackExecuted, expectedCallbackResult](const LeakObject &obj) -> bool {
        callbackExecuted = true;
        EXPECT_EQ(obj.leakType, LeakType::ION_MEMORY);
        EXPECT_EQ(obj.leakSize, 2048 * 1024);
        EXPECT_EQ(obj.detailInfo.arktsSize, 0);
        EXPECT_EQ(obj.detailInfo.nativeSize, 0);
        EXPECT_EQ(obj.detailInfo.ionSize, 2048 * 1024);
        EXPECT_EQ(obj.detailInfo.gpuSize, 0);
        EXPECT_EQ(obj.detailInfo.ashmemSize, 0);
        EXPECT_EQ(obj.detailInfo.otherSize, 0);
        return expectedCallbackResult;
    };
    ApplicationDataManager::GetInstance().SetLeakObserver(testCallback);
    bool result = ApplicationDataManager::GetInstance().NotifyLeakObject(testObj);
    EXPECT_TRUE(callbackExecuted);
    EXPECT_EQ(result, expectedCallbackResult);
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyLeakObject_002 end";
}

/**
* @tc.number: ApplicationDataManager_RegisterResourceObserver_001
* @tc.name: ApplicationDataManager RegisterResourceObserver Normal
* @tc.desc: Test RegisterResourceObserver with valid callback and leak type
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_RegisterResourceObserver_001,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_RegisterResourceObserver_001 start";

    bool gCallbackInvoked  = false;
    AppTelemetryLeakType gReceivedLeakType = ATLT_PSS;
    std::string gReceivedRunningId;

    auto callback = [&gCallbackInvoked, &gReceivedLeakType, &gReceivedRunningId]
                    (const AppTelemetryObject& atObj) {
        gCallbackInvoked = true;
        gReceivedLeakType = atObj.atLeakType;
        gReceivedRunningId = atObj.runningId;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_PSS) |
                                   static_cast<uint64_t>(ATLT_GPU);

    bool result = ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ApplicationDataManager_RegisterResourceObserver_001 end";
}

/**
* @tc.number: ApplicationDataManager_RegisterResourceObserver_002
* @tc.name: ApplicationDataManager RegisterResourceObserver Unregister NullCallback
* @tc.desc: Test RegisterResourceObserver with null callback to unregister
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_RegisterResourceObserver_002,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_RegisterResourceObserver_002 start";

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_PSS);

    bool result = ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, nullptr);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ApplicationDataManager_RegisterResourceObserver_002 end";
}

/**
* @tc.number: ApplicationDataManager_RegisterResourceObserver_003
* @tc.name: ApplicationDataManager RegisterResourceObserver Unregister ZeroType
* @tc.desc: Test RegisterResourceObserver with zero leak type to unregister
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_RegisterResourceObserver_003,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_RegisterResourceObserver_003 start";

    bool gCallbackInvoked = false;
    auto callback = [&gCallbackInvoked](const AppTelemetryObject& atObj) {
        gCallbackInvoked = true;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = 0;

    bool result = ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ApplicationDataManager_RegisterResourceObserver_003 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_001
* @tc.name: ApplicationDataManager NotifyAppTelemetry NoCallback
* @tc.desc: Test NotifyAppTelemetry when no callback is registered
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_001,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_001 start";

    ApplicationDataManager::GetInstance().resourceOverlimitCB_ = nullptr;

    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_PSS, "");

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_001 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_002
* @tc.name: ApplicationDataManager NotifyAppTelemetry PSS
* @tc.desc: Test NotifyAppTelemetry with PSS leak type
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_002,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_002 start";

    bool gCallbackInvoked = false;
    AppTelemetryLeakType gReceivedLeakType;

    auto callback = [&gCallbackInvoked, &gReceivedLeakType]
                    (const AppTelemetryObject& atObj) {
        gCallbackInvoked = true;
        gReceivedLeakType = atObj.atLeakType;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_PSS);
    ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);

    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_PSS, "");

    EXPECT_TRUE(gCallbackInvoked);
    EXPECT_EQ(gReceivedLeakType, ATLT_PSS);

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_002 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_003
* @tc.name: ApplicationDataManager NotifyAppTelemetry GPU
* @tc.desc: Test NotifyAppTelemetry with GPU leak type
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_003,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_003 start";

    bool gCallbackInvoked = false;
    AppTelemetryLeakType gReceivedLeakType;

    auto callback = [&gCallbackInvoked, &gReceivedLeakType]
                    (const AppTelemetryObject& atObj) {
        gCallbackInvoked = true;
        gReceivedLeakType = atObj.atLeakType;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_GPU);
    ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);

    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_GPU, "");

    EXPECT_TRUE(gCallbackInvoked);
    EXPECT_EQ(gReceivedLeakType, ATLT_GPU);

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_003 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_004
* @tc.name: ApplicationDataManager NotifyAppTelemetry FD
* @tc.desc: Test NotifyAppTelemetry with FD leak type
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_004,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_004 start";

    bool gCallbackInvoked = false;
    AppTelemetryLeakType gReceivedLeakType;

    auto callback = [&gCallbackInvoked, &gReceivedLeakType]
                    (const AppTelemetryObject& atObj) {
        gCallbackInvoked = true;
        gReceivedLeakType = atObj.atLeakType;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_FD);
    ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);

    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_FD, "");

    EXPECT_TRUE(gCallbackInvoked);
    EXPECT_EQ(gReceivedLeakType, ATLT_FD);

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_004 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_005
* @tc.name: ApplicationDataManager NotifyAppTelemetry RSS_ARK_TS
* @tc.desc: Test NotifyAppTelemetry with RSS_ARK_TS leak type
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_005,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_005 start";

    bool gCallbackInvoked = false;
    AppTelemetryLeakType gReceivedLeakType;

    auto callback = [&gCallbackInvoked, &gReceivedLeakType]
                    (const AppTelemetryObject& atObj) {
        gCallbackInvoked = true;
        gReceivedLeakType = atObj.atLeakType;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_RSS_ARK_TS);
    ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);

    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_RSS_ARK_TS, "");

    EXPECT_TRUE(gCallbackInvoked);
    EXPECT_EQ(gReceivedLeakType, ATLT_RSS_ARK_TS);

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_005 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_006
* @tc.name: ApplicationDataManager NotifyAppTelemetry MultipleTypes
* @tc.desc: Test NotifyAppTelemetry with multiple leak types registered
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_006,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_006 start";

    int gCallbackCount = 0;

    auto callback = [&gCallbackCount](const AppTelemetryObject& atObj) {
        gCallbackCount++;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_PSS) |
                                   static_cast<uint64_t>(ATLT_GPU) |
                                   static_cast<uint64_t>(ATLT_FD);
    ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);

    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_PSS, "");
    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_GPU, "");
    ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_FD, "");

    EXPECT_EQ(gCallbackCount, 3);

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_006 end";
}

/**
* @tc.number: ApplicationDataManager_NotifyAppTelemetry_007
* @tc.name: ApplicationDataManager NotifyAppTelemetry Concurrent
* @tc.desc: Test NotifyAppTelemetry thread safety with concurrent calls
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyAppTelemetry_007,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_007 start";

    int gCallbackCount = 0;
    auto callback = [&gCallbackCount](const AppTelemetryObject& atObj) {
        gCallbackCount++;
    };

    RegisterResourceParams params;
    params.appTelemetryLeakType = static_cast<uint64_t>(ATLT_PSS);
    ApplicationDataManager::GetInstance().RegisterResourceObserver(
        params, callback);

    std::thread t1([]() { ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_PSS, ""); });
    std::thread t2([]() { ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_PSS, ""); });
    std::thread t3([]() { ApplicationDataManager::GetInstance().NotifyAppTelemetry(ATLT_PSS, ""); });

    t1.join();
    t2.join();
    t3.join();

    EXPECT_EQ(gCallbackCount, 3);

    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyAppTelemetry_007 end";
}

/**
* @tc.number: ApplicationDataManager_AppTelemetryLeakType_001
* @tc.name: ApplicationDataManager AppTelemetryLeakType Values
* @tc.desc: Test AppTelemetryLeakType enum values are correctly defined
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_AppTelemetryLeakType_001,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_AppTelemetryLeakType_001 start";

    EXPECT_EQ(ATLT_PSS, 1 << static_cast<int>(LeakType::PSS_MEMORY));
    EXPECT_EQ(ATLT_GPU, 1 << static_cast<int>(LeakType::GPU_MEMORY));
    EXPECT_EQ(ATLT_FD, 1 << static_cast<int>(LeakType::FD));
    EXPECT_EQ(ATLT_RSS_ARK_TS, 1 << static_cast<int>(LeakType::RSS_ARK_TS));

    GTEST_LOG_(INFO) << "ApplicationDataManager_AppTelemetryLeakType_001 end";
}

/**
* @tc.number: ApplicationDataManager_AppTelemetryObject_001
* @tc.name: ApplicationDataManager AppTelemetryObject Structure
* @tc.desc: Test AppTelemetryObject structure
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_AppTelemetryObject_001,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_AppTelemetryObject_001 start";

    AppTelemetryObject atObj;
    atObj.atLeakType = ATLT_PSS;
    atObj.runningId = "test_running_id_12345";

    EXPECT_EQ(atObj.atLeakType, ATLT_PSS);
    EXPECT_EQ(atObj.runningId, "test_running_id_12345");

    GTEST_LOG_(INFO) << "ApplicationDataManager_AppTelemetryObject_001 end";
}

/**
* @tc.number: ApplicationDataManager_ParseGpuHookSize_001
* @tc.name: ApplicationDataManager ParseGpuHookSize EmptyString
* @tc.desc: Test ParseGpuHookSize with empty string
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_ParseGpuHookSize_001,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_001 start";

    GpuHookSize result = ApplicationDataManager::GetInstance().ParseGpuHookSize("");
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_001 end";
}

/**
* @tc.number: ApplicationDataManager_ParseGpuHookSize_002
* @tc.name: ApplicationDataManager ParseGpuHookSize InvalidJson
* @tc.desc: Test ParseGpuHookSize with invalid JSON string
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_ParseGpuHookSize_002,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_002 start";

    GpuHookSize result = ApplicationDataManager::GetInstance().ParseGpuHookSize("invalid json");
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_002 end";
}

/**
* @tc.number: ApplicationDataManager_ParseGpuHookSize_003
* @tc.name: ApplicationDataManager ParseGpuHookSize ValidJson
* @tc.desc: Test ParseGpuHookSize with valid JSON string
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_ParseGpuHookSize_003,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_003 start";

    std::string jsonStr = R"({"type1": {"first": [100, 200], "second": [300, 400]}})";
    GpuHookSize result = ApplicationDataManager::GetInstance().ParseGpuHookSize(jsonStr);

    EXPECT_EQ(result.size(), 1);
    EXPECT_NE(result.find("type1"), result.end());
    EXPECT_EQ(result["type1"].first.first, 100);
    EXPECT_EQ(result["type1"].first.second, 200);
    EXPECT_EQ(result["type1"].second.first, 300);
    EXPECT_EQ(result["type1"].second.second, 400);

    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_003 end";
}

/**
* @tc.number: ApplicationDataManager_ParseGpuHookSize_004
* @tc.name: ApplicationDataManager ParseGpuHookSize MultipleTypes
* @tc.desc: Test ParseGpuHookSize with multiple GPU types
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_ParseGpuHookSize_004,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_004 start";

    std::string jsonStr = R"({
        "type1": {"first": [100, 200], "second": [300, 400]},
        "type2": {"first": [500, 600], "second": [700, 800]},
        "type3": {"first": [900, 1000], "second": [1100, 1200]}
    })";
    GpuHookSize result = ApplicationDataManager::GetInstance().ParseGpuHookSize(jsonStr);

    EXPECT_EQ(result.size(), 3);
    EXPECT_NE(result.find("type1"), result.end());
    EXPECT_NE(result.find("type2"), result.end());
    EXPECT_NE(result.find("type3"), result.end());

    EXPECT_EQ(result["type1"].first.first, 100);
    EXPECT_EQ(result["type1"].first.second, 200);
    EXPECT_EQ(result["type2"].first.first, 500);
    EXPECT_EQ(result["type2"].first.second, 600);
    EXPECT_EQ(result["type3"].first.first, 900);
    EXPECT_EQ(result["type3"].first.second, 1000);

    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_004 end";
}

/**
* @tc.number: ApplicationDataManager_ParseGpuHookSize_005
* @tc.name: ApplicationDataManager ParseGpuHookSize MalformedJson
* @tc.desc: Test ParseGpuHookSize with malformed JSON (missing fields)
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_ParseGpuHookSize_005,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_005 start";

    std::string jsonStr = R"({"type1": {"first": [100, 200]}})";
    GpuHookSize result = ApplicationDataManager::GetInstance().ParseGpuHookSize(jsonStr);
    EXPECT_TRUE(result.empty());

    jsonStr = R"({"type1": {"first": [100], "second": [300, 400]}})";
    result = ApplicationDataManager::GetInstance().ParseGpuHookSize(jsonStr);
    EXPECT_TRUE(result.empty());

    jsonStr = R"({"type1": {"first": [100, 200], "second": [300]}})";
    result = ApplicationDataManager::GetInstance().ParseGpuHookSize(jsonStr);
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "ApplicationDataManager_ParseGpuHookSize_005 end";
}

/**
* @tc.number: ApplicationDataManager_LogGpuHookSize_001
* @tc.name: ApplicationDataManager LogGpuHookSize Empty
* @tc.desc: Test LogGpuHookSize with empty data
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_LogGpuHookSize_001,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_LogGpuHookSize_001 start";

    GpuHookSize gpuHookSize;
    ApplicationDataManager::GetInstance().LogGpuHookSize(gpuHookSize);

    GTEST_LOG_(INFO) << "ApplicationDataManager_LogGpuHookSize_001 end";
}

/**
* @tc.number: ApplicationDataManager_LogGpuHookSize_002
* @tc.name: ApplicationDataManager LogGpuHookSize ValidData
* @tc.desc: Test LogGpuHookSize with valid data
*/
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_LogGpuHookSize_002,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_LogGpuHookSize_002 start";

    GpuHookSize gpuHookSize;
    gpuHookSize["type1"] = {{100, 200}, {300, 400}};
    gpuHookSize["type2"] = {{500, 600}, {700, 800}};

    ApplicationDataManager::GetInstance().LogGpuHookSize(gpuHookSize);

    GTEST_LOG_(INFO) << "ApplicationDataManager_LogGpuHookSize_002 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
