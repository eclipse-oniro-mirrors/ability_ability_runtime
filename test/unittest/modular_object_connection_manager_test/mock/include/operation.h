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

#ifndef MOCK_AAFWK_OPERATION_H
#define MOCK_AAFWK_OPERATION_H

#include <string>

namespace OHOS {
namespace AAFwk {

class Operation {
public:
    std::string GetBundleName() const { return bundleName_; }
    std::string GetModuleName() const { return moduleName_; }
    std::string GetAbilityName() const { return abilityName_; }
    void SetBundleName(const std::string &name) { bundleName_ = name; }
    void SetModuleName(const std::string &name) { moduleName_ = name; }
    void SetAbilityName(const std::string &name) { abilityName_ = name; }

    bool operator<(const Operation &other) const
    {
        if (bundleName_ < other.bundleName_) return true;
        if (bundleName_ > other.bundleName_) return false;
        if (moduleName_ < other.moduleName_) return true;
        if (moduleName_ > other.moduleName_) return false;
        return abilityName_ < other.abilityName_;
    }

private:
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_AAFWK_OPERATION_H
