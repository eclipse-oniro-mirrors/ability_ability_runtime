/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H

#include <functional>

#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
enum class DialogType {
    DIALOG_ANR = 0,
    DIALOG_TIPS,
    DIALOG_SELECTOR,
};
enum class DialogAlign {
    TOP = 0,
    CENTER,
    BOTTOM,
    LEFT,
    RIGHT,
};
struct DialogPosition {
    DialogAlign align = DialogAlign::CENTER;
    int32_t window_width = 0;
    int32_t window_height = 0;
    int32_t window_offsetX = 0;
    int32_t window_offsetY = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t width_narrow = 0;
    int32_t height_narrow = 0;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    bool wideScreen = true;
    bool oversizeHeight = false;
};
struct DialogAppInfo {
    int32_t abilityIconId = 0;
    int32_t abilityLabelId = 0;
    int32_t bundleIconId = 0;
    int32_t bundleLabelId = 0;
    std::string bundleName = {};
    std::string abilityName = {};
    std::string moduleName = {};
    std::string codePath = "";
    std::string installSource = "";
    bool visible = true;
    bool isAppLink = false;
    AppExecFwk::MultiAppModeData multiAppMode;
    int32_t appIndex = 0;
};
/**
 * @class SystemDialogScheduler
 * SystemDialogScheduler.
 */
class SystemDialogScheduler : public DelayedSingleton<SystemDialogScheduler> {
public:

    explicit SystemDialogScheduler() = default;
    virtual ~SystemDialogScheduler() = default;

    int GetSelectorDialogWantCommon(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant,
        Want &targetWant, const sptr<IRemoteObject> &callerToken);
    int GetPcSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant, Want &targetWant,
        const std::string &type, int32_t userId, const sptr<IRemoteObject> &callerToken);
    int GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant, Want &targetWant,
        const sptr<IRemoteObject> &callerToken);
    Want GetTipsDialogWant(const sptr<IRemoteObject> &callerToken);
    Want GetSwitchUserDialogWant();
    bool GetAssertFaultDialogWant(Want &want);

private:
    const std::string GetSelectorParams(const std::vector<DialogAppInfo> &infos) const;
    const std::string GetPcSelectorParams(const std::vector<DialogAppInfo> &infos,
        const std::string &type, int32_t userId, const std::string &action) const;
    const std::string GetDialogPositionParams(const DialogPosition position) const;

    void InitDialogPosition(DialogType type, DialogPosition &position) const;
    void GetDialogPositionAndSize(DialogType type, DialogPosition &position, int lineNums = 0) const;
    void GetSelectorDialogPositionAndSize(
        DialogPosition &portraitPosition, DialogPosition &landscapePosition, int lineNums) const;
    void GetSelectorDialogLandscapePosition(
        DialogPosition &position, int32_t height, int32_t width, int lineNums, float densityPixels) const;
    void DialogLandscapePositionAdaptive(
        DialogPosition &position, float densityPixels, int lineNums) const;
    void GetSelectorDialogPortraitPosition(
        DialogPosition &position, int32_t height, int32_t width, int lineNums, float densityPixels) const;
    void DialogPortraitPositionAdaptive(
        DialogPosition &position, float densityPixels, int lineNums) const;
    void DialogPositionAdaptive(DialogPosition &position, int lineNums) const;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H
