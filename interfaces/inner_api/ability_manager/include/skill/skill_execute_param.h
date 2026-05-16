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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_PARAM_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_PARAM_H

#include <string>
#include <vector>

#include "parcel.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {

using WantParams = OHOS::AAFwk::WantParams;

struct SkillExecuteRequest {
    uint32_t callerTokenId = 0;
    std::string bundleName;
    std::string moduleName;
    std::string skillName;
    std::string scriptPath;
    std::string functionName;
    std::shared_ptr<WantParams> skillArgs;
};

// Want parameter keys for skill execution
constexpr char SKILL_EXECUTE_PARAM_BUNDLE_NAME[] = "ohos.skill.executeParam.bundleName";
constexpr char SKILL_EXECUTE_PARAM_MODULE_NAME[] = "ohos.skill.executeParam.moduleName";
constexpr char SKILL_EXECUTE_PARAM_SKILL_NAME[] = "ohos.skill.executeParam.skillName";
constexpr char SKILL_EXECUTE_PARAM_SCRIPT_PATH[] = "ohos.skill.executeParam.scriptPath";
constexpr char SKILL_EXECUTE_PARAM_FUNCTION_NAME[] = "ohos.skill.executeParam.functionName";
constexpr char SKILL_EXECUTE_PARAM_ARGS_KEYS[] = "ohos.skill.executeParam.argsKeys";
constexpr char SKILL_EXECUTE_PARAM_ARGS_PREFIX[] = "ohos.skill.executeParam.args.";
constexpr char SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT[] = "ohos.skill.executeParam.srcEntriesCount";
constexpr char SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX[] = "ohos.skill.executeParam.srcEntry.";
constexpr char SKILL_EXECUTE_PARAM_HAP_PATH[] = "ohos.skill.executeParam.hapPath";
constexpr char SKILL_EXECUTE_PARAM_REQUEST_CODE[] = "ohos.skill.executeParam.requestCode";

class SkillExecuteParam : public Parcelable {
public:
    SkillExecuteParam() = default;
    ~SkillExecuteParam() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static SkillExecuteParam *Unmarshalling(Parcel &parcel);

    static bool IsSkillExecute(const AAFwk::Want &want);
    static bool GenerateFromWant(const AAFwk::Want &want, SkillExecuteParam &param);
    static bool RemoveSkillParam(AAFwk::Want &want);
    static void WriteToWant(AAFwk::Want &want, const std::string &bundleName,
        const std::string &moduleName, const std::string &skillName,
        const std::string &scriptPath = "", const std::string &functionName = "",
        const std::shared_ptr<AAFwk::WantParams> &skillArgs = nullptr,
        const std::vector<std::string> &srcEntries = {},
        const std::string &requestCode = "", const std::string &hapPath = "");

    std::string bundleName_;
    std::string moduleName_;
    std::string skillName_;
    std::string scriptPath_;
    std::string functionName_;
    std::shared_ptr<AAFwk::WantParams> skillArgs_;
    std::vector<std::string> srcEntries_;
    std::string hapPath_;
    std::string requestCode_;
};

} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_PARAM_H
