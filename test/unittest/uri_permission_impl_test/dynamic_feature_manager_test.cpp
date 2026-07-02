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

}  // namespace AAFwk
}  // namespace OHOS
