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

#ifndef MOCK_APP_EXEC_FWK_ELEMENT_NAME_H
#define MOCK_APP_EXEC_FWK_ELEMENT_NAME_H

#include <string>

namespace OHOS {
namespace AppExecFwk {

class ElementName {
public:
    ElementName() = default;
    ElementName(const std::string &deviceId, const std::string &bundleName,
        const std::string &abilityName)
        : bundleName_(bundleName), abilityName_(abilityName) {}
    ElementName(const std::string &deviceId, const std::string &bundleName,
        const std::string &moduleName, const std::string &abilityName)
        : bundleName_(bundleName), moduleName_(moduleName), abilityName_(abilityName) {}

    std::string GetBundleName() const { return bundleName_; }
    std::string GetModuleName() const { return moduleName_; }
    std::string GetAbilityName() const { return abilityName_; }
    void SetBundleName(const std::string &name) { bundleName_ = name; }
    void SetModuleName(const std::string &name) { moduleName_ = name; }
    void SetAbilityName(const std::string &name) { abilityName_ = name; }

private:
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_APP_EXEC_FWK_ELEMENT_NAME_H
