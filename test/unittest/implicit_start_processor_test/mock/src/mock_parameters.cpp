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

#include "parameters.h"
#include <iostream>
#include <unordered_map>

namespace OHOS {
namespace system {

bool g_returnFlag{false};

static std::unordered_map<std::string, std::string>& GetParamsMap()
{
    static std::unordered_map<std::string, std::string> paramsMap;
    return paramsMap;
}

void SetBoolParameter(const std::string& key, bool def)
{
    g_returnFlag = def;
}

bool GetBoolParameter(const std::string& key, bool def)
{
    return g_returnFlag;
}

void ResetParameters()
{
    g_returnFlag = false;
    GetParamsMap().clear();
}

bool SetParameter(const std::string& key, const std::string& value)
{
    GetParamsMap()[key] = value;
    return true;
}

std::string GetParameter(const std::string& key, const std::string& def)
{
    auto& paramsMap = GetParamsMap();
    auto it = paramsMap.find(key);
    if (it != paramsMap.end()) {
        return it->second;
    }
    return def;
}
} // namespace system
} // namespace OHOS
