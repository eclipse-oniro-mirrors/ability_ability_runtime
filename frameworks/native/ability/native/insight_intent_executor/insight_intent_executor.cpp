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

#include "insight_intent_executor.h"
#include "ets_insight_intent_executor_instance.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "js_insight_intent_entry.h"
#include "js_insight_intent_executor.h"
#include "js_insight_intent_func.h"
#include "js_insight_intent_page.h"
#include "js_insight_intent_query_entity.h"
#include "js_runtime.h"
#include "runtime.h"

namespace OHOS::AbilityRuntime {
std::shared_ptr<InsightIntentExecutor> InsightIntentExecutor::Create(Runtime& runtime, InsightIntentType type)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    switch (runtime.GetLanguage()) {
        case Runtime::Language::JS: {
            switch (type) {
                case InsightIntentType::DECOR_NONE:
                    return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentExecutor::Create(
                        static_cast<JsRuntime&>(runtime)));
                case InsightIntentType::DECOR_ENTRY:
                    return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentEntry::Create(
                        static_cast<JsRuntime&>(runtime)));
                case InsightIntentType::DECOR_FUNC:
                    return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentFunc::Create(
                        static_cast<JsRuntime&>(runtime)));
                case InsightIntentType::DECOR_PAGE:
                    return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentPage::Create(
                        static_cast<JsRuntime&>(runtime)));
                case InsightIntentType::DECOR_QUERY_ENTITY:
                    return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentQueryEntity::Create(
                        static_cast<JsRuntime&>(runtime)));
                case InsightIntentType::DECOR_LINK:
                case InsightIntentType::DECOR_FORM:
                default:
                    return nullptr;
            }
        }
        case Runtime::Language::ETS:
            switch (type) {
                case InsightIntentType::DECOR_NONE:
                    return std::shared_ptr<InsightIntentExecutor>(CreateETSInsightIntentExecutor(runtime));
                case InsightIntentType::DECOR_ENTRY:
                    return std::shared_ptr<InsightIntentExecutor>(CreateETSInsightIntentEntry(runtime));
                case InsightIntentType::DECOR_FUNC:
                    return std::shared_ptr<InsightIntentExecutor>(CreateETSInsightIntentFunc(runtime));
                case InsightIntentType::DECOR_PAGE:
                    return std::shared_ptr<InsightIntentExecutor>(CreateETSInsightIntentPage(runtime));
                case InsightIntentType::DECOR_QUERY_ENTITY:
                    return std::shared_ptr<InsightIntentExecutor>(CreateETSInsightIntentQueryEntity(runtime));
                case InsightIntentType::DECOR_LINK:
                case InsightIntentType::DECOR_FORM:
                default:
                    return nullptr;
            }
        default:
            return nullptr;
    }
}

bool InsightIntentExecutor::Init(const InsightIntentExecutorInfo& intentInfo)
{
    auto executeParam = intentInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return false;
    }

    context_ = std::make_shared<InsightIntentContext>(intentInfo.token, executeParam->bundleName_,
        intentInfo.windowMode, executeParam->insightIntentId_);
    return true;
}

std::shared_ptr<InsightIntentContext> InsightIntentExecutor::GetContext()
{
    return context_;
}
} // namespace OHOS::AbilityRuntime
