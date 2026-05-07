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

#include "modular_object_extension.h"
#include "native_runtime.h"
#include "want_manager.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {

bool NativeRuntime::g_loadModuleResult = true;

} // namespace AbilityRuntime

namespace AAFwk {
int CWantManager::g_transformResult = 0;
} // namespace AAFwk
} // namespace OHOS

// Static flags for callbacks
static bool g_onCreateCalled = false;
static bool g_onDestroyCalled = false;
static bool g_onDisconnectCalled = false;
static OHIPCRemoteStub *g_connectStubResult = nullptr;

static void OnCreateCallback(OH_AbilityRuntime_ModObjExtensionInstanceHandle, AbilityBase_Want *)
{
    g_onCreateCalled = true;
}

static void OnDestroyCallback(OH_AbilityRuntime_ModObjExtensionInstanceHandle)
{
    g_onDestroyCalled = true;
}

static OHIPCRemoteStub *OnConnectCallback(OH_AbilityRuntime_ModObjExtensionInstanceHandle, AbilityBase_Want *)
{
    return g_connectStubResult;
}

static void OnDisconnectCallback(OH_AbilityRuntime_ModObjExtensionInstanceHandle)
{
    g_onDisconnectCalled = true;
}

class ModularObjectExtensionTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        NativeRuntime::g_loadModuleResult = true;
        CWantManager::g_transformResult = 0;
        g_onCreateCalled = false;
        g_onDestroyCalled = false;
        g_onDisconnectCalled = false;
        g_connectStubResult = nullptr;
    }
    void TearDown() override {}
};

// ==================== Create ====================

HWTEST_F(ModularObjectExtensionTest, Create_ReturnsNonNull_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Create_ReturnsNonNull_001 start";
    auto *ext = ModularObjectExtension::Create();
    EXPECT_NE(ext, nullptr);
    delete ext;
    GTEST_LOG_(INFO) << "Create_ReturnsNonNull_001 end";
}

// ==================== BuildElement ====================

HWTEST_F(ModularObjectExtensionTest, BuildElement_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildElement_Success_001 start";
    AppExecFwk::ElementName elementName("", "com.test", "entry", "MainAbility");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    bool ret = ModularObjectExtension::BuildElement(elementName, element);
    EXPECT_TRUE(ret);
    EXPECT_STREQ(element.bundleName, "com.test");
    EXPECT_STREQ(element.moduleName, "entry");
    EXPECT_STREQ(element.abilityName, "MainAbility");
    ModularObjectExtension::DestroyElement(element);
    GTEST_LOG_(INFO) << "BuildElement_Success_001 end";
}

HWTEST_F(ModularObjectExtensionTest, BuildElement_EmptyStrings_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildElement_EmptyStrings_001 start";
    AppExecFwk::ElementName elementName("", "", "", "");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    bool ret = ModularObjectExtension::BuildElement(elementName, element);
    EXPECT_TRUE(ret);
    ModularObjectExtension::DestroyElement(element);
    GTEST_LOG_(INFO) << "BuildElement_EmptyStrings_001 end";
}

// ==================== DestroyElement ====================

HWTEST_F(ModularObjectExtensionTest, DestroyElement_NullPtrs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyElement_NullPtrs_001 start";
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    ModularObjectExtension::DestroyElement(element);
    EXPECT_EQ(element.bundleName, nullptr);
    EXPECT_EQ(element.moduleName, nullptr);
    EXPECT_EQ(element.abilityName, nullptr);
    GTEST_LOG_(INFO) << "DestroyElement_NullPtrs_001 end";
}

HWTEST_F(ModularObjectExtensionTest, DestroyElement_AllocatedPtrs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyElement_AllocatedPtrs_001 start";
    AppExecFwk::ElementName elementName("", "bundle", "module", "ability");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    ModularObjectExtension::BuildElement(elementName, element);
    ModularObjectExtension::DestroyElement(element);
    EXPECT_EQ(element.bundleName, nullptr);
    EXPECT_EQ(element.moduleName, nullptr);
    EXPECT_EQ(element.abilityName, nullptr);
    GTEST_LOG_(INFO) << "DestroyElement_AllocatedPtrs_001 end";
}

// ==================== Init ====================

HWTEST_F(ModularObjectExtensionTest, Init_SetsUpInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Init_SetsUpInstance_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    EXPECT_NE(ext->moeInstance_, nullptr);
    EXPECT_NE(ext->moeContext_, nullptr);
    GTEST_LOG_(INFO) << "Init_SetsUpInstance_001 end";
}

HWTEST_F(ModularObjectExtensionTest, Init_SetsExtensionType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Init_SetsExtensionType_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    EXPECT_EQ(ext->moeInstance_->type, AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT);
    EXPECT_EQ(ext->moeContext_->type, AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT);
    GTEST_LOG_(INFO) << "Init_SetsExtensionType_001 end";
}

HWTEST_F(ModularObjectExtensionTest, Init_LoadModuleFails_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Init_LoadModuleFails_001 start";
    NativeRuntime::g_loadModuleResult = false;
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    EXPECT_NE(ext->moeInstance_, nullptr);
    GTEST_LOG_(INFO) << "Init_LoadModuleFails_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnStart_NullOnCreateFunc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStart_NullOnCreateFunc_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onCreateFunc = nullptr;
    Want want;
    ext->OnStart(want);
    EXPECT_FALSE(g_onCreateCalled);
    GTEST_LOG_(INFO) << "OnStart_NullOnCreateFunc_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnStart_WithCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStart_WithCallback_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onCreateFunc = OnCreateCallback;
    Want want;
    ext->OnStart(want);
    EXPECT_TRUE(g_onCreateCalled);
    GTEST_LOG_(INFO) << "OnStart_WithCallback_001 end";
}

// ==================== OnStop ====================

HWTEST_F(ModularObjectExtensionTest, OnStop_NullInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop_NullInstance_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    ext->OnStop();
    GTEST_LOG_(INFO) << "OnStop_NullInstance_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnStop_NullOnDestroyFunc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop_NullOnDestroyFunc_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onDestroyFunc = nullptr;
    ext->OnStop();
    EXPECT_FALSE(g_onDestroyCalled);
    GTEST_LOG_(INFO) << "OnStop_NullOnDestroyFunc_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnStop_WithCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop_WithCallback_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onDestroyFunc = OnDestroyCallback;
    ext->OnStop();
    EXPECT_TRUE(g_onDestroyCalled);
    GTEST_LOG_(INFO) << "OnStop_WithCallback_001 end";
}

// ==================== OnConnect ====================

HWTEST_F(ModularObjectExtensionTest, OnConnect_NullInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnect_NullInstance_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    Want want;
    auto ret = ext->OnConnect(want);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "OnConnect_NullInstance_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnConnect_NullOnConnectFunc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnect_NullOnConnectFunc_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onConnectFunc = nullptr;
    Want want;
    auto ret = ext->OnConnect(want);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "OnConnect_NullOnConnectFunc_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnConnect_NullStubReturned_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnConnect_NullStubReturned_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    g_connectStubResult = nullptr;
    ext->moeInstance_->onConnectFunc = OnConnectCallback;
    Want want;
    auto ret = ext->OnConnect(want);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "OnConnect_NullStubReturned_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnDisconnect_NullOnDisconnectFunc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnect_NullOnDisconnectFunc_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onDisconnectFunc = nullptr;
    Want want;
    ext->OnDisconnect(want);
    EXPECT_FALSE(g_onDisconnectCalled);
    GTEST_LOG_(INFO) << "OnDisconnect_NullOnDisconnectFunc_001 end";
}

HWTEST_F(ModularObjectExtensionTest, OnDisconnect_WithCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDisconnect_WithCallback_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    ext->Init(record, app, handler, token);
    ext->moeInstance_->onDisconnectFunc = OnDisconnectCallback;
    Want want;
    ext->OnDisconnect(want);
    EXPECT_TRUE(g_onDisconnectCalled);
    GTEST_LOG_(INFO) << "OnDisconnect_WithCallback_001 end";
}

// ==================== CreateAndInitContext ====================

HWTEST_F(ModularObjectExtensionTest, CreateAndInitContext_ReturnsNullptr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAndInitContext_ReturnsNullptr_001 start";
    auto ext = std::make_shared<ModularObjectExtension>();
    auto record = std::make_shared<AbilityLocalRecord>();
    auto app = std::make_shared<OHOSApplication>();
    auto handler = std::make_shared<AbilityHandler>();
    sptr<OHOS::IRemoteObject> token;
    auto ret = ext->CreateAndInitContext(record, app, handler, token);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "CreateAndInitContext_ReturnsNullptr_001 end";
}
