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

#ifndef OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_H

#include "ani.h"
#include "ets_native_reference.h"
#include "form_extension.h"

class NativeReference;
namespace OHOS {
namespace AbilityRuntime {
class FormExtension;
class ETSRuntime;

class ETSFormExtension : public FormExtension {
public:
    static ETSFormExtension *Create(const std::unique_ptr<Runtime> &runtime);
    const ETSRuntime &GetETSRuntime();
    explicit ETSFormExtension(ETSRuntime &etsRuntime);
    ~ETSFormExtension() override;

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;
    OHOS::AppExecFwk::FormProviderInfo OnCreate(const OHOS::AAFwk::Want &want) override;

    void OnDestroy(const int64_t formId) override;

    void OnEvent(const int64_t formId, const std::string &message) override;

    void OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams) override;

    void OnCastToNormal(const int64_t formId) override;

    void OnVisibilityChange(const std::map<int64_t, int32_t> &formEventsMap) override;

    sptr<IRemoteObject> OnConnect(const OHOS::AAFwk::Want &want) override;

    void OnStop() override;

    bool OnShare(int64_t formId, AAFwk::WantParams &wantParams) override;

    bool OnAcquireData(int64_t formId, AAFwk::WantParams &wantParams) override;

    void OnFormLocationChanged(const int64_t formId, const int32_t formLocation) override;

    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    FormState OnAcquireFormState(const Want &want) override;

    void OnSizeChanged(int64_t formId, int32_t newDimension, const Rect &newRect) override;
private:
    void BindContext(std::shared_ptr<AbilityInfo> &abilityInfo, std::shared_ptr<AAFwk::Want> want,
        const std::string &moduleName, const std::string &srcPath);

    void UpdateFormExtensionObj(
        std::shared_ptr<AbilityInfo> &abilityInfo, const std::string &moduleName, const std::string &srcPath);

    void GetSrcPath(std::string &srcPath);

    bool CreateAndFillRecordObject(
        ani_env *env, const std::map<int64_t, int32_t> &formEventsMap, ani_object &recordObject);

    bool CallNativeFormMethod(ani_env *env, ani_object aniWant, ani_ref &nativeResult);

    bool ExtractFormData(ani_env *env, ani_ref nativeResult, AppExecFwk::FormProviderData &formData,
        std::vector<FormDataProxy> &formDataProxies);

    bool ConvertFromDataProxies(ani_env *env, ani_object arrayValue, std::vector<FormDataProxy> &formDataProxies);

    bool ExtractSingleDataProxy(ani_env *env, ani_ref stringEntryRef, FormDataProxy &formDataProxy);

    ani_object WrapFormRect(ani_env *env, const Rect &newRect);
    
    bool CreateObject(ani_env *env, ani_object &object, const std::string &className);

    std::string ANIUtils_ANIStringToStdString(ani_env *env, ani_string ani_str);
    ani_status ANIUtils_FormIdToAniString(ani_env *env, int64_t formId, ani_string &formIdStr);
    std::unique_ptr<NativeReference> etsObj_;
    std::shared_ptr<AppExecFwk::ETSNativeReference> etsAbilityObj_;
    ETSRuntime &etsRuntime_;
    sptr<IRemoteObject> providerRemoteObject_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_ETS_ABILITY_CONTEXT_H
