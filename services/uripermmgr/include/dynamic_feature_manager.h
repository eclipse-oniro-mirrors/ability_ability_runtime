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

#ifndef OHOS_AAFWK_DYNAMIC_FEATURE_MANAGER_H
#define OHOS_AAFWK_DYNAMIC_FEATURE_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "ffrt_inner.h"
#include "feature/idynamic_feature.h"

namespace OHOS {
namespace AAFwk {

class DynamicFeatureManager;

// RAII scope that keeps a feature plugin loaded and borrows its interface pointer.
//
// The borrowed pointer is valid ONLY while this scope is alive (the .so stays loaded
// because the scope holds an active reference for that feature). When the LAST scope
// for a feature releases, a per-feature idle timer is (re)armed; when it fires with no
// further Acquire, only THAT feature is dlclose'd (design ADR-7, per-feature idle).
// Callers MUST NOT retain or use the pointer beyond the scope.
class DynamicFeatureScope {
public:
    DynamicFeatureScope() = default;
    DynamicFeatureScope(FeatureId id, IDynamicFeature* feature);
    ~DynamicFeatureScope();

    DynamicFeatureScope(const DynamicFeatureScope&) = delete;
    DynamicFeatureScope& operator=(const DynamicFeatureScope&) = delete;
    DynamicFeatureScope(DynamicFeatureScope&& other) noexcept;
    DynamicFeatureScope& operator=(DynamicFeatureScope&& other) noexcept;

    // Returns the feature interface, or nullptr if the plugin failed to load.
    // IFace must derive from IDynamicFeature.
    template <typename IFace>
    IFace* Get() const
    {
        return static_cast<IFace*>(feature_);
    }

    // Valid iff a non-null feature was acquired. A default-constructed or
    // released/moved-from scope is invalid and its destructor is a no-op.
    bool IsValid() const { return feature_ != nullptr; }

private:
    void Release();

    FeatureId id_{};
    IDynamicFeature* feature_ = nullptr;  // non-owning borrow; nullptr == invalid/released
};

// Manages on-demand load (dlopen) and per-feature idle unload (dlclose) of dependency
// plugins. Thread-safe. Each feature tracks its own active scopes and idle timer; a
// plugin is dlclose'd only when its own idle timer fires with no active scope for that
// feature, so unloading one feature never affects in-flight calls of another.
class DynamicFeatureManager {
    friend class DynamicFeatureScope;

public:
    static DynamicFeatureManager& GetInstance();

    // Register a plugin .so for a feature category. Idempotent.
    void Register(FeatureId id, const std::string& soname);

    // Load on demand and borrow the interface. Increments the feature's active
    // count for the scope's lifetime and cancels any pending idle-unload for it.
    DynamicFeatureScope Acquire(FeatureId id);

private:
    DynamicFeatureManager() = default;
    ~DynamicFeatureManager();
    DynamicFeatureManager(const DynamicFeatureManager&) = delete;
    DynamicFeatureManager& operator=(const DynamicFeatureManager&) = delete;

    using CreateFn = IDynamicFeature* (*)();
    using DestroyFn = void (*)(IDynamicFeature*);

    // Custom deleter invoking the plugin's DestroyFeature (cross-DSO-safe: the host
    // never `delete`s a plugin object). Null-safe so an empty instance can be reset
    // without invoking a dangling destroy pointer.
    struct DestroyDeleter {
        DestroyFn fn = nullptr;
        void operator()(IDynamicFeature* p) const
        {
            if (fn != nullptr && p != nullptr) {
                fn(p);
            }
        }
    };

    struct Entry {
        std::string soname;
        void* handle = nullptr;
        std::unique_ptr<IDynamicFeature, DestroyDeleter> instance{nullptr, DestroyDeleter{}};
        CreateFn create = nullptr;
        DestroyFn destroy = nullptr;
        bool loaded = false;
        int activeCount = 0;      // active DynamicFeatureScope count for this feature
        std::optional<ffrt::task_handle> unloadHandle;  // per-feature idle-unload task
    };

    bool LoadLocked(Entry& entry);            // caller holds mutex_
    void UnloadLocked(Entry& entry);          // caller holds mutex_
    void OnScopeReleased(FeatureId id);       // called by DynamicFeatureScope dtor
    void UnloadFeatureIfIdle(FeatureId id);   // per-feature delayed-task body
    void ArmUnloadLocked(Entry& entry, FeatureId id);     // caller holds mutex_
    void CancelUnloadLocked(Entry& entry);               // caller holds mutex_

    std::mutex mutex_;
    std::map<FeatureId, Entry> registry_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DYNAMIC_FEATURE_MANAGER_H
