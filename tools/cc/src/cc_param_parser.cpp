/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "cc_param_parser.h"

#include "want_params_wrapper.h"
#include "string_wrapper.h"
#include "int_wrapper.h"
#include "bool_wrapper.h"
#include "long_wrapper.h"
#include "float_wrapper.h"
#include "double_wrapper.h"
#include "array_wrapper.h"

#include "hilog_tag_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {

WantParams CcParamParser::BuildWantParamsFromJson(const std::string &jsonStr)
{
    WantParams wantParams;

    if (jsonStr.empty()) {
        TAG_LOGW(AAFwkTag::CC_TOOL, "json string is empty");
        return wantParams;
    }

    try {
        nlohmann::json jsonObj = nlohmann::json::parse(jsonStr);
        if (!jsonObj.is_object()) {
            TAG_LOGE(AAFwkTag::CC_TOOL, "json is not an object");
            return wantParams;
        }

        wantParams = ParseJsonObjectToWantParams(jsonObj);

        TAG_LOGI(AAFwkTag::CC_TOOL,
            "BuildWantParamsFromJson success, count: %{public}d",
            wantParams.Size());
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "json parse error: %{public}s", e.what());
    }

    return wantParams;
}

WantParams CcParamParser::ParseJsonObjectToWantParams(
    const nlohmann::json &jsonObj)
{
    WantParams wantParams;

    if (!jsonObj.is_object()) {
        return wantParams;
    }

    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        const std::string &key = it.key();
        const nlohmann::json &value = it.value();

        if (value.is_string()) {
            wantParams.SetParam(key,
                String::Box(value.get<std::string>()));
        } else if (value.is_number_integer()) {
            int64_t longValue = value.get<int64_t>();
            if (longValue > INT32_MAX || longValue < INT32_MIN) {
                wantParams.SetParam(key, Long::Box(longValue));
            } else {
                wantParams.SetParam(key,
                    Integer::Box(static_cast<int32_t>(longValue)));
            }
        } else if (value.is_number_float()) {
            wantParams.SetParam(key,
                String::Box(std::to_string(value.get<double>())));
        } else if (value.is_boolean()) {
            wantParams.SetParam(key, Boolean::Box(value.get<bool>()));
        } else if (value.is_null()) {
            wantParams.SetParam(key, String::Box(""));
        } else if (value.is_object()) {
            WantParams nested = ParseJsonObjectToWantParams(value);
            wantParams.SetParam(key, WantParamWrapper::Box(nested));
        } else if (value.is_array()) {
            sptr<IArray> arr = ParseJsonArrayToIArray(value);
            if (arr != nullptr) {
                wantParams.SetParam(key, arr);
            }
        } else {
            TAG_LOGW(AAFwkTag::CC_TOOL,
                "unsupported type for key: %{public}s", key.c_str());
        }
    }

    return wantParams;
}

sptr<IArray> CcParamParser::ParseJsonArrayToIArray(
    const nlohmann::json &jsonArr)
{
    if (!jsonArr.is_array()) {
        return nullptr;
    }

    std::vector<sptr<IInterface>> items;
    for (const auto &item : jsonArr) {
        if (item.is_object()) {
            WantParams p = ParseJsonObjectToWantParams(item);
            items.push_back(WantParamWrapper::Box(p));
        } else if (item.is_string()) {
            items.push_back(String::Box(item.get<std::string>()));
        } else if (item.is_number_integer()) {
            int64_t lv = item.get<int64_t>();
            if (lv > INT32_MAX || lv < INT32_MIN) {
                items.push_back(Long::Box(lv));
            } else {
                items.push_back(
                    Integer::Box(static_cast<int32_t>(lv)));
            }
        } else if (item.is_number_float()) {
            items.push_back(
                String::Box(std::to_string(item.get<double>())));
        } else if (item.is_boolean()) {
            items.push_back(Boolean::Box(item.get<bool>()));
        } else {
            TAG_LOGW(AAFwkTag::CC_TOOL,
                "unsupported array item type");
        }
    }

    if (items.empty()) {
        return nullptr;
    }

    InterfaceID type = DetectArrayInterfaceType(items[0]);
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "array type detected, size: %{public}zu", items.size());

    sptr<IArray> arrayObj = new (std::nothrow) Array(items.size(), type);
    if (arrayObj != nullptr) {
        for (size_t i = 0; i < items.size(); i++) {
            arrayObj->Set(i, items[i]);
        }
    }

    return arrayObj;
}

InterfaceID CcParamParser::DetectArrayInterfaceType(
    const sptr<IInterface> &firstItem)
{
    if (IString::Query(firstItem) != nullptr) {
        return g_IID_IString;
    }
    if (IInteger::Query(firstItem) != nullptr) {
        return g_IID_IInteger;
    }
    if (ILong::Query(firstItem) != nullptr) {
        return g_IID_ILong;
    }
    if (IBoolean::Query(firstItem) != nullptr) {
        return g_IID_IBoolean;
    }
    if (IFloat::Query(firstItem) != nullptr) {
        return g_IID_IFloat;
    }
    if (IDouble::Query(firstItem) != nullptr) {
        return g_IID_IDouble;
    }
    return g_IID_IWantParams;
}
}  // namespace AAFwk
}  // namespace OHOS
