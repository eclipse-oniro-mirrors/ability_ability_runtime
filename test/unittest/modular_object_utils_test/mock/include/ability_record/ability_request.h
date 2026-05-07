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

#ifndef MOCK_ABILITY_REQUEST_H
#define MOCK_ABILITY_REQUEST_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace AppExecFwk {
struct ApplicationInfo {
    std::string appDistributionType;
    uint32_t accessTokenId = 0;
    int32_t uid = 0;
};
enum ExtensionAbilityType {
    UNSPECIFIED = 0,
    SERVICE = 1,
    MODULAR_OBJECT = 99,
};

struct AbilityInfo {
    ExtensionAbilityType extensionAbilityType = UNSPECIFIED;
    bool visible = false;
};
} // namespace AppExecFwk

namespace AAFwk {

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

class Want {
public:
    inline static const std::string PARAM_APP_CLONE_INDEX_KEY = "appCloneIndex";
    ElementName GetElement() const { return element_; }
    void SetElement(const ElementName &element) { element_ = element; }
    int32_t GetIntParam(const std::string &key, int32_t defaultValue) const
    {
        if (key == PARAM_APP_CLONE_INDEX_KEY) {
            return appCloneIndex_;
        }
        return defaultValue;
    }
    void SetAppCloneIndex(int32_t index) { appCloneIndex_ = index; }
private:
    ElementName element_;
    int32_t appCloneIndex_ = 0;
};

struct AbilityRequest {
    Want want;
    AppExecFwk::ApplicationInfo appInfo;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t uid = 0;
    int32_t userId = 0;
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_ABILITY_REQUEST_H
