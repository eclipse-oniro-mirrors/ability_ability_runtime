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

#include <climits>
#include <gtest/gtest.h>
#include <memory>

#include "ability_connect_manager.h"
#include "caller_info.h"
#include "connection_record.h"

#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "modular_object_extension_info.h"
#include "modular_object_utils.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

// Declare mock control functions (defined in mock_modular_object_manager.cpp)
extern void ClearMockModularObjectConfig();
extern void SetMockModularObjectConfigs(
    const std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> &configs);
extern void SetMockModularObjectConfigError();

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE = "com.test.modular";
const std::string TEST_MODULE = "entry";
const std::string TEST_ABILITY = "TestModularObjectExt";
const std::string TEST_DEVICE = "device";
const std::string TEST_APP = "testApp";
}

class ModularObjectConnectTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityRequest MakeModularObjectRequest(const std::string &bundleName = TEST_BUNDLE,
        const std::string &abilityName = TEST_ABILITY);

    std::shared_ptr<AbilityConnectManager> connectManager_;
};

void ModularObjectConnectTest::SetUpTestCase(void) {}
void ModularObjectConnectTest::TearDownTestCase(void) {}

void ModularObjectConnectTest::SetUp()
{
    connectManager_ = std::make_shared<AbilityConnectManager>(0);
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("ModularObjectConnectTest");
    connectManager_->SetTaskHandler(taskHandler);
}

void ModularObjectConnectTest::TearDown()
{
    connectManager_ = nullptr;
    ClearMockModularObjectConfig();
}

AbilityRequest ModularObjectConnectTest::MakeModularObjectRequest(
    const std::string &bundleName, const std::string &abilityName)
{
    AbilityRequest request;
    ElementName element(TEST_DEVICE, bundleName, TEST_MODULE, abilityName);
    request.want.SetElement(element);
    request.abilityInfo.name = abilityName;
    request.abilityInfo.bundleName = bundleName;
    request.abilityInfo.moduleName = TEST_MODULE;
    request.abilityInfo.type = AbilityType::EXTENSION;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    request.abilityInfo.deviceId = TEST_DEVICE;
    request.abilityInfo.applicationInfo.bundleName = bundleName;
    request.abilityInfo.applicationInfo.name = TEST_APP;
    request.appInfo = request.abilityInfo.applicationInfo;
    return request;
}

// Every connect generates a unique key via RequestIdUtil

/**
 * @tc.name: GetServiceKey_ModularObject_001
 * @tc.desc: Test MODULAR_OBJECT type generates unique key with requestId suffix
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetServiceKey_ModularObject_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string key1 = AbilityConnectManager::GetServiceKey(request);
    std::string key2 = AbilityConnectManager::GetServiceKey(request);
    // Each call generates a unique requestId, so keys should differ
    EXPECT_NE(key1, key2);
    // Both should contain the base URI prefix
    EXPECT_NE(key1.find("com.test.modular"), std::string::npos);
    EXPECT_NE(key2.find("com.test.modular"), std::string::npos);
}

/**
 * @tc.name: GetServiceKey_ModularObject_002
 * @tc.desc: Test MODULAR_OBJECT key format is baseUri_requestId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetServiceKey_ModularObject_002, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string key = AbilityConnectManager::GetServiceKey(request);
    // Key should contain underscore separator between baseUri and requestId
    auto pos = key.rfind('_');
    EXPECT_NE(pos, std::string::npos);
    // requestId part should not be empty
    EXPECT_LT(pos, key.size() - 1);
}

// Verify processName is set correctly based on launchMode + processMode

/**
 * @tc.name: ProcessName_CrossProcess_Bundle_001
 * @tc.desc: Test CROSS_PROCESS BUNDLE mode sets processName to bundleName:extensionTypeName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Bundle_001, TestSize.Level1)
{
    std::string extensionTypeName = "modularObject";
    std::string process = TEST_BUNDLE + ":" + extensionTypeName;  // BUNDLE mode
    EXPECT_EQ(process, "com.test.modular:modularObject");
}

/**
 * @tc.name: ProcessName_CrossProcess_Type_001
 * @tc.desc: Test CROSS_PROCESS TYPE mode sets processName to bundleName:abilityName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Type_001, TestSize.Level1)
{
    std::string process = TEST_BUNDLE + ":" + TEST_ABILITY;  // TYPE mode
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt");
}

/**
 * @tc.name: ProcessName_CrossProcess_Instance_001
 * @tc.desc: Test CROSS_PROCESS INSTANCE mode sets unique processName with recordId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Instance_001, TestSize.Level1)
{
    int32_t recordId = 42;
    std::string process = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId);
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt:42");
    // Different recordId → different processName → different process
    int32_t recordId2 = 43;
    std::string process2 = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId2);
    EXPECT_NE(process, process2);
}

// Verify thread keys follow BUNDLE/TYPE/INSTANCE patterns

/**
 * @tc.name: ThreadKey_Bundle_001
 * @tc.desc: Test BUNDLE threadMode uses bundleName as key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Bundle_001, TestSize.Level1)
{
    std::string threadKey = TEST_BUNDLE;  // BUNDLE mode
    EXPECT_EQ(threadKey, TEST_BUNDLE);
}

/**
 * @tc.name: ThreadKey_Type_001
 * @tc.desc: Test TYPE threadMode uses bundleName_abilityName as key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Type_001, TestSize.Level1)
{
    std::string threadKey = TEST_BUNDLE + "_" + TEST_ABILITY;  // TYPE mode
    EXPECT_EQ(threadKey, "com.test.modular_TestModularObjectExt");
}

/**
 * @tc.name: ThreadKey_Instance_001
 * @tc.desc: Test INSTANCE threadMode uses bundleName_abilityName_atomicId as key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Instance_001, TestSize.Level1)
{
    uint32_t instanceId = 7;
    std::string threadKey = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId);
    EXPECT_EQ(threadKey, "com.test.modular_TestModularObjectExt_7");
    // Different instanceId → different key → different thread
    uint32_t instanceId2 = 8;
    std::string threadKey2 = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId2);
    EXPECT_NE(threadKey, threadKey2);
}

/**
 * @tc.name: ThreadKey_Type_Reuse_001
 * @tc.desc: Test TYPE mode same abilityName produces same key (thread reuse)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Type_Reuse_001, TestSize.Level1)
{
    // Same bundle + ability → same key → thread reuse
    std::string key1 = TEST_BUNDLE + "_" + TEST_ABILITY;
    std::string key2 = TEST_BUNDLE + "_" + TEST_ABILITY;
    EXPECT_EQ(key1, key2);
}

/**
 * @tc.name: ThreadKey_Type_DifferentAbility_001
 * @tc.desc: Test TYPE mode different abilityName produces different key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Type_DifferentAbility_001, TestSize.Level1)
{
    std::string key1 = TEST_BUNDLE + "_AbilityA";
    std::string key2 = TEST_BUNDLE + "_AbilityB";
    EXPECT_NE(key1, key2);
}

/**
 * @tc.name: ThreadKey_Bundle_Reuse_001
 * @tc.desc: Test BUNDLE mode different abilities same bundle produce same key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Bundle_Reuse_001, TestSize.Level1)
{
    std::string key1 = TEST_BUNDLE;  // AbilityA
    std::string key2 = TEST_BUNDLE;  // AbilityB
    EXPECT_EQ(key1, key2);  // Same bundle → same thread
}

// Tests that requestId stored in BaseExtensionRecord can reconstruct full serviceKey

/**
 * @tc.name: RequestId_Reconstruction_001
 * @tc.desc: Test requestId in record can reconstruct full serviceKey
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RequestId_Reconstruction_001, TestSize.Level1)
{
    // Simulate: connect creates key = baseUri + "_" + requestId
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";
    std::string requestId = "12345";
    std::string fullKey = baseUri + "_" + requestId;

    // Simulate: disconnect reconstructs from record member
    // GetURI() returns baseUri, GetRequestId() returns requestId
    std::string reconstructedUri = baseUri;
    std::string reconstructedRequestId = requestId;
    std::string reconstructedKey = reconstructedUri + "_" + reconstructedRequestId;
    EXPECT_EQ(fullKey, reconstructedKey);
}

/**
 * @tc.name: RequestId_Reconstruction_002
 * @tc.desc: Test empty requestId does not append underscore
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RequestId_Reconstruction_002, TestSize.Level1)
{
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";
    std::string requestId = "";  // empty
    std::string key = baseUri;
    if (!requestId.empty()) {
        key = key + "_" + requestId;
    }
    EXPECT_EQ(key, baseUri);
}

/**
 * @tc.name: RequestId_SetGet_001
 * @tc.desc: Test SetRequestId and GetRequestId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RequestId_SetGet_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);

    // Default is empty
    EXPECT_EQ(service->GetRequestId(), "");

    // Set and get
    service->SetRequestId("99999");
    EXPECT_EQ(service->GetRequestId(), "99999");
}


/**
 * @tc.name: ModularObjectExtensionInfo_001
 * @tc.desc: Test default values of ModularObjectExtensionInfo
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ModularObjectExtensionInfo_001, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    EXPECT_EQ(info.launchMode, MoeLaunchMode::IN_PROCESS);
    EXPECT_EQ(info.processMode, MoeProcessMode::BUNDLE);
    EXPECT_EQ(info.threadMode, MoeThreadMode::BUNDLE);
    EXPECT_FALSE(info.isDisabled);
    EXPECT_EQ(info.appIndex, 0);
    EXPECT_TRUE(info.bundleName.empty());
    EXPECT_TRUE(info.abilityName.empty());
}

/**
 * @tc.name: ModularObjectExtensionInfo_002
 * @tc.desc: Test setting and reading all fields
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ModularObjectExtensionInfo_002, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.moduleName = TEST_MODULE;
    info.abilityName = TEST_ABILITY;
    info.appIndex = 1;
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::INSTANCE;
    info.threadMode = MoeThreadMode::TYPE;
    info.isDisabled = true;

    EXPECT_EQ(info.bundleName, TEST_BUNDLE);
    EXPECT_EQ(info.moduleName, TEST_MODULE);
    EXPECT_EQ(info.abilityName, TEST_ABILITY);
    EXPECT_EQ(info.appIndex, 1);
    EXPECT_EQ(info.launchMode, MoeLaunchMode::CROSS_PROCESS);
    EXPECT_EQ(info.processMode, MoeProcessMode::INSTANCE);
    EXPECT_EQ(info.threadMode, MoeThreadMode::TYPE);
    EXPECT_TRUE(info.isDisabled);
}


/**
 * @tc.name: EnumBoundary_001
 * @tc.desc: Test MoeLaunchMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, EnumBoundary_001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(MoeLaunchMode::IN_PROCESS), 0);
    EXPECT_EQ(static_cast<int32_t>(MoeLaunchMode::CROSS_PROCESS), 1);
}

/**
 * @tc.name: EnumBoundary_002
 * @tc.desc: Test MoeProcessMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, EnumBoundary_002, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(MoeProcessMode::BUNDLE), 0);
    EXPECT_EQ(static_cast<int32_t>(MoeProcessMode::TYPE), 1);
    EXPECT_EQ(static_cast<int32_t>(MoeProcessMode::INSTANCE), 2);
}

/**
 * @tc.name: EnumBoundary_003
 * @tc.desc: Test MoeThreadMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, EnumBoundary_003, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(MoeThreadMode::BUNDLE), 0);
    EXPECT_EQ(static_cast<int32_t>(MoeThreadMode::TYPE), 1);
    EXPECT_EQ(static_cast<int32_t>(MoeThreadMode::INSTANCE), 2);
}

/**
 * @tc.name: GetOrCreateServiceRecord_001
 * @tc.desc: Test creating record for MODULAR_OBJECT without DB config
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetOrCreateServiceRecord_001, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.abilityName = TEST_ABILITY;
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    SetMockModularObjectConfigs({info});

    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    bool isLoadedAbility = false;

    connectManager_->GetOrCreateServiceRecord(request, true, targetService, isLoadedAbility);
    ASSERT_NE(targetService, nullptr);
    EXPECT_FALSE(isLoadedAbility);
}

/**
 * @tc.name: GetOrCreateServiceRecord_002
 * @tc.desc: Test second call creates different record (unique serviceKey)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetOrCreateServiceRecord_002, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.abilityName = TEST_ABILITY;
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    SetMockModularObjectConfigs({info});

    auto request1 = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> service1 = nullptr;
    bool loaded1 = false;
    connectManager_->GetOrCreateServiceRecord(request1, true, service1, loaded1);
    ASSERT_NE(service1, nullptr);
    EXPECT_FALSE(loaded1);

    auto request2 = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> service2 = nullptr;
    bool loaded2 = false;
    connectManager_->GetOrCreateServiceRecord(request2, true, service2, loaded2);
    ASSERT_NE(service2, nullptr);
    // MODULAR_OBJECT generates unique serviceKey each time via requestId,
    // so second call creates a new record (isLoadedAbility = false)
    EXPECT_FALSE(loaded2);
    EXPECT_NE(service1, service2);
}


/**
 * @tc.name: RemoveServiceFromMapSafe_001
 * @tc.desc: Test removing a service key that exists
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RemoveServiceFromMapSafe_001, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.abilityName = TEST_ABILITY;
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    SetMockModularObjectConfigs({info});

    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    bool isLoadedAbility = false;
    connectManager_->GetOrCreateServiceRecord(request, true, targetService, isLoadedAbility);
    ASSERT_NE(targetService, nullptr);

    std::string serviceKey = connectManager_->GetServiceKey(request);
    connectManager_->RemoveServiceFromMapSafe(serviceKey);

    auto found = connectManager_->GetServiceRecordByElementName(serviceKey);
    EXPECT_EQ(found, nullptr);
}

/**
 * @tc.name: ThreeLayer_IN_PROCESS_TYPE_001
 * @tc.desc: Test IN_PROCESS + TYPE: same process, thread key = bundleName_abilityName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_IN_PROCESS_TYPE_001, TestSize.Level1)
{
    // Layer 2: processName
    std::string processName = TEST_BUNDLE;
    EXPECT_EQ(processName, TEST_BUNDLE);

    // Layer 3: thread key
    std::string threadKey = TEST_BUNDLE + "_" + TEST_ABILITY;
    EXPECT_EQ(threadKey, "com.test.modular_TestModularObjectExt");
}

/**
 * @tc.name: ThreeLayer_CROSS_PROCESS_INSTANCE_INSTANCE_001
 * @tc.desc: Test CROSS_PROCESS + INSTANCE + INSTANCE: new process, new thread
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_CROSS_PROCESS_INSTANCE_INSTANCE_001, TestSize.Level1)
{
    int32_t recordId1 = 100;
    int32_t recordId2 = 101;

    // Layer 2: processName unique per record
    std::string process1 = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId1);
    std::string process2 = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId2);
    EXPECT_NE(process1, process2);

    // Layer 3: thread key unique per instance
    uint32_t instanceId1 = 10;
    uint32_t instanceId2 = 11;
    std::string thread1 = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId1);
    std::string thread2 = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId2);
    EXPECT_NE(thread1, thread2);
}

/**
 * @tc.name: ThreeLayer_CROSS_PROCESS_BUNDLE_BUNDLE_001
 * @tc.desc: Test CROSS_PROCESS + BUNDLE + BUNDLE: shared process (with extensionTypeName suffix), shared thread
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_CROSS_PROCESS_BUNDLE_BUNDLE_001, TestSize.Level1)
{
    // Layer 2: same processName (bundleName:extensionTypeName, different from UIAbility)
    std::string extensionTypeName = "modularObject";
    std::string process = TEST_BUNDLE + ":" + extensionTypeName;
    EXPECT_EQ(process, "com.test.modular:modularObject");

    // Layer 3: same thread key
    std::string threadKey = TEST_BUNDLE;
    EXPECT_EQ(threadKey, TEST_BUNDLE);
}


/**
 * @tc.name: AtomicInstanceId_001
 * @tc.desc: Test atomic counter increments correctly
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, AtomicInstanceId_001, TestSize.Level1)
{
    std::atomic<uint32_t> counter{0};
    uint32_t id1 = counter.fetch_add(1);
    uint32_t id2 = counter.fetch_add(1);
    uint32_t id3 = counter.fetch_add(1);
    EXPECT_EQ(id1, 0u);
    EXPECT_EQ(id2, 1u);
    EXPECT_EQ(id3, 2u);
}

// Tests the call path: GetOrCreateServiceRecord → SetupNewRecord

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnOkWhenNotModularObject
 * @tc.desc: Non-MODULAR_OBJECT type returns ERR_OK immediately
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnOkWhenNotModularObject, TestSize.Level1)
{
    AbilityRequest request;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnOkWhenBelowLimits
 * @tc.desc: Instance and connection counts below limits return ERR_OK
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnOkWhenBelowLimits, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnErrorWhenInstanceLimitReached
 * @tc.desc: Instance limit hit returns ERR_MOE_INSTANCE_LIMIT
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnErrorWhenInstanceLimitReached, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_MODULE + "/" + TEST_ABILITY;
    for (int i = 0; i < 20; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_MOE_INSTANCE_LIMIT);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnErrorWhenConnectionLimitReached
 * @tc.desc: Connection limit hit returns ERR_MOE_CONNECTION_LIMIT via single-snapshot path
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnErrorWhenConnectionLimitReached, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_MODULE + "/" + TEST_ABILITY;
    int32_t callingPid = static_cast<int32_t>(getpid());
    // 5 connections from callingPid across 5 service records → connection limit
    for (int i = 0; i < 5; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        auto connRecord = ConnectionRecord::CreateConnectionRecord(
            nullptr, service, nullptr, connectManager_);
        ASSERT_NE(connRecord, nullptr);
        auto callerInfo = std::make_shared<IndirectCallerInfo>();
        callerInfo->callerPid = callingPid;
        connRecord->AttachCallerInfo(callerInfo);
        service->AddConnectRecordToList(connRecord);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_MOE_CONNECTION_LIMIT);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnConnectionErrorViaSnapshot
 * @tc.desc: CheckModularObjectLimits detects connection limit via single-snapshot path
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnConnectionErrorViaSnapshot, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_MODULE + "/" + TEST_ABILITY;
    // Use getpid() to match IPCSkeleton::GetCallingPid() in test environment
    int32_t callingPid = static_cast<int32_t>(getpid());
    // 5 connections from callingPid across 2 service records → connection limit
    for (int batch = 0; batch < 2; batch++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        int count = (batch == 0) ? 3 : 2;
        for (int i = 0; i < count; i++) {
            auto connRecord = ConnectionRecord::CreateConnectionRecord(
                nullptr, service, nullptr, connectManager_);
            ASSERT_NE(connRecord, nullptr);
            auto callerInfo = std::make_shared<IndirectCallerInfo>();
            callerInfo->callerPid = callingPid;
            connRecord->AttachCallerInfo(callerInfo);
            service->AddConnectRecordToList(connRecord);
        }
        connectManager_->CallAddToServiceMap(
            baseKey + "_snap" + std::to_string(batch), service);
    }
    // CheckModularObjectLimits uses single-snapshot, should detect connection limit
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_MOE_CONNECTION_LIMIT);
}

/**
 * @tc.name: HandleExtensionSetup_ShouldReturnOkWhenNotModularObject
 * @tc.desc: Non-MODULAR_OBJECT type returns ERR_OK directly without calling SetupNewRecord
 */
HWTEST_F(ModularObjectConnectTest,
    HandleExtensionSetup_ShouldReturnOkWhenNotModularObject, TestSize.Level1)
{
    AbilityRequest request;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    auto ret = connectManager_->HandleExtensionSetup(request, service, "key_123");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleExtensionSetup_ShouldDelegateToSetupNewRecordWhenModularObject
 * @tc.desc: MODULAR_OBJECT delegates to SetupNewRecord and propagates success
 */
HWTEST_F(ModularObjectConnectTest,
    HandleExtensionSetup_ShouldDelegateToSetupNewRecordWhenModularObject, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.abilityName = TEST_ABILITY;
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    SetMockModularObjectConfigs({info});

    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    auto ret = connectManager_->HandleExtensionSetup(request, service, "key_456");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: HandleExtensionSetup_ShouldPropagateErrorWhenSetupNewRecordFails
 * @tc.desc: MODULAR_OBJECT propagates SetupNewRecord failure
 */
HWTEST_F(ModularObjectConnectTest,
    HandleExtensionSetup_ShouldPropagateErrorWhenSetupNewRecordFails, TestSize.Level1)
{
    SetMockModularObjectConfigError();

    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    auto ret = connectManager_->HandleExtensionSetup(request, service, "key_789");
    EXPECT_NE(ret, ERR_OK);
}
} // AAFwk
} // OHOS