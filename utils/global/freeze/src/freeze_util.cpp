/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "freeze_util.h"

#include "hilog_tag_wrapper.h"
#include "time_util.h"
#include "file_ex.h"

namespace OHOS::AbilityRuntime {
namespace {
constexpr int32_t MAX_ENTRY_COUNT = 20;
std::string ConcatStr(const std::list<std::string> &strList, const std::string &split)
{
    if (strList.empty()) {
        return "";
    }
    if (strList.size() == 1) {
        return strList.front();
    }

    int32_t reserveSize = 0;
    for (const auto &item : strList) {
        reserveSize += static_cast<int32_t>(split.size() + item.size());
    }
    reserveSize -= static_cast<int32_t>(split.size());
    std::string result;
    if (reserveSize > 0) {
        result.reserve(reserveSize);
    }
    result.append(strList.front());
    auto iter = strList.begin();
    for (++iter; iter != strList.end(); ++iter) {
        result.append(split).append(*iter);
    }
    return result;
}
}

FreezeUtil& FreezeUtil::GetInstance()
{
    static FreezeUtil instance;
    return instance;
}

void FreezeUtil::AddLifecycleEvent(sptr<IRemoteObject> token, const std::string &entry)
{
    auto newEntry = TimeUtil::DefaultCurrentTimeStr() + "; " + entry;
    std::lock_guard lock(mutex_);
    auto &entryList = lifecycleFlow_[token];
    entryList.emplace_back(TimeUtil::DefaultCurrentTimeStr() + "; " + entry);
    if (entryList.size() > MAX_ENTRY_COUNT) {
        entryList.pop_front();
    }
}

bool FreezeUtil::AppendLifecycleEvent(sptr<IRemoteObject> token, const std::string &entry)
{
    std::lock_guard lock(mutex_);
    auto iter = lifecycleFlow_.find(token);
    if (iter == lifecycleFlow_.end()) {
        return false;
    }
    auto &entryList = iter->second;
    entryList.emplace_back(TimeUtil::DefaultCurrentTimeStr() + "; " + entry);
    if (entryList.size() > MAX_ENTRY_COUNT) {
        entryList.pop_front();
    }
    return true;
}

bool FreezeUtil::IsFrozen(pid_t pid)
{
    std::string content;
    std::string cgroupPath = "/proc/" + std::to_string(pid) + "/cgroup";
    if (!LoadStringFromFile(cgroupPath, content)) {
        return true;
    }

    if (content.find("Frozen") != std::string::npos) {
        return true;
    }
    return false;
}

std::string FreezeUtil::GetLifecycleEvent(sptr<IRemoteObject> token)
{
    std::lock_guard lock(mutex_);
    auto search = lifecycleFlow_.find(token);
    if (search != lifecycleFlow_.end()) {
        return ConcatStr(search->second, "\n");
    }
    return "";
}

void FreezeUtil::DeleteLifecycleEvent(sptr<IRemoteObject> token)
{
    std::lock_guard lock(mutex_);
    lifecycleFlow_.erase(token);
}

void FreezeUtil::AddAppLifecycleEvent(pid_t pid, const std::string &entry)
{
    std::lock_guard lock(mutex_);
    auto &entryList = appLifeCycleFlow_[pid];
    entryList.emplace_back(TimeUtil::DefaultCurrentTimeStr() + "; " + entry);
    if (entryList.size() > MAX_ENTRY_COUNT) {
        entryList.pop_front();
    }
}

void FreezeUtil::DeleteAppLifecycleEvent(pid_t pid)
{
    std::lock_guard lock(mutex_);
    appLifeCycleFlow_.erase(pid);
}

std::string FreezeUtil::GetAppLifecycleEvent(pid_t pid)
{
    std::lock_guard lock(mutex_);
    auto search = appLifeCycleFlow_.find(pid);
    if (search != appLifeCycleFlow_.end()) {
        return ConcatStr(search->second, "\n");
    }
    return "";
}
}  // namespace OHOS::AbilityRuntime
