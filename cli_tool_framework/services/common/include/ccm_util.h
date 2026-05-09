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

#ifndef OHOS_ABILITY_RUNTIME_CCM_UTIL_H
#define OHOS_ABILITY_RUNTIME_CCM_UTIL_H

#include <mutex>

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t DEFAULT_MAX_CLI_QUANTITY = 8;

template<typename T>
class DeviceConfiguration {
public:
    bool isLoaded = false;
    T value;
};
}
class CcmUtil {
public:
    /**
     * GetInstance, get an instance of CcmUtil.
     *
     * @return An instance of CcmUtil.
     */
    static CcmUtil &GetInstance();
    ~CcmUtil() = default;

    /**
     * GetCliConcurrencyLimit, Get cli concurrency limit.
     *
     * @return Quantity.
     */
    int32_t GetCliConcurrencyLimit();

private:
    /**
     * CcmUtil, private constructor.
     *
     */
    CcmUtil() = default;

    volatile DeviceConfiguration<int32_t> maxCliQuantity_ = {false, DEFAULT_MAX_CLI_QUANTITY};
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CCM_UTIL_H
