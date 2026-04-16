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

#include "js_insight_intent_page.h"

#include <want_params.h>

#include "execute_ohmurl_operator.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char INSIGHT_INTENT_DOT_BUNDLE_NAME[] = "ohos.insightIntent.bundleName";
constexpr char INSIGHT_INTENT_DOT_MODULE_NAME[] = "ohos.insightIntent.moduleName";
constexpr char INSIGHT_INTENT_DOT_HAP_PATH[] = "ohos.insightIntent.hapPath";
} // namespace

std::shared_ptr<JsInsightIntentPage> JsInsightIntentPage::Create(JsRuntime& runtime)
{
    return std::make_shared<JsInsightIntentPage>(runtime);
}

JsInsightIntentPage::JsInsightIntentPage(JsRuntime& runtime) : runtime_(runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "constructor");
}

JsInsightIntentPage::~JsInsightIntentPage()
{
    state_ = State::DESTROYED;
    if (safeData_ != nullptr) {
        safeData_->SetAutoReleaseMem(true);
    }
    TAG_LOGI(AAFwkTag::INTENT, "destructor");
}

bool JsInsightIntentPage::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "Init");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    bool ret = JsInsightIntentPage::LoadJsCode(insightIntentInfo, runtime_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "load js failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    return true;
}

bool JsInsightIntentPage::HandleExecuteIntent(
    std::shared_ptr<InsightIntentExecuteParam> executeParam,
    const std::shared_ptr<NativeReference>& pageLoader,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
    bool& isAsync)
{
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    auto name = executeParam->insightIntentName_;
    auto param = executeParam->insightIntentParam_;
    TAG_LOGD(AAFwkTag::INTENT, "execute intent %{public}s", name.c_str());
    bool successful = ExecuteInsightIntent(name, *param);
    isAsync = false;
    return successful;
}

bool JsInsightIntentPage::LoadJsCode(const InsightIntentExecutorInfo& info, JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "load module");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return false;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGD(AAFwkTag::INTENT, "moduleName %{public}s, hapPath %{private}s, srcEntrance %{private}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    safeData_ = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    return safeData_ != nullptr;
}

void JsInsightIntentPage::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply failed");
    state_ = State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplyFailed(callback, innerErr);
}

void JsInsightIntentPage::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply succeed");
    state_ = State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
}

bool JsInsightIntentPage::ExecuteInsightIntent(
    const std::string& name,
    const AAFwk::WantParams& param)
{
    TAG_LOGD(AAFwkTag::INTENT, "execute insight intent page");
    auto result = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    ReplySucceededInner(result);
    return true;
}

void JsInsightIntentPage::SetInsightIntentParam(JsRuntime& runtime, const std::string &hapPath,
    const AAFwk::Want &want, wptr<Rosen::Window> window, bool coldStart)
{
    TAG_LOGD(AAFwkTag::INTENT, "set intent param");
    auto windowSptr = window.promote();
    if (windowSptr == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid window");
        return;
    }

    // parse want and get insight intent info
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return;
    }

    Want newWant;
    auto insightIntentParam = executeParam->insightIntentParam_;
    if (insightIntentParam != nullptr) {
        sptr<AAFwk::IWantParams> pExecuteParams = AAFwk::WantParamWrapper::Box(*insightIntentParam);
        if (pExecuteParams != nullptr) {
            WantParams wantParams;
            wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, pExecuteParams);
            newWant.SetParams(wantParams);
        }
    }
    // param of page insight intent
    newWant.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_PAGEPATH, executeParam->pagePath_);
    newWant.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID, executeParam->navigationId_);
    newWant.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME, executeParam->navDestinationName_);

    // param of common
    newWant.SetParam(INSIGHT_INTENT_DOT_BUNDLE_NAME, executeParam->bundleName_);
    newWant.SetParam(INSIGHT_INTENT_DOT_MODULE_NAME, executeParam->moduleName_);
    newWant.SetParam(INSIGHT_INTENT_DOT_HAP_PATH, hapPath);

    auto* env = runtime.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "env nullptr");
        return;
    }
    HandleScope handleScope(env);
    napi_value paramNapiVal = AppExecFwk::WrapWantParams(env, newWant.GetParams());
    std::string paramStr = JsInsightIntentUtils::StringifyObject(env, paramNapiVal);
    TAG_LOGD(AAFwkTag::INTENT, "param string %{private}s", paramStr.c_str());

    Rosen::WMError wmRet = windowSptr->SetIntentParam(paramStr,
        ExecuteOhmUrlOperator(runtime, executeParam->moduleName_, hapPath, executeParam->pagePath_), coldStart);
    if (wmRet != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Set intent param failed %{public}d", wmRet);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
