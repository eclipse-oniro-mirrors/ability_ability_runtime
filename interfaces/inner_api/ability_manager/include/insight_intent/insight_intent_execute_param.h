/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_PARAM_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_PARAM_H

#include <string>
#include <vector>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

/**
 * @enum ParamType
 * ParamType defines the parameter type for function intent validation.
 */
enum class ParamType : uint8_t {
    STRING = 0,
    NUMBER = 1,
    BOOLEAN = 2,
    OBJECT = 3,
    ARRAY = 4,
    INTEGER = 5,
    UNKNOWN = 255
};

/**
 * @struct InsightIntentParam
 * InsightIntentParam defines the parameter info for function intent.
 */
struct InsightIntentParam {
    std::string paramName;
    std::string paramSchema;
    ParamType type = ParamType::UNKNOWN;
    bool isRequired = false;

    InsightIntentParam() = default;
};

/**
 * @enum ExecuteMode
 * ExecuteMode defines the supported execute mode.
 */
enum ExecuteMode {
    UI_ABILITY_FOREGROUND = 0,
    UI_ABILITY_BACKGROUND,
    UI_EXTENSION_ABILITY,
    SERVICE_EXTENSION_ABILITY,
};

constexpr char INSIGHT_INTENT_EXECUTE_PARAM_NAME[] = "ohos.insightIntent.executeParam.name";
constexpr char INSIGHT_INTENT_EXECUTE_PARAM_PARAM[] = "ohos.insightIntent.executeParam.param";
constexpr char INSIGHT_INTENT_EXECUTE_PARAM_MODE[] = "ohos.insightIntent.executeParam.mode";
constexpr char INSIGHT_INTENT_EXECUTE_PARAM_ID[] = "ohos.insightIntent.executeParam.id";
constexpr char INSIGHT_INTENT_EXECUTE_PARAM_URI[] = "ohos.insightIntent.executeParam.uris";
constexpr char INSIGHT_INTENT_EXECUTE_PARAM_FLAGS[] = "ohos.insightIntent.executeParam.flags";
constexpr char INSIGHT_INTENT_SRC_ENTRY[] = "ohos.insightIntent.srcEntry";
constexpr char INSIGHT_INTENT_ARKTS_MODE[] = "ohos.insightIntent.arkTSMode";
constexpr char INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG[] = "ohos.insightIntent.execute.openlink.flag";
constexpr char INSIGHT_INTENT_DECORATOR_TYPE[] = "ohos.insightIntent.decoratorType";
constexpr char INSIGHT_INTENT_SRC_ENTRANCE[] = "ohos.insightIntent.srcEntrance";
constexpr char INSIGHT_INTENT_FUNC_PARAM_CLASSNAME[] = "ohos.insightIntent.funcParam.className";
constexpr char INSIGHT_INTENT_FUNC_PARAM_METHODNAME[] = "ohos.insightIntent.funcParam.methodName";
constexpr char INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS[] = "ohos.insightIntent.funcParam.methodParams";
constexpr char INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE[] = "ohos.insightIntent.funcParam.returnType";
constexpr char INSIGHT_INTENT_PAGE_PARAM_PAGEPATH[] = "ohos.insightIntent.pageParam.pagePath";
constexpr char INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID[] = "ohos.insightIntent.pageParam.navigationId";
constexpr char INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME[] = "ohos.insightIntent.pageParam.navDestinationName";
constexpr char INSIGHT_INTENT_DECORATOR_CLASS[] = "ohos.insightIntent.decoratorClass";
constexpr char INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME[] = "ohos.insightIntent.queryEntity.className";
constexpr char INSIGHT_INTENT_QUERY_TYPE[] = "ohos.insightIntent.queryEntity.queryType";
constexpr char INSIGHT_INTENT_QUERY_ENTITY_PARAM_PARAM[] = "ohos.insightIntent.queryEntity.param";
constexpr char INSIGHT_INTENT_PARAM_USER_ID[] = "ohos.insightIntent.param.userId";
constexpr char INSIGHT_INTENT_DISTRIBUTED_CALLBACK_KEY[] = "ohos.insightIntent.distributed.callbackKey";
constexpr char INSIGHT_INTENT_DISTRIBUTED_SRC_DEVICE_ID[] = "ohos.insightIntent.distributed.srcDeviceId";
constexpr char INSIGHT_INTENT_DISTRIBUTED_REQUEST_CODE[] = "ohos.insightIntent.distributed.requestCode";

constexpr int32_t INVALID_DISPLAY_ID = -1;
constexpr char METHOD_PARAM_SEPARATOR = '\x1f';

std::string EncodeMethodParam(const InsightIntentParam &param);
bool DecodeMethodParam(const std::string &encodedParam, InsightIntentParam &param);
std::string GetMethodParamName(const std::string &encodedMethodParam);

class InsightIntentExecuteParam : public Parcelable {
    const int32_t DEFAULT_INVAL_VALUE = -1;
public:
    InsightIntentExecuteParam() = default;
    ~InsightIntentExecuteParam() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static InsightIntentExecuteParam *Unmarshalling(Parcel &parcel);
    static bool IsInsightIntentExecute(const AAFwk::Want &want);
    static bool IsInsightIntentPage(const AAFwk::Want &want);
    static bool GenerateFromWant(const AAFwk::Want &want, InsightIntentExecuteParam &executeParam);
    static bool RemoveInsightIntent(AAFwk::Want &want);
    static void UpdateInsightIntentCallerInfo(const WantParams &wantParams, WantParams &insightIntentParam);

    int32_t executeMode_ = -1;
    int32_t displayId_ = INVALID_DISPLAY_ID;
    int32_t flags_ = 0;
    int32_t userId_ = DEFAULT_INVAL_VALUE;
    uint64_t insightIntentId_ = 0;
    std::shared_ptr<WantParams> insightIntentParam_;
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::string insightIntentName_;
    std::vector<std::string> uris_;
    int8_t decoratorType_ = 0; // default is InsightIntentType::DECOR_NONE
    std::string srcEntrance_;
    bool isServiceMatch_ = false;

    // params below belongs to InsightIntentFunc
    std::string className_;
    std::string methodName_;
    std::string methodReturnType_;
    std::vector<std::string> methodParams_;

    // params below belongs to InsightIntentPage
    std::string pagePath_;
    std::string navigationId_;
    std::string navDestinationName_;

    // params below belongs to InsightIntentQueryEntity
    std::string queryType_;
    std::string queryEntityClassName_;
    std::shared_ptr<WantParams> queryParams_;
     
    // distributed intent params
    std::string deviceId_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_PARAM_H
