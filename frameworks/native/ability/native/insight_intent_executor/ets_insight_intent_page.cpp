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

#include "ets_insight_intent_page.h"

#include <want_params.h>
#include <want_params_wrapper.h>

#include "ability_transaction_callback_info.h"
#include "insight_intent_execute_result.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "nlohmann/json.hpp"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define ETS_EXPORT __attribute__((visibility("default")))

namespace OHOS::AbilityRuntime {
using State = EtsInsightIntentUtils::State;

namespace {
constexpr char INSIGHT_INTENT_DOT_BUNDLE_NAME[] = "ohos.insightIntent.bundleName";
constexpr char INSIGHT_INTENT_DOT_MODULE_NAME[] = "ohos.insightIntent.moduleName";
constexpr char INSIGHT_INTENT_DOT_HAP_PATH[] = "ohos.insightIntent.hapPath";
}

InsightIntentExecutor *EtsInsightIntentPage::Create(Runtime &runtime)
{
    return new (std::nothrow) EtsInsightIntentPage(static_cast<ETSRuntime &>(runtime));
}

EtsInsightIntentPage::EtsInsightIntentPage(ETSRuntime &runtime) : runtime_(runtime) {}

EtsInsightIntentPage::~EtsInsightIntentPage()
{
    state_ = State::DESTROYED;
}

bool EtsInsightIntentPage::Init(const InsightIntentExecutorInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    return InsightIntentExecutor::Init(insightIntentInfo);
}

void EtsInsightIntentPage::SetInsightIntentParam(const std::string &hapPath,
    const AAFwk::Want &want, wptr<Rosen::Window> window, bool coldStart)
{
    TAG_LOGD(AAFwkTag::INTENT, "set intent param");
    auto windowSptr = window.promote();
    if (windowSptr == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid window");
        return;
    }

    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return;
    }

    AAFwk::Want newWant;
    auto insightIntentParam = executeParam->insightIntentParam_;
    if (insightIntentParam != nullptr) {
        sptr<AAFwk::IWantParams> pExecuteParams = AAFwk::WantParamWrapper::Box(*insightIntentParam);
        if (pExecuteParams != nullptr) {
            AAFwk::WantParams wantParams;
            wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, pExecuteParams);
            newWant.SetParams(wantParams);
        }
    }
    newWant.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_PAGEPATH, executeParam->pagePath_);
    newWant.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID, executeParam->navigationId_);
    newWant.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME, executeParam->navDestinationName_);
    newWant.SetParam(INSIGHT_INTENT_DOT_BUNDLE_NAME, executeParam->bundleName_);
    newWant.SetParam(INSIGHT_INTENT_DOT_MODULE_NAME, executeParam->moduleName_);
    newWant.SetParam(INSIGHT_INTENT_DOT_HAP_PATH, hapPath);

    std::string paramStr = newWant.GetParams().ToString();
    TAG_LOGD(AAFwkTag::INTENT, "param string %{private}s", paramStr.c_str());

    Rosen::WMError wmRet = windowSptr->SetIntentParam(paramStr, []() {}, coldStart);
    if (wmRet != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Set intent param failed %{public}d", wmRet);
    }
}

bool EtsInsightIntentPage::HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    void *pageLoader, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::INTENT, "HandleExecuteIntent called");
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null execute param");
        EtsInsightIntentUtils::ReplyFailed(callback.release());
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    isAsync = false;
    auto result = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    ReplySucceededInner(result);
    state_ = State::EXECUTATION_DONE;
    return true;
}

void EtsInsightIntentPage::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    EtsInsightIntentUtils::ReplyFailed(callback_.release(), innerErr);
}

void EtsInsightIntentPage::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
}
} // namespace OHOS::AbilityRuntime

ETS_EXPORT extern "C" OHOS::AbilityRuntime::InsightIntentExecutor *OHOS_ETS_Insight_Intent_Page_Create(
    OHOS::AbilityRuntime::Runtime &runtime)
{
    return OHOS::AbilityRuntime::EtsInsightIntentPage::Create(runtime);
}
