/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "insight_intent_execute_result.h"

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

namespace {
constexpr const char *KEY_INNER_ERR = "innerErr";
constexpr const char *KEY_CODE = "code";
constexpr const char *KEY_FLAGS = "flags";
constexpr const char *KEY_RESULT = "result";
constexpr const char *KEY_URIS = "uris";
constexpr const char *KEY_IS_DECORATOR = "isDecorator";
constexpr const char *KEY_IS_NEED_DELAY_RESULT = "isNeedDelayResult";
constexpr const char *KEY_IS_QUERY_ENTITY = "isQueryEntity";
constexpr const char *KEY_QUERY_RESULTS = "queryResults";
constexpr int32_t CYCLE_LIMIT = 1000;
} // namespace

bool InsightIntentExecuteResult::ReadFromParcel(Parcel &parcel)
{
    innerErr = parcel.ReadInt32();
    code = parcel.ReadInt32();
    result = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
    if (!parcel.ReadStringVector(&uris)) {
        return false;
    }
    flags = parcel.ReadInt32();
    isDecorator = parcel.ReadBool();
    isQueryEntity = parcel.ReadBool();
    int32_t resultSize = parcel.ReadInt32();
    if (resultSize < 0 || resultSize > CYCLE_LIMIT) {
        return false;
    }
    queryResults.clear();
    for (int32_t i = 0; i < resultSize; i++) {
        auto temp = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
        if (temp == nullptr) {
            return false;
        }
        queryResults.push_back(temp);
    }
    return true;
}

bool InsightIntentExecuteResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(innerErr)) {
        return false;
    }
    if (!parcel.WriteInt32(code)) {
        return false;
    }
    if (!parcel.WriteParcelable(result.get())) {
        return false;
    }
    if (!parcel.WriteStringVector(uris)) {
        return false;
    }
    if (!parcel.WriteInt32(flags)) {
        return false;
    }
    if (!parcel.WriteBool(isDecorator)) {
        return false;
    }
    if (!parcel.WriteBool(isQueryEntity)) {
        return false;
    }
    if (!parcel.WriteInt32(queryResults.size())) {
        return false;
    }
    for (const auto &item : queryResults) {
        if (!parcel.WriteParcelable(item.get())) {
            return false;
        }
    }
    return true;
}

InsightIntentExecuteResult *InsightIntentExecuteResult::Unmarshalling(Parcel &parcel)
{
    auto res = new (std::nothrow) InsightIntentExecuteResult();
    if (res == nullptr) {
        return nullptr;
    }

    if (!res->ReadFromParcel(parcel)) {
        delete res;
        res = nullptr;
    }
    return res;
}

void InsightIntentExecuteResult::FromJsonString(const std::string &jsonStr)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded() || !jsonObject.is_object()) {
        return;
    }

    if (jsonObject.contains(KEY_INNER_ERR) && jsonObject.at(KEY_INNER_ERR).is_number_integer()) {
        innerErr = jsonObject.at(KEY_INNER_ERR).get<int32_t>();
    }
    if (jsonObject.contains(KEY_CODE) && jsonObject.at(KEY_CODE).is_number_integer()) {
        code = jsonObject.at(KEY_CODE).get<int32_t>();
    }
    if (jsonObject.contains(KEY_FLAGS) && jsonObject.at(KEY_FLAGS).is_number_integer()) {
        flags = jsonObject.at(KEY_FLAGS).get<int32_t>();
    }
    if (jsonObject.contains(KEY_IS_DECORATOR) && jsonObject.at(KEY_IS_DECORATOR).is_boolean()) {
        isDecorator = jsonObject.at(KEY_IS_DECORATOR).get<bool>();
    }
    if (jsonObject.contains(KEY_IS_NEED_DELAY_RESULT) && jsonObject.at(KEY_IS_NEED_DELAY_RESULT).is_boolean()) {
        isNeedDelayResult = jsonObject.at(KEY_IS_NEED_DELAY_RESULT).get<bool>();
    }
    if (jsonObject.contains(KEY_IS_QUERY_ENTITY) && jsonObject.at(KEY_IS_QUERY_ENTITY).is_boolean()) {
        isQueryEntity = jsonObject.at(KEY_IS_QUERY_ENTITY).get<bool>();
    }

    if (jsonObject.contains(KEY_URIS) && jsonObject.at(KEY_URIS).is_array()) {
        uris.clear();
        for (const auto &item : jsonObject.at(KEY_URIS)) {
            if (item.is_string()) {
                uris.emplace_back(item.get<std::string>());
            }
        }
    }

    if (jsonObject.contains(KEY_RESULT)) {
        const auto &resultJson = jsonObject.at(KEY_RESULT);
        if (resultJson.is_object()) {
            result = std::make_shared<WantParams>();
            OHOS::AAFwk::from_json(resultJson, *result);
        } else if (resultJson.is_null()) {
            result = nullptr;
        }
    }

    if (jsonObject.contains(KEY_QUERY_RESULTS) && jsonObject.at(KEY_QUERY_RESULTS).is_array()) {
        queryResults.clear();
        for (const auto &item : jsonObject.at(KEY_QUERY_RESULTS)) {
            if (!item.is_object()) {
                continue;
            }
            auto queryResult = std::make_shared<WantParams>();
            OHOS::AAFwk::from_json(item, *queryResult);
            queryResults.emplace_back(queryResult);
        }
    }
}

std::string InsightIntentExecuteResult::ToJsonString() const
{
    nlohmann::json jsonObject;
    jsonObject[KEY_INNER_ERR] = innerErr;
    jsonObject[KEY_CODE] = code;
    jsonObject[KEY_FLAGS] = flags;
    jsonObject[KEY_URIS] = uris;
    jsonObject[KEY_IS_DECORATOR] = isDecorator;
    jsonObject[KEY_IS_NEED_DELAY_RESULT] = isNeedDelayResult;
    jsonObject[KEY_IS_QUERY_ENTITY] = isQueryEntity;

    if (result != nullptr) {
        nlohmann::json resultJson;
        OHOS::AAFwk::to_json(resultJson, *result);
        jsonObject[KEY_RESULT] = resultJson;
    } else {
        jsonObject[KEY_RESULT] = nullptr;
    }

    nlohmann::json queryResultsJson = nlohmann::json::array();
    for (const auto &item : queryResults) {
        if (item == nullptr) {
            continue;
        }
        nlohmann::json itemJson;
        OHOS::AAFwk::to_json(itemJson, *item);
        queryResultsJson.emplace_back(itemJson);
    }
    jsonObject[KEY_QUERY_RESULTS] = queryResultsJson;
    return jsonObject.dump();
}

bool InsightIntentExecuteResult::CheckResult(std::shared_ptr<const WantParams> result)
{
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
