/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_FOR_QUERY_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_FOR_QUERY_H

#include <string>
#include <vector>

#include "parcel.h"
#include "insight_intent_constant.h"
#include "insight_intent_execute_param.h"

namespace OHOS {
namespace AbilityRuntime {
struct LinkInfoForQuery {
    std::string uri;
};

struct PageInfoForQuery {
    std::string uiAbility;
    std::string pagePath;
    std::string navigationId;
    std::string navDestinationName;
};

struct EntryInfoForQuery {
    std::string abilityName;
    std::vector<AppExecFwk::ExecuteMode> executeMode;
};

struct FunctionInfoForQuery {};

struct FormInfoForQuery {
    std::string abilityName;
    std::string formName;
};

struct EntityInfoForQuery {
    std::string className;
    std::string entityId;
    std::string entityCategory;
    std::string parameters;
    std::string parentClassName;
};

struct UIAbilityIntentInfoForQuery {
    std::string abilityName;
    std::vector<AppExecFwk::ExecuteMode> supportExecuteMode {};
};

struct UIExtensionIntentInfoForQuery {
    std::string abilityName;
};

struct ServiceExtensionIntentInfoForQuery {
    std::string abilityName;
};

struct FormIntentInfoForQuery {
    std::string abilityName;
    std::string formName;
};

struct InsightIntentInfoForQuery : public Parcelable {
    bool isConfig = false;
    std::string bundleName;
    std::string develoType;
    std::string moduleName;
    std::string intentName;
    std::string srcEntry;
    std::string domain;
    std::string intentVersion;
    std::string displayName;
    std::string displayDescription;
    std::string schema;
    std::string icon;
    std::string llmDescription;
    std::string intentType;
    std::string parameters;
    std::string result;
    std::vector<std::string> keywords;
    LinkInfoForQuery linkInfo;
    PageInfoForQuery pageInfo;
    EntryInfoForQuery entryInfo;
    FunctionInfoForQuery functionInfo;
    FormInfoForQuery formInfo;
    std::vector<std::string> inputParams;
    std::vector<std::string> outputParams;
    std::string cfgEntities;
    UIAbilityIntentInfoForQuery uiAbilityIntentInfo;
    UIExtensionIntentInfoForQuery uiExtensionIntentInfo;
    ServiceExtensionIntentInfoForQuery serviceExtensionIntentInfo;
    FormIntentInfoForQuery formIntentInfo;
    std::vector<EntityInfoForQuery> entities {};

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static InsightIntentInfoForQuery *Unmarshalling(Parcel &parcel);
    static bool MarshallingVector(Parcel &parcel, const std::vector<InsightIntentInfoForQuery> &infos);
    static bool UnmarshallingVector(Parcel &parcel, std::vector<InsightIntentInfoForQuery> &infos);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_FOR_QUERY_H
