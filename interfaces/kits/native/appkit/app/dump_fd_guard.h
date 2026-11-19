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

#ifndef OHOS_ABILITY_RUNTIME_DUMP_FD_GUARD_H
#define OHOS_ABILITY_RUNTIME_DUMP_FD_GUARD_H

#include <cstdint>
#include <cstdio>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
// fdsan domain, reuse the hilog domain of this module (AAFwkTag::APPKIT = 0xD001317)
constexpr uint64_t DUMP_FD_SAN_TAG = static_cast<uint64_t>(AAFwkTag::APPKIT);

// The fd returned by RequestFileDescriptor is created by faultloggerd and delivered through
// SCM_RIGHTS. The receiving side never sets an owner tag, so the fd arrives unowned
// (close_tag == 0). This module takes over the ownership, therefore it claims the fd on
// construction and always closes it with the same valid tag on destruction.
// RAII is used instead of manual tag/close pairs because DumpCjHeap is wrapped by try/catch
// in main_thread.cpp and this target is built with -fexceptions, so a bare close() placed
// after the callee would be skipped once an exception unwinds the stack.
class DumpFdGuard {
public:
    explicit DumpFdGuard(int32_t fd) : fd_(fd)
    {
        if (fd_ >= 0) {
            fdsan_exchange_owner_tag(fd_, 0, DUMP_FD_SAN_TAG);
        }
    }

    ~DumpFdGuard()
    {
        CloseFd();
    }

    DumpFdGuard(const DumpFdGuard &) = delete;
    DumpFdGuard &operator=(const DumpFdGuard &) = delete;

    DumpFdGuard(DumpFdGuard &&other) noexcept : fd_(other.fd_)
    {
        other.fd_ = INVALID_FD;
    }

    DumpFdGuard &operator=(DumpFdGuard &&other) noexcept
    {
        if (this != &other) {
            CloseFd();
            fd_ = other.fd_;
            other.fd_ = INVALID_FD;
        }
        return *this;
    }

    int32_t Get() const
    {
        return fd_;
    }

private:
    void CloseFd()
    {
        if (fd_ >= 0) {
            fdsan_close_with_tag(fd_, DUMP_FD_SAN_TAG);
        }
        fd_ = INVALID_FD;
    }

    static constexpr int32_t INVALID_FD = -1;
    int32_t fd_ = INVALID_FD;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DUMP_FD_GUARD_H
