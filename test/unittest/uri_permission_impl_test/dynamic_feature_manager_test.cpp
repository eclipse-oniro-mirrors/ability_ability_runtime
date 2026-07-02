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

#include "dynamic_feature_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

// Minimal mock feature for framework tests. Does NOT include mock_dynamic_features.h
// (which pulls in mock_storage_manager_service.h whose static `isZero` member would
// collide with uri_permission_impl_test.o at link time). The framework tests only
// exercise DynamicFeatureManager state machine / RAII, not any feature interface.
class MockFeature : public IDynamicFeature {
public:
    ~MockFeature() override = default;
};

// Derived interface used to exercise DynamicFeatureScope::Get<IFace>() downcast.
class DerivedMockFeature : public MockFeature {
public:
    int value = 42;
};

// Standalone destroy function used to observe DestroyDeleter invocation.
static IDynamicFeature *g_destroyDeleterCaptured = nullptr;
static void TestDestroyFeature(IDynamicFeature *p)
{
    g_destroyDeleterCaptured = p;
}

// Unit tests for DynamicFeatureManager itself (framework AC-3/5/6/8).
// Uses #define private public to access registry_ and inject mock instances,
// bypassing dlopen (no real plugin .so needed). Each case clears the singleton
// registry in SetUp/TearDown for isolation; TearDown also cancels any pending
// idle-unload ffrt task so it cannot fire into the next case's entries.
class DynamicFeatureManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        DynamicFeatureManager::GetInstance().registry_.clear();
    }
    void TearDown() override
    {
        auto &reg = DynamicFeatureManager::GetInstance().registry_;
        for (auto &[id, entry] : reg) {
            if (entry.unloadHandle.has_value()) {
                ffrt::skip(*entry.unloadHandle);
                entry.unloadHandle.reset();
            }
            entry.instance.reset(); // NoOpDestroy for injected mocks
            entry.loaded = false;
        }
        reg.clear();
    }

    // Helper: inject a mock feature instance for a feature id (bypass dlopen).
    void InjectMock(FeatureId id, IDynamicFeature *mock)
    {
        auto &entry = DynamicFeatureManager::GetInstance().registry_[id];
        entry.destroy = nullptr; // NoOpDestroy: manager never deletes injected mocks
        entry.instance.reset(mock);
        entry.loaded = true;
    }
};

/*
 * Feature: DynamicFeatureManager
 * Function: Register
 * FunctionPoints: Register sets soname in registry
 */
HWTEST_F(DynamicFeatureManagerTest, Register_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    mgr.Register(FeatureId::STORAGE, "libupms_storage_ext.z.so");
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].soname, "libupms_storage_ext.z.so");
}

/*
 * Feature: DynamicFeatureManager
 * Function: Acquire
 * FunctionPoints: Acquire on unregistered feature returns invalid scope
 */
HWTEST_F(DynamicFeatureManagerTest, Acquire_Unregistered_001, TestSize.Level1)
{
    auto scope = DynamicFeatureManager::GetInstance().Acquire(FeatureId::STORAGE);
    EXPECT_FALSE(scope.IsValid());
}

/*
 * Feature: DynamicFeatureManager
 * Function: Acquire + RAII
 * FunctionPoints: Acquire returns injected instance; scope holds activeCount; release decrements (AC-3)
 */
HWTEST_F(DynamicFeatureManagerTest, Acquire_Injected_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
    {
        auto scope = mgr.Acquire(FeatureId::STORAGE);
        EXPECT_TRUE(scope.IsValid());
        EXPECT_NE(scope.Get<IDynamicFeature>(), nullptr);
        EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 1);
    }
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
}

/*
 * Feature: DynamicFeatureManager
 * Function: Acquire
 * FunctionPoints: activeCount accumulates across scopes; all release -> 0 (AC-5)
 */
HWTEST_F(DynamicFeatureManagerTest, Acquire_Multiple_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    {
        auto s1 = mgr.Acquire(FeatureId::STORAGE);
        auto s2 = mgr.Acquire(FeatureId::STORAGE);
        EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 2);
    }
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadFeatureIfIdle
 * FunctionPoints: idle unload resets instance and loaded flag (AC-5)
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadIdle_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    EXPECT_TRUE(mgr.registry_[FeatureId::STORAGE].loaded);
    mgr.UnloadFeatureIfIdle(FeatureId::STORAGE);
    EXPECT_FALSE(mgr.registry_[FeatureId::STORAGE].loaded);
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].instance.get(), nullptr);
}

/*
 * Feature: DynamicFeatureScope
 * Function: move semantics
 * FunctionPoints: moved-from scope invalid; moved-to valid; activeCount unchanged (AC-3)
 */
HWTEST_F(DynamicFeatureManagerTest, Scope_Move_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto s1 = mgr.Acquire(FeatureId::STORAGE);
    EXPECT_TRUE(s1.IsValid());
    auto s2 = std::move(s1);
    EXPECT_TRUE(s2.IsValid());
    EXPECT_FALSE(s1.IsValid()); // moved-from
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 1); // move does not change count
}

// ============================================================================
// DynamicFeatureManager — private method & branch coverage tests
// (Private access relies on the -Dprivate=public cflag in BUILD.gn, same as
//  the existing cases above.)
// ============================================================================

/*
 * Feature: DynamicFeatureManager
 * Function: Register
 * FunctionPoints: Register is idempotent — re-registering updates soname
 */
HWTEST_F(DynamicFeatureManagerTest, Register_Idempotent_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    mgr.Register(FeatureId::STORAGE, "liba.z.so");
    mgr.Register(FeatureId::STORAGE, "libb.z.so");
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].soname, "libb.z.so");
}

/*
 * Feature: DynamicFeatureManager
 * Function: Register
 * FunctionPoints: Register creates distinct entries per FeatureId
 */
HWTEST_F(DynamicFeatureManagerTest, Register_Multiple_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    mgr.Register(FeatureId::MEDIA, "libmedia.z.so");
    mgr.Register(FeatureId::STORAGE, "libstorage.z.so");
    EXPECT_EQ(mgr.registry_[FeatureId::MEDIA].soname, "libmedia.z.so");
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].soname, "libstorage.z.so");
    EXPECT_EQ(mgr.registry_.size(), 2u);
}

/*
 * Feature: DynamicFeatureManager
 * Function: LoadLocked (private)
 * FunctionPoints: already-loaded entry returns true without re-dlopen
 */
HWTEST_F(DynamicFeatureManagerTest, LoadLocked_AlreadyLoaded_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.soname = "libtest.z.so";
    entry.loaded = true;
    EXPECT_TRUE(mgr.LoadLocked(entry));
    EXPECT_TRUE(entry.loaded);
    EXPECT_EQ(entry.handle, nullptr); // no dlopen happened
}

/*
 * Feature: DynamicFeatureManager
 * Function: LoadLocked (private)
 * FunctionPoints: empty soname returns false early (no dlopen attempt)
 */
HWTEST_F(DynamicFeatureManagerTest, LoadLocked_EmptySoname_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.soname = "";
    entry.loaded = false;
    EXPECT_FALSE(mgr.LoadLocked(entry));
    EXPECT_FALSE(entry.loaded);
    EXPECT_EQ(entry.handle, nullptr);
}

/*
 * Feature: DynamicFeatureManager
 * Function: LoadLocked (private)
 * FunctionPoints: dlopen of a non-existent .so fails and returns false
 */
HWTEST_F(DynamicFeatureManagerTest, LoadLocked_DlopenFail_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.soname = "libnonexistent_test_feature_zzz.z.so";
    entry.loaded = false;
    EXPECT_FALSE(mgr.LoadLocked(entry));
    EXPECT_FALSE(entry.loaded);
    EXPECT_EQ(entry.handle, nullptr);
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadLocked (private)
 * FunctionPoints: unloading a not-loaded entry is a no-op
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadLocked_NotLoaded_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.loaded = false;
    entry.handle = nullptr;
    mgr.UnloadLocked(entry);
    EXPECT_FALSE(entry.loaded);
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadLocked (private)
 * FunctionPoints: unloading a loaded entry resets instance and clears all fields
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadLocked_Loaded_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.destroy = nullptr; // NoOpDestroy for injected mock
    entry.instance.reset(&mock);
    entry.loaded = true;
    mgr.UnloadLocked(entry);
    EXPECT_FALSE(entry.loaded);
    EXPECT_EQ(entry.instance.get(), nullptr);
    EXPECT_EQ(entry.handle, nullptr);
    EXPECT_EQ(entry.create, nullptr);
    EXPECT_EQ(entry.destroy, nullptr);
}

/*
 * Feature: DynamicFeatureManager
 * Function: CancelUnloadLocked (private)
 * FunctionPoints: no pending handle — no-op
 */
HWTEST_F(DynamicFeatureManagerTest, CancelUnloadLocked_NoHandle_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.unloadHandle.reset();
    mgr.CancelUnloadLocked(entry);
    EXPECT_FALSE(entry.unloadHandle.has_value());
}

/*
 * Feature: DynamicFeatureManager
 * Function: ArmUnloadLocked / CancelUnloadLocked (private)
 * FunctionPoints: Arm creates a handle; Cancel clears it
 */
HWTEST_F(DynamicFeatureManagerTest, ArmUnloadLocked_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    EXPECT_FALSE(entry.unloadHandle.has_value());
    mgr.ArmUnloadLocked(entry, FeatureId::STORAGE);
    EXPECT_TRUE(entry.unloadHandle.has_value());
    mgr.CancelUnloadLocked(entry);
    EXPECT_FALSE(entry.unloadHandle.has_value());
}

/*
 * Feature: DynamicFeatureManager
 * Function: ArmUnloadLocked (private)
 * FunctionPoints: arming a new handle replaces (cancels) the previous one
 */
HWTEST_F(DynamicFeatureManagerTest, ArmUnloadLocked_ReplacesPrevious_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    mgr.ArmUnloadLocked(entry, FeatureId::STORAGE);
    ASSERT_TRUE(entry.unloadHandle.has_value());
    mgr.ArmUnloadLocked(entry, FeatureId::STORAGE);
    EXPECT_TRUE(entry.unloadHandle.has_value()); // new handle set, previous cancelled
}

/*
 * Feature: DynamicFeatureManager
 * Function: Acquire
 * FunctionPoints: registered but load fails returns invalid scope (null feature)
 */
HWTEST_F(DynamicFeatureManagerTest, Acquire_LoadFail_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    mgr.Register(FeatureId::STORAGE, "libnonexistent_test_feature_zzz.z.so");
    auto scope = mgr.Acquire(FeatureId::STORAGE);
    EXPECT_FALSE(scope.IsValid());
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0); // no increment on failure
}

/*
 * Feature: DynamicFeatureManager
 * Function: Acquire
 * FunctionPoints: acquiring an already-loaded feature cancels any pending idle-unload
 */
HWTEST_F(DynamicFeatureManagerTest, Acquire_CancelsPendingUnload_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    mgr.ArmUnloadLocked(entry, FeatureId::STORAGE);
    ASSERT_TRUE(entry.unloadHandle.has_value());
    {
        auto scope = mgr.Acquire(FeatureId::STORAGE);
        EXPECT_TRUE(scope.IsValid());
        EXPECT_FALSE(entry.unloadHandle.has_value()); // cancelled by Acquire
        EXPECT_EQ(entry.activeCount, 1);
    }
    EXPECT_EQ(entry.activeCount, 0);
}

/*
 * Feature: DynamicFeatureManager
 * Function: OnScopeReleased (private)
 * FunctionPoints: release for unregistered feature is a no-op
 */
HWTEST_F(DynamicFeatureManagerTest, OnScopeReleased_Unregistered_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    mgr.OnScopeReleased(FeatureId::MEDIA);
    EXPECT_EQ(mgr.registry_.count(FeatureId::MEDIA), 0u);
}

/*
 * Feature: DynamicFeatureManager
 * Function: OnScopeReleased (private)
 * FunctionPoints: last release (activeCount → 0) arms idle-unload timer
 */
HWTEST_F(DynamicFeatureManagerTest, OnScopeReleased_ArmsUnload_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.activeCount = 1;
    EXPECT_FALSE(entry.unloadHandle.has_value());
    mgr.OnScopeReleased(FeatureId::STORAGE);
    EXPECT_EQ(entry.activeCount, 0);
    EXPECT_TRUE(entry.unloadHandle.has_value()); // armed
}

/*
 * Feature: DynamicFeatureManager
 * Function: OnScopeReleased (private)
 * FunctionPoints: non-last release does NOT arm idle-unload
 */
HWTEST_F(DynamicFeatureManagerTest, OnScopeReleased_NotLast_NoUnload_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.activeCount = 2;
    mgr.OnScopeReleased(FeatureId::STORAGE);
    EXPECT_EQ(entry.activeCount, 1);
    EXPECT_FALSE(entry.unloadHandle.has_value()); // not armed
}

/*
 * Feature: DynamicFeatureManager
 * Function: OnScopeReleased (private)
 * FunctionPoints: release when activeCount already 0 does not go negative; arms unload
 */
HWTEST_F(DynamicFeatureManagerTest, OnScopeReleased_AlreadyZero_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.activeCount = 0;
    mgr.OnScopeReleased(FeatureId::STORAGE);
    EXPECT_EQ(entry.activeCount, 0); // not negative
    EXPECT_TRUE(entry.unloadHandle.has_value()); // armed because activeCount == 0
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadFeatureIfIdle
 * FunctionPoints: unregistered feature is a no-op
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadFeatureIfIdle_Unregistered_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    mgr.UnloadFeatureIfIdle(FeatureId::MEDIA);
    EXPECT_EQ(mgr.registry_.count(FeatureId::MEDIA), 0u);
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadFeatureIfIdle
 * FunctionPoints: active feature (activeCount != 0) stays loaded
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadFeatureIfIdle_Active_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.activeCount = 1;
    mgr.UnloadFeatureIfIdle(FeatureId::STORAGE);
    EXPECT_TRUE(entry.loaded); // still loaded
    EXPECT_EQ(entry.activeCount, 1);
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadFeatureIfIdle
 * FunctionPoints: idle but not loaded just clears the handle and returns
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadFeatureIfIdle_NotLoaded_001, TestSize.Level1)
{
    auto &mgr = DynamicFeatureManager::GetInstance();
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.loaded = false;
    entry.activeCount = 0;
    entry.unloadHandle.reset();
    mgr.UnloadFeatureIfIdle(FeatureId::STORAGE);
    EXPECT_FALSE(entry.unloadHandle.has_value());
    EXPECT_FALSE(entry.loaded);
}

/*
 * Feature: DynamicFeatureManager
 * Function: UnloadFeatureIfIdle
 * FunctionPoints: idle and loaded unloads the feature and clears the handle
 */
HWTEST_F(DynamicFeatureManagerTest, UnloadFeatureIfIdle_LoadedIdle_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.activeCount = 0;
    EXPECT_TRUE(entry.loaded);
    mgr.UnloadFeatureIfIdle(FeatureId::STORAGE);
    EXPECT_FALSE(entry.loaded);
    EXPECT_EQ(entry.instance.get(), nullptr);
    EXPECT_FALSE(entry.unloadHandle.has_value());
}

/*
 * Feature: DynamicFeatureManager
 * Function: DestroyDeleter (private nested struct)
 * FunctionPoints: null fn is a no-op — does not call any destroy or delete p
 */
HWTEST_F(DynamicFeatureManagerTest, DestroyDeleter_NullFn_001, TestSize.Level1)
{
    static MockFeature mock;
    DynamicFeatureManager::DestroyDeleter deleter; // fn = nullptr
    deleter(&mock);   // no-op: mock not freed
    deleter(nullptr); // no-op
    SUCCEED();
}

/*
 * Feature: DynamicFeatureManager
 * Function: DestroyDeleter (private nested struct)
 * FunctionPoints: non-null fn with null p is a no-op
 */
HWTEST_F(DynamicFeatureManagerTest, DestroyDeleter_NullP_001, TestSize.Level1)
{
    g_destroyDeleterCaptured = nullptr;
    DynamicFeatureManager::DestroyDeleter deleter;
    deleter.fn = TestDestroyFeature;
    deleter(nullptr);
    EXPECT_EQ(g_destroyDeleterCaptured, nullptr); // fn not called
}

/*
 * Feature: DynamicFeatureManager
 * Function: DestroyDeleter (private nested struct)
 * FunctionPoints: non-null fn with non-null p invokes fn(p)
 */
HWTEST_F(DynamicFeatureManagerTest, DestroyDeleter_InvokesFn_001, TestSize.Level1)
{
    static MockFeature mock;
    g_destroyDeleterCaptured = nullptr;
    DynamicFeatureManager::DestroyDeleter deleter;
    deleter.fn = TestDestroyFeature;
    deleter(&mock);
    EXPECT_EQ(g_destroyDeleterCaptured, &mock);
}

/*
 * Feature: DynamicFeatureManager
 * Function: Destructor (private)
 * FunctionPoints: destructor cancels pending tasks and unloads all loaded entries
 */
HWTEST_F(DynamicFeatureManagerTest, Destructor_LoadedAndPending_001, TestSize.Level1)
{
    static MockFeature mock;
    // Use a local (non-singleton) instance — the default constructor is
    // accessible via -Dprivate=public; the destructor runs at scope exit.
    {
        DynamicFeatureManager localMgr;
        auto &entry = localMgr.registry_[FeatureId::STORAGE];
        entry.destroy = nullptr; // NoOpDestroy for injected mock
        entry.instance.reset(&mock);
        entry.loaded = true;
        localMgr.ArmUnloadLocked(entry, FeatureId::STORAGE);
        ASSERT_TRUE(entry.unloadHandle.has_value());
        // ~DynamicFeatureManager runs here: cancels task, unloads entry
    }
    SUCCEED(); // did not crash
}

/*
 * Feature: DynamicFeatureManager
 * Function: Destructor (private)
 * FunctionPoints: destructor skips not-loaded entries
 */
HWTEST_F(DynamicFeatureManagerTest, Destructor_NotLoaded_001, TestSize.Level1)
{
    {
        DynamicFeatureManager localMgr;
        auto &entry = localMgr.registry_[FeatureId::MEDIA];
        entry.soname = "libtest.z.so";
        entry.loaded = false;
        // ~DynamicFeatureManager: !loaded → skip UnloadLocked
    }
    SUCCEED();
}

// ============================================================================
// DynamicFeatureScope — separate fixture for the RAII guard itself
// ============================================================================

class DynamicFeatureScopeTest : public testing::Test {
public:
    void SetUp() override
    {
        DynamicFeatureManager::GetInstance().registry_.clear();
    }
    void TearDown() override
    {
        auto &reg = DynamicFeatureManager::GetInstance().registry_;
        for (auto &[id, entry] : reg) {
            if (entry.unloadHandle.has_value()) {
                ffrt::skip(*entry.unloadHandle);
                entry.unloadHandle.reset();
            }
            entry.instance.reset();
            entry.loaded = false;
        }
        reg.clear();
    }

    void InjectMock(FeatureId id, IDynamicFeature *mock)
    {
        auto &entry = DynamicFeatureManager::GetInstance().registry_[id];
        entry.destroy = nullptr;
        entry.instance.reset(mock);
        entry.loaded = true;
    }
};

/*
 * Feature: DynamicFeatureScope
 * Function: default constructor
 * FunctionPoints: default-constructed scope is invalid; Get returns nullptr
 */
HWTEST_F(DynamicFeatureScopeTest, DefaultConstructor_001, TestSize.Level1)
{
    DynamicFeatureScope scope;
    EXPECT_FALSE(scope.IsValid());
    EXPECT_EQ(scope.Get<IDynamicFeature>(), nullptr);
}

/*
 * Feature: DynamicFeatureScope
 * Function: parameterized constructor
 * FunctionPoints: scope with non-null feature is valid; Get returns the pointer
 */
HWTEST_F(DynamicFeatureScopeTest, ParameterizedConstructor_Valid_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto &entry = mgr.registry_[FeatureId::STORAGE];
    entry.activeCount = 1; // account for the scope we construct directly
    {
        DynamicFeatureScope scope(FeatureId::STORAGE, &mock);
        EXPECT_TRUE(scope.IsValid());
        EXPECT_EQ(scope.Get<IDynamicFeature>(), &mock);
    }
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0); // dtor released
}

/*
 * Feature: DynamicFeatureScope
 * Function: parameterized constructor
 * FunctionPoints: scope with null feature is invalid; destructor is a no-op
 */
HWTEST_F(DynamicFeatureScopeTest, ParameterizedConstructor_NullFeature_001, TestSize.Level1)
{
    DynamicFeatureScope scope(FeatureId::STORAGE, nullptr);
    EXPECT_FALSE(scope.IsValid());
    EXPECT_EQ(scope.Get<IDynamicFeature>(), nullptr);
}

/*
 * Feature: DynamicFeatureScope
 * Function: destructor
 * FunctionPoints: destructor of a valid scope releases (decrements activeCount)
 */
HWTEST_F(DynamicFeatureScopeTest, Destructor_Releases_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    mgr.registry_[FeatureId::STORAGE].activeCount = 1;
    {
        DynamicFeatureScope scope(FeatureId::STORAGE, &mock);
    }
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
}

/*
 * Feature: DynamicFeatureScope
 * Function: destructor
 * FunctionPoints: destructor of an invalid scope is a no-op (no OnScopeReleased)
 */
HWTEST_F(DynamicFeatureScopeTest, Destructor_Invalid_NoOp_001, TestSize.Level1)
{
    {
        DynamicFeatureScope scope; // invalid
    }
    SUCCEED();
}

/*
 * Feature: DynamicFeatureScope
 * Function: move constructor
 * FunctionPoints: moved-to valid, moved-from invalid, activeCount unchanged
 */
HWTEST_F(DynamicFeatureScopeTest, MoveConstructor_Valid_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto s1 = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    auto s2 = std::move(s1);
    EXPECT_TRUE(s2.IsValid());
    EXPECT_FALSE(s1.IsValid()); // moved-from
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 1); // move does not change count
}

/*
 * Feature: DynamicFeatureScope
 * Function: move constructor
 * FunctionPoints: moving from an invalid scope yields an invalid scope
 */
HWTEST_F(DynamicFeatureScopeTest, MoveConstructor_FromInvalid_001, TestSize.Level1)
{
    DynamicFeatureScope s1; // invalid
    auto s2 = std::move(s1);
    EXPECT_FALSE(s1.IsValid());
    EXPECT_FALSE(s2.IsValid());
}

/*
 * Feature: DynamicFeatureScope
 * Function: move assignment
 * FunctionPoints: assigns feature to target; source invalidated; count unchanged
 */
HWTEST_F(DynamicFeatureScopeTest, MoveAssignment_Valid_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto s1 = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    DynamicFeatureScope s2;                    // invalid
    s2 = std::move(s1);
    EXPECT_TRUE(s2.IsValid());
    EXPECT_FALSE(s1.IsValid());
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 1);
}

/*
 * Feature: DynamicFeatureScope
 * Function: move assignment
 * FunctionPoints: move assignment releases the target's current feature first
 */
HWTEST_F(DynamicFeatureScopeTest, MoveAssignment_ReleasesCurrent_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto s1 = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    auto s2 = mgr.Acquire(FeatureId::STORAGE); // activeCount = 2
    s1 = std::move(s2);                        // Release s1 (2→1), then take s2's feature
    EXPECT_TRUE(s1.IsValid());
    EXPECT_FALSE(s2.IsValid());
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 1);
}

/*
 * Feature: DynamicFeatureScope
 * Function: move assignment
 * FunctionPoints: assigning from an invalid scope releases current and yields invalid
 */
HWTEST_F(DynamicFeatureScopeTest, MoveAssignment_FromInvalid_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto s1 = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    DynamicFeatureScope s2;                    // invalid
    s1 = std::move(s2);                        // Release s1 (1→0), s1 takes nullptr
    EXPECT_FALSE(s1.IsValid());
    EXPECT_FALSE(s2.IsValid());
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
}

/*
 * Feature: DynamicFeatureScope
 * Function: move assignment
 * FunctionPoints: self move-assignment is a no-op (this == &other guard)
 */
HWTEST_F(DynamicFeatureScopeTest, MoveAssignment_SelfAssignment_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto s1 = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    DynamicFeatureScope &ref = s1;
    s1 = std::move(ref);                       // this == &other → no-op
    EXPECT_TRUE(s1.IsValid());
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 1);
}

/*
 * Feature: DynamicFeatureScope
 * Function: move assignment
 * FunctionPoints: assigning between two invalid scopes is a no-op
 */
HWTEST_F(DynamicFeatureScopeTest, MoveAssignment_BothInvalid_001, TestSize.Level1)
{
    DynamicFeatureScope s1;
    DynamicFeatureScope s2;
    s1 = std::move(s2);
    EXPECT_FALSE(s1.IsValid());
    EXPECT_FALSE(s2.IsValid());
}

/*
 * Feature: DynamicFeatureScope
 * Function: Get (template)
 * FunctionPoints: Get returns the borrowed interface pointer
 */
HWTEST_F(DynamicFeatureScopeTest, Get_ReturnsPointer_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto scope = mgr.Acquire(FeatureId::STORAGE);
    EXPECT_EQ(scope.Get<IDynamicFeature>(), &mock);
}

/*
 * Feature: DynamicFeatureScope
 * Function: Get (template)
 * FunctionPoints: Get downcasts to a derived interface type
 */
HWTEST_F(DynamicFeatureScopeTest, Get_DerivedType_001, TestSize.Level1)
{
    static DerivedMockFeature dmock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::MEDIA, &dmock);
    auto scope = mgr.Acquire(FeatureId::MEDIA);
    auto *p = scope.Get<DerivedMockFeature>();
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->value, 42);
}

/*
 * Feature: DynamicFeatureScope
 * Function: Get (template)
 * FunctionPoints: Get on an invalid scope returns nullptr
 */
HWTEST_F(DynamicFeatureScopeTest, Get_Invalid_001, TestSize.Level1)
{
    DynamicFeatureScope scope;
    EXPECT_EQ(scope.Get<IDynamicFeature>(), nullptr);
}

/*
 * Feature: DynamicFeatureScope
 * Function: IsValid
 * FunctionPoints: IsValid reflects feature_ state; moved-from is invalid
 */
HWTEST_F(DynamicFeatureScopeTest, IsValid_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto scope = mgr.Acquire(FeatureId::STORAGE);
    EXPECT_TRUE(scope.IsValid());
    auto moved = std::move(scope);
    EXPECT_FALSE(scope.IsValid()); // moved-from
    EXPECT_TRUE(moved.IsValid());
}

/*
 * Feature: DynamicFeatureScope
 * Function: Release (private)
 * FunctionPoints: explicit Release decrements activeCount and invalidates scope
 */
HWTEST_F(DynamicFeatureScopeTest, Release_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto scope = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    EXPECT_TRUE(scope.IsValid());
    scope.Release();                              // explicit release
    EXPECT_FALSE(scope.IsValid());
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
    // destructor calls Release again — should be a no-op (feature_ already null)
}

/*
 * Feature: DynamicFeatureScope
 * Function: Release (private)
 * FunctionPoints: double release is safe — second call is a no-op
 */
HWTEST_F(DynamicFeatureScopeTest, Release_DoubleRelease_001, TestSize.Level1)
{
    static MockFeature mock;
    auto &mgr = DynamicFeatureManager::GetInstance();
    InjectMock(FeatureId::STORAGE, &mock);
    auto scope = mgr.Acquire(FeatureId::STORAGE); // activeCount = 1
    scope.Release();
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0);
    scope.Release(); // no-op: feature_ already null
    EXPECT_EQ(mgr.registry_[FeatureId::STORAGE].activeCount, 0); // still 0
}

/*
 * Feature: DynamicFeatureScope
 * Function: Release (private)
 * FunctionPoints: Release on an invalid scope is a no-op
 */
HWTEST_F(DynamicFeatureScopeTest, Release_Invalid_001, TestSize.Level1)
{
    DynamicFeatureScope scope;
    scope.Release(); // no-op: feature_ is null
    EXPECT_FALSE(scope.IsValid());
}

}  // namespace AAFwk
}  // namespace OHOS
