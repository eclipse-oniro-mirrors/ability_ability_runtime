/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FD_GUARD_H
#define OHOS_ABILITY_RUNTIME_FD_GUARD_H

#include <cstdint>
#include <unistd.h>

namespace OHOS {
namespace AAFwk {
class FdGuard {
public:
    FdGuard() = default;
    explicit FdGuard(int32_t fd) : fd_(fd) {}
    ~FdGuard()
    {
        if (fd_ > -1) {
            close(fd_);
        }
    }
    FdGuard(const FdGuard &) = delete;
    FdGuard(FdGuard &&other) : fd_(other.fd_)
    {
        other.fd_ = -1;
    }
    void operator=(const FdGuard &) = delete;
    FdGuard &operator=(FdGuard &&other)
    {
        if (fd_ > -1) {
            close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
        return *this;
    }

    int32_t Get() const
    {
        return fd_;
    }

    int32_t Release()
    {
        auto ret = fd_;
        fd_ = -1;
        return ret;
    }

    void Reset()
    {
        if (fd_ > -1) {
            close(fd_);
        }
        fd_ = -1;
    }

private:
    int32_t fd_ = -1;
};
} // AAFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_FD_GUARD_H