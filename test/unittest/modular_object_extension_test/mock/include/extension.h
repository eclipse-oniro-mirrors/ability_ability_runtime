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

#ifndef MOCK_EXTENSION_H
#define MOCK_EXTENSION_H

#include <memory>
#include <string>
#include "mock_types.h"
#include "refbase.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {

class AbilityLocalRecord {};
class OHOSApplication {};
class AbilityHandler {};

struct AbilityInfo {
    std::string srcEntrance;
    std::string moduleName;
    std::string bundleName;
    std::string name;
};

class Extension : public std::enable_shared_from_this<Extension> {
public:
    Extension() = default;
    virtual ~Extension() = default;

    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &,
        const std::shared_ptr<OHOSApplication> &,
        std::shared_ptr<AbilityHandler> &,
        const sptr<IRemoteObject> &) {}

    virtual void OnStart(const AAFwk::Want &want) {}
    virtual void OnStop() {}
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) { return nullptr; }
    virtual void OnDisconnect(const AAFwk::Want &want) {}

    std::shared_ptr<AbilityInfo> abilityInfo_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_EXTENSION_H
