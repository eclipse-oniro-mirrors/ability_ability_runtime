/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H
#define OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H

#include <list>
#include <vector>
#include <string>
#include <unordered_set>

#include "ability_record.h"
#include "bundle_mgr_helper.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace EcologicalRuleMgrService {
struct AbilityCallerInfo;
}
namespace AAFwk {
struct IdentityNode {
    int32_t tokenId;
    std::string identity;
    IdentityNode(int tokenId, std::string identity) : tokenId(tokenId), identity(identity)
    {}
};

struct AddInfoParam {
    AppExecFwk::AbilityInfo info;
    int32_t userId = 0;
    bool isExtension = false;
    bool isMoreHapList = false;
    bool withDefault = false;
    std::string typeName;
    std::vector<std::string> infoNames;
};
using namespace OHOS::EcologicalRuleMgrService;
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::AbilityCallerInfo;
/**
 * @class ImplicitStartProcessor
 * ImplicitStartProcessor.
 */
class ImplicitStartProcessor : public std::enable_shared_from_this<ImplicitStartProcessor> {
public:
    explicit ImplicitStartProcessor() = default;
    virtual ~ImplicitStartProcessor() = default;

    static bool IsImplicitStartAction(const Want &want);

    int ImplicitStartAbility(AbilityRequest &request, int32_t userId, int32_t windowMode = 0,
        const std::string &replaceWantString = "", bool isAppCloneSelector = false);

    void ResetCallingIdentityAsCaller(int32_t tokenId);

private:
    int GenerateAbilityRequestByAction(int32_t userId, AbilityRequest &request,
        std::vector<DialogAppInfo> &dialogAppInfos, bool isMoreHapList, bool &findDefaultApp);

    int GenerateAbilityRequestByAppIndexes(int32_t userId, AbilityRequest &request,
        std::vector<DialogAppInfo> &dialogAppInfos);

    int FindExtensionInfo(const Want &want, int32_t flags, int32_t userId, int32_t appIndex,
        AppExecFwk::AbilityInfo &abilityInfo);

    std::string MatchTypeAndUri(const AAFwk::Want &want);
    std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManagerHelper();
    std::vector<std::string> SplitStr(const std::string& str, char delimiter);
    int QueryBmsAppInfos(AbilityRequest &request, int32_t userId, std::vector<DialogAppInfo> &dialogAppInfos);

    using StartAbilityClosure = std::function<int32_t()>;
    int CallStartAbilityInner(int32_t userId, const Want &want, const StartAbilityClosure &callBack,
        const AbilityCallType &callType);

    int32_t ImplicitStartAbilityInner(const Want &targetWant, const AbilityRequest &request, int32_t userId);

    bool CheckImplicitStartExtensionIsValid(const AbilityRequest &request,
        const AppExecFwk::ExtensionAbilityInfo &extensionInfo);

    bool FilterAbilityList(const Want &want, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, int32_t userId);
    sptr<AppExecFwk::IDefaultApp> GetDefaultAppProxy();

    void GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId);

    void AddIdentity(int32_t tokenId, std::string identity);

    void AddAbilityInfoToDialogInfos(const AddInfoParam &param, std::vector<DialogAppInfo> &dialogAppInfos);

    bool IsExistDefaultApp(int32_t userId, const std::string &typeName);

    void SetTargetLinkInfo(const std::vector<AppExecFwk::SkillUriForAbilityAndExtension> &skillUri, Want &want);

    void OnlyKeepReserveApp(std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
        std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos, const AbilityRequest &request);

    bool IsActionImplicitStart(const Want &want, bool findDeafultApp);

private:
    const static std::vector<std::string> blackList;
    const static std::unordered_set<AppExecFwk::ExtensionAbilityType> extensionWhiteList;
    std::shared_ptr<AppExecFwk::BundleMgrHelper> iBundleManagerHelper_;
    ffrt::mutex identityListLock_;
    std::list<IdentityNode> identityList_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H