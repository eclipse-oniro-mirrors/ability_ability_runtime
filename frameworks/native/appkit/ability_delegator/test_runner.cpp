/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "test_runner.h"

#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#ifdef CJ_FRONTEND
#include "runner_runtime/cj_test_runner.h"
#endif
#include "runner_runtime/js_test_runner.h"
#include "runtime.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
std::unique_ptr<TestRunner> TestRunner::Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, bool isFaJsModel)
{
    if (!runtime) {
        return std::make_unique<TestRunner>();
    }

    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null bundleMgrHelper");
        return nullptr;
    }

    if (!args) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid args");
        return nullptr;
    }

    BundleInfo bundleInfo;
    if (bundleMgrHelper->GetBundleInfoForSelf(
        (static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "get bundle info failed");
        return nullptr;
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return RunnerRuntime::JsTestRunner::Create(runtime, args, bundleInfo, isFaJsModel);
#ifdef CJ_FRONTEND
        case AbilityRuntime::Runtime::Language::CJ:
            return RunnerRuntime::CJTestRunner::Create(runtime, args, bundleInfo);
#endif
        default:
            return std::make_unique<TestRunner>();
    }
}

void TestRunner::Prepare()
{}

void TestRunner::Run()
{}

bool TestRunner::Initialize()
{
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
