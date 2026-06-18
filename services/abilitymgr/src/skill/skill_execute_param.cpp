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

#include "skill_execute_param.h"

#include <cerrno>
#include <cstdlib>
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_SRC_ENTRIES_COUNT = 500;

int32_t ParseSrcCount(const std::string &countStr)
{
    errno = 0;
    char *endPtr = nullptr;
    long value = std::strtol(countStr.c_str(), &endPtr, 10);
    if (errno == ERANGE || endPtr == countStr.c_str() || *endPtr != '\0' ||
        value < 0 || value > MAX_SRC_ENTRIES_COUNT) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid srcCount: %{public}s", countStr.c_str());
        return 0;
    }
    return static_cast<int32_t>(value);
}
} // namespace

bool SkillExecuteParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    skillName_ = Str16ToStr8(parcel.ReadString16());
    scriptPath_ = Str16ToStr8(parcel.ReadString16());
    functionName_ = Str16ToStr8(parcel.ReadString16());
    auto *args = parcel.ReadParcelable<AAFwk::WantParams>();
    if (args != nullptr) {
        skillArgs_ = std::shared_ptr<AAFwk::WantParams>(args);
    } else {
        skillArgs_ = std::make_shared<AAFwk::WantParams>();
    }
    int32_t srcCount = parcel.ReadInt32();
    if (srcCount < 0 || srcCount > MAX_SRC_ENTRIES_COUNT) {
        return false;
    }
    for (int32_t i = 0; i < srcCount; i++) {
        srcEntries_.push_back(Str16ToStr8(parcel.ReadString16()));
    }
    requestCode_ = Str16ToStr8(parcel.ReadString16());
    hapPath_ = Str16ToStr8(parcel.ReadString16());
    return true;
}

SkillExecuteParam *SkillExecuteParam::Unmarshalling(Parcel &parcel)
{
    auto *param = new (std::nothrow) SkillExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        return nullptr;
    }
    return param;
}

bool SkillExecuteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(skillName_));
    parcel.WriteString16(Str8ToStr16(scriptPath_));
    parcel.WriteString16(Str8ToStr16(functionName_));
    if (skillArgs_ != nullptr) {
        parcel.WriteParcelable(skillArgs_.get());
    } else {
        auto empty = std::make_shared<AAFwk::WantParams>();
        parcel.WriteParcelable(empty.get());
    }
    parcel.WriteInt32(static_cast<int32_t>(srcEntries_.size()));
    for (const auto &entry : srcEntries_) {
        parcel.WriteString16(Str8ToStr16(entry));
    }
    parcel.WriteString16(Str8ToStr16(requestCode_));
    parcel.WriteString16(Str8ToStr16(hapPath_));
    return true;
}

bool SkillExecuteParam::IsSkillExecute(const AAFwk::Want &want)
{
    return want.HasParameter(SKILL_EXECUTE_PARAM_SKILL_NAME);
}

bool SkillExecuteParam::GenerateFromWant(const AAFwk::Want &want, SkillExecuteParam &param)
{
    const WantParams &wantParams = want.GetParams();
    if (!wantParams.HasParam(SKILL_EXECUTE_PARAM_SKILL_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no skill name in want");
        return false;
    }

    param.bundleName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME);
    param.moduleName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_MODULE_NAME);
    param.skillName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_SKILL_NAME);
    param.scriptPath_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_SCRIPT_PATH);
    param.functionName_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_FUNCTION_NAME);

    // Extract skill args from Want
    auto argsKeysStr = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_ARGS_KEYS);
    if (!argsKeysStr.empty()) {
        param.skillArgs_ = std::make_shared<AAFwk::WantParams>();
        std::istringstream stream(argsKeysStr);
        std::string key;
        while (std::getline(stream, key, ';')) {
            if (key.empty()) { continue; }
            auto wantKey = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + key;
            auto it = wantParams.GetParams().find(wantKey);
            if (it != wantParams.GetParams().end()) {
                param.skillArgs_->SetParam(key, it->second);
            }
        }
    }

    int32_t srcCount = 0;
    auto srcCountStr = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT);
    if (!srcCountStr.empty()) {
        srcCount = ParseSrcCount(srcCountStr);
    }
    for (int32_t i = 0; i < srcCount; i++) {
        auto key = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + std::to_string(i);
        param.srcEntries_.push_back(wantParams.GetStringParam(key));
    }
    param.requestCode_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_REQUEST_CODE);
    param.hapPath_ = wantParams.GetStringParam(SKILL_EXECUTE_PARAM_HAP_PATH);
    return true;
}

bool SkillExecuteParam::RemoveSkillParam(AAFwk::Want &want)
{
    auto params = want.GetParams();
    auto argsKeysStr = params.GetStringParam(SKILL_EXECUTE_PARAM_ARGS_KEYS);
    auto srcCountStr = params.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT);

    want.RemoveParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_MODULE_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SKILL_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SCRIPT_PATH);
    want.RemoveParam(SKILL_EXECUTE_PARAM_FUNCTION_NAME);
    want.RemoveParam(SKILL_EXECUTE_PARAM_ARGS_KEYS);
    want.RemoveParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT);
    want.RemoveParam(SKILL_EXECUTE_PARAM_HAP_PATH);
    want.RemoveParam(SKILL_EXECUTE_PARAM_REQUEST_CODE);

    if (!argsKeysStr.empty()) {
        std::istringstream stream(argsKeysStr);
        std::string key;
        while (std::getline(stream, key, ';')) {
            if (key.empty()) { continue; }
            auto wantKey = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + key;
            want.RemoveParam(wantKey);
        }
    }
    if (!srcCountStr.empty()) {
        int32_t srcCount = ParseSrcCount(srcCountStr);
        for (int32_t i = 0; i < srcCount; i++) {
            auto key = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + std::to_string(i);
            want.RemoveParam(key);
        }
    }
    return true;
}

void SkillExecuteParam::WriteToWant(AAFwk::Want &want, const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName,
    const std::string &scriptPath, const std::string &functionName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs,
    const std::vector<std::string> &srcEntries,
    const std::string &requestCode, const std::string &hapPath)
{
    want.SetParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME, bundleName);
    want.SetParam(SKILL_EXECUTE_PARAM_MODULE_NAME, moduleName);
    want.SetParam(SKILL_EXECUTE_PARAM_SKILL_NAME, skillName);
    if (!scriptPath.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_SCRIPT_PATH, scriptPath);
    }
    if (!functionName.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_FUNCTION_NAME, functionName);
    }
    if (skillArgs != nullptr && !skillArgs->GetParams().empty()) {
        std::string argsKeys;
        auto params = want.GetParams();
        for (auto &[key, value] : skillArgs->GetParams()) {
            if (!argsKeys.empty()) { argsKeys += ";"; }
            argsKeys += key;
            auto wantKey = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + key;
            params.SetParam(wantKey, value);
        }
        params.SetParam(SKILL_EXECUTE_PARAM_ARGS_KEYS, AAFwk::String::Box(argsKeys));
        want.SetParams(params);
    }
    if (!srcEntries.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT, std::to_string(srcEntries.size()));
        for (size_t i = 0; i < srcEntries.size(); i++) {
            auto key = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + std::to_string(i);
            want.SetParam(key, srcEntries[i]);
        }
    }
    if (!requestCode.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_REQUEST_CODE, requestCode);
    }
    if (!hapPath.empty()) {
        want.SetParam(SKILL_EXECUTE_PARAM_HAP_PATH, hapPath);
    }
}

} // namespace AppExecFwk
} // namespace OHOS
