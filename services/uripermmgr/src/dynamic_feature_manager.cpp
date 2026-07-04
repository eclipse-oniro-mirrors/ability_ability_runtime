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

#include "dynamic_feature_manager.h"

#include <dlfcn.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int64_t UNLOAD_DELAY_TIME_US = 90000000; // 90s per-feature idle delay
constexpr const char *UNLOAD_TASK_PREFIX = "UPMSFeatureIdleUnload";
constexpr const char *CREATE_FEATURE_SYM = "CreateFeature";
constexpr const char *DESTROY_FEATURE_SYM = "DestroyFeature";
} // namespace

DynamicFeatureManager &DynamicFeatureManager::GetInstance()
{
    static DynamicFeatureManager instance;
    return instance;
}

DynamicFeatureManager::~DynamicFeatureManager()
{
    std::lock_guard<std::mutex> lk(mutex_);
    for (auto &item : registry_) {
        CancelUnloadLocked(item.second);
        if (item.second.loaded) {
            UnloadLocked(item.second);
        }
    }
}

void DynamicFeatureManager::Register(FeatureId id, const std::string &soname)
{
    std::lock_guard<std::mutex> lk(mutex_);
    registry_[id].soname = soname;
}

bool DynamicFeatureManager::LoadLocked(Entry &entry)
{
    if (entry.loaded) {
        return true;
    }
    if (entry.soname.empty()) {
        return false;
    }
    entry.handle = dlopen(entry.soname.c_str(), RTLD_NOW);
    if (entry.handle == nullptr) {
        const char *err = dlerror();
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "dlopen %{public}s failed: %{public}s",
            entry.soname.c_str(), err ? err : "unknown");
        return false;
    }
    entry.create = reinterpret_cast<CreateFn>(dlsym(entry.handle, CREATE_FEATURE_SYM));
    entry.destroy = reinterpret_cast<DestroyFn>(dlsym(entry.handle, DESTROY_FEATURE_SYM));
    if (entry.create == nullptr || entry.destroy == nullptr) {
        const char *err = dlerror();
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "dlsym factory failed: %{public}s", err ? err : "unknown");
        dlclose(entry.handle);
        entry.handle = nullptr;
        return false;
    }
    IDynamicFeature *raw = entry.create();
    if (raw == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "CreateFeature returned null: %{public}s", entry.soname.c_str());
        dlclose(entry.handle);
        entry.handle = nullptr;
        entry.create = nullptr;
        entry.destroy = nullptr;
        return false;
    }
    // Lifetime owned via the plugin's DestroyFeature (cross-DSO-safe); the deleter
    // encodes that invariant so the host never `delete`s a plugin object directly.
    entry.instance = std::unique_ptr<IDynamicFeature, DestroyDeleter>(raw, DestroyDeleter{entry.destroy});
    entry.loaded = true;
    TAG_LOGI(AAFwkLogTag::URIPERMMGR, "feature loaded: %{public}s", entry.soname.c_str());
    return true;
}

void DynamicFeatureManager::UnloadLocked(Entry &entry)
{
    if (!entry.loaded) {
        return;
    }
    entry.instance.reset();  // invokes the plugin DestroyFeature deleter (or NoOpDestroy)
    if (entry.handle != nullptr) {
        dlclose(entry.handle);
        entry.handle = nullptr;
    }
    entry.create = nullptr;
    entry.destroy = nullptr;
    entry.loaded = false;
    TAG_LOGI(AAFwkLogTag::URIPERMMGR, "feature unloaded: %{public}s", entry.soname.c_str());
}

void DynamicFeatureManager::CancelUnloadLocked(Entry &entry)
{
    if (entry.unloadHandle.has_value()) {
        ffrt::skip(*entry.unloadHandle);
        entry.unloadHandle.reset();
    }
}

void DynamicFeatureManager::ArmUnloadLocked(Entry &entry, FeatureId id)
{
    CancelUnloadLocked(entry);
    // Per-feature delayed unload. Captures `this` (singleton, lives till process exit)
    // and `id`; fires UnloadFeatureIfIdle after the idle delay, unloading ONLY this feature.
    entry.unloadHandle = ffrt::submit_h(
        [this, id]() { UnloadFeatureIfIdle(id); },
        {}, {},
        ffrt::task_attr().delay(UNLOAD_DELAY_TIME_US).name(UNLOAD_TASK_PREFIX));
}

DynamicFeatureScope DynamicFeatureManager::Acquire(FeatureId id)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = registry_.find(id);
    if (it == registry_.end()) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "Acquire: feature not registered");
        return DynamicFeatureScope();
    }
    Entry &entry = it->second;
    if (!entry.loaded && !LoadLocked(entry)) {
        return DynamicFeatureScope(id, nullptr);
    }
    entry.activeCount++;
    CancelUnloadLocked(entry); // feature is in use again — cancel its pending idle unload
    return DynamicFeatureScope(id, entry.instance.get());
}

void DynamicFeatureManager::OnScopeReleased(FeatureId id)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = registry_.find(id);
    if (it == registry_.end()) {
        return;
    }
    Entry &entry = it->second;
    if (entry.activeCount > 0) {
        entry.activeCount--;
    }
    // Last reference released: start this feature's own idle-unload countdown.
    if (entry.activeCount == 0) {
        ArmUnloadLocked(entry, id);
    }
}

void DynamicFeatureManager::UnloadFeatureIfIdle(FeatureId id)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = registry_.find(id);
    if (it == registry_.end()) {
        return;
    }
    Entry &entry = it->second;
    // Re-acquired during the delay window: keep loaded.
    if (entry.activeCount != 0) {
        return;
    }
    if (!entry.loaded) {
        entry.unloadHandle.reset();
        return;
    }
    entry.unloadHandle.reset(); // this delayed task has fired
    UnloadLocked(entry);
}

DynamicFeatureScope::DynamicFeatureScope(FeatureId id, IDynamicFeature *feature)
    : id_(id), feature_(feature)
{
}

DynamicFeatureScope::~DynamicFeatureScope()
{
    Release();
}

DynamicFeatureScope::DynamicFeatureScope(DynamicFeatureScope &&other) noexcept
    : id_(other.id_), feature_(other.feature_)
{
    other.feature_ = nullptr;
}

DynamicFeatureScope &DynamicFeatureScope::operator=(DynamicFeatureScope &&other) noexcept
{
    if (this != &other) {
        Release();
        id_ = other.id_;
        feature_ = other.feature_;
        other.feature_ = nullptr;
    }
    return *this;
}

void DynamicFeatureScope::Release()
{
    if (feature_ == nullptr) {
        return;
    }
    feature_ = nullptr; // mark released; guards against re-entry / double-release
    DynamicFeatureManager::GetInstance().OnScopeReleased(id_);
}
} // namespace AAFwk
} // namespace OHOS
