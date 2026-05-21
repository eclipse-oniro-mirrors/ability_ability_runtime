/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import display from '@ohos.display';
import { AbilityInfoParam, TargetInfo, UIExtensionInfo } from '../data/Model';
import { ContentDisplayComponent } from '../view/ContentDisplayComponent';
import { TargetManager } from '../data/TargetManager';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import wantConstant from '@ohos.app.ability.wantConstant';
import BreakpointSystem, { ColumnType } from '../utils/BreakpointSystem';
import { mediaSyncFacade } from '../utils/MediaQueryFacade';
import mediaquery from '@ohos.mediaquery';
import uriPermissionManager from '@ohos.application.uriPermissionManager';
import Want from '@ohos.app.ability.Want';
import { DeviceInfoUtil } from '../utils/DeviceUtil';
import { TipsDialogComponent } from '../view/TipsDialogComponent';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import { AbilityInfoUtils } from '../utils/AbilityInfoUtils';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import { localStorageKeyConstants, TargetsUpperLimitOneLine } from '../utils/Constants';

const storage = LocalStorage.GetShared();

const TAG = 'SelectorExtensionAbility.PhonePage';

const FOUR_COLUMN_WIDTH: number = 600;

@Entry(storage)
@Component
struct PhonePage {
    @Provide openUIExtension: boolean = false;
    @Provide uiExtensionInfo: UIExtensionInfo | null = null;
    @Provide @Watch('onLevel2PageChanged') level2Page: boolean = false;
    @Provide realHeight: number | PanelHeight = PanelHeight.WRAP_CONTENT;
    @State displayException: boolean = false;
    @State targetInfoArray: Array<TargetInfo> = [];
    @State targetInfoArraylevel2: Array<TargetInfo> = [];
    @State maxLengthOneLine: number = TargetsUpperLimitOneLine.BREAKPOINT_8_12_COLUMN;
    private session: UIExtensionContentSession = storage.get('session') as UIExtensionContentSession;
    @StorageProp('currentColorMode') currentColorMode: number = ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT;
    @StorageProp('currentLanguage') @Watch('refreshTarget') currentLang: string = '';
    private defaultStatusBarHeight: number = 0;
    private statusBarHeight: number = 0;
    private panelTopPadding: number = 0;
    @State panelHeight: number = 0;
    private titleHeight: number = 0;
    private rowHeight: number = 0;
    private rowTopPadding: number = 0;
    private isLandscape: boolean = false;
    private dialogController?: CustomDialogController;
    private tipsDialogBottomOffset: number = 0;
    private listener = mediaquery.matchMediaSync('(orientation: landscape)');
    @State dialogAlign:DialogAlignment = DialogAlignment.Bottom;
    aboutToAppear() {
        console.info(TAG, `aboutToAppear`);
        if (!storage || !storage.get(localStorageKeyConstants.CALLER_WANT)) {
            console.error(TAG, `storage or callerWant is null`);
            return;
        }
        this.refreshTarget();
        const context: Context = getContext(this);
        this.defaultStatusBarHeight = 137;
        this.rowTopPadding = 16;
        let displayData: display.Display;
        try {
            displayData = display.getDefaultDisplaySync();
            this.dialogAlign = DeviceInfoUtil.isTable() || (display.isFoldable() &&
                display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_FULL) ?
            DialogAlignment.Center : DialogAlignment.Bottom;
        } catch (err) {
            console.error(TAG, `Invalid display or screen`);
            return;
        }
        const displayWidth = displayData.width / displayData.densityPixels;
        if (displayWidth < FOUR_COLUMN_WIDTH) {
            this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN;
        }
        this.registerBreakpointSystem();
        this.tipsDialogBottomOffset = -16;
        this.statusBarHeight = this.defaultStatusBarHeight / displayData.densityPixels;
        this.panelTopPadding = 8;
        this.panelHeight = displayData.height / displayData.densityPixels - this.statusBarHeight - this.panelTopPadding;
        const portraitFunc = (mediaQueryResult: mediaquery.MediaQueryResult): void => this.onPortrait(mediaQueryResult);
        this.listener.on('change', portraitFunc);
        this.titleHeight = 56;
        this.rowHeight = 104;
        this.initTipsDialogController();
        this.getTargetsToDisplay();
    }

    refreshTarget() {
        this.targetInfoArraylevel2 = 
                TargetManager.getAvailableTargets(storage.get(localStorageKeyConstants.CALLER_WANT) ?? {});
        this.getTargetsToDisplay();
    }

    registerBreakpointSystem() {
        const context: Context = getContext(this);
        (mediaSyncFacade['BREAK_POINT_SYSTEM'] as BreakpointSystem).register((columnType: ColumnType) => {
            let displayData: display.Display;
            try {
                displayData = display.getDefaultDisplaySync()
                this.dialogAlign = DeviceInfoUtil.isTable() || (display.isFoldable() &&
                    display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_FULL) ?
                DialogAlignment.Center : DialogAlignment.Bottom;
            } catch (err) {
                console.error(TAG, `Invalid display or screen`);
                return;
            }
            if (columnType === ColumnType.BREAKPOINT_8_COLUMN || columnType === ColumnType.BREAKPOINT_12_COLUMN) {
                this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_8_12_COLUMN;
                if (DeviceInfoUtil.isTable()) {
                    this.panelTopPadding = (displayData.height / displayData.densityPixels - this.statusBarHeight) *
                        (this.isLandscape ? 0.1 : 0.2) + this.statusBarHeight;
                }
                this.panelHeight = 
                    displayData.height / displayData.densityPixels - this.statusBarHeight - this.panelTopPadding;
            } else {
                this.maxLengthOneLine = TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN;
                this.panelTopPadding = 8;
                this.panelHeight = 
                    displayData.height / displayData.densityPixels - this.statusBarHeight - this.panelTopPadding;
            }
            if (!this.level2Page) {
                this.targetInfoArray = 
                    TargetManager.removePartialTargetListIfExceedsMax(
                        this.targetInfoArraylevel2, 2 * this.maxLengthOneLine);
            }
        })
    }

    onBackPress() {
        console.info(TAG, `onBackPress.this.level2Page: ${this.level2Page}`);
        if (this.level2Page) {
            this.level2Page = false;
            return;
        }
    }

    onPortrait(mediaQueryResult: mediaquery.MediaQueryResult) {
        if (DeviceInfoUtil.isTable() && mediaQueryResult.matches) {
            this.isLandscape = true;
        } else {
            this.isLandscape = false;
        }
    }

    onLevel2PageChanged() {
        this.getTargetsToDisplay();
    }

    getTargetsToDisplay() {
        if (this.targetInfoArraylevel2.length === 0) {
            this.displayException = true;
            this.openTipsDialogAndReport();
            return;
        }
        if (this.level2Page) {
            this.targetInfoArray = this.targetInfoArraylevel2;
            return;
        }
        this.targetInfoArray = 
            TargetManager.removePartialTargetListIfExceedsMax(this.targetInfoArraylevel2, this.maxLengthOneLine * 2);
    }

    openTipsDialogAndReport(): void {
        this.dialogController?.open();
    }

    initTipsDialogController() {
        this.dialogController = new CustomDialogController({
            builder: TipsDialogComponent({
                storage: storage,
            }),
            autoCancel: false,
            backgroundColor: $r('sys.color.ohos_id_color_dialog_bg'),
            maskColor: $r('sys.color.ohos_id_color_mask_thin'),
            alignment: this.dialogAlign,
            offset: { dx: 0, dy: DeviceInfoUtil.isTable() ? 0 : this.tipsDialogBottomOffset },
            cancel: () => {
                const session: UIExtensionContentSession = 
                    storage.get(localStorageKeyConstants.SESSION) as UIExtensionContentSession;
                session.terminateSelf();
                storage.clear();
            }
        });
    }

    @Builder
    displayPanel() {
        Column() {
            Panel(true) {
                ContentDisplayComponent({
                    storage: storage,
                    maxLengthOneLine: this.maxLengthOneLine,
                    targetInfoArray: this.targetInfoArray,
                    panelHeight: this.panelHeight
                })
            }
            .type(PanelType.CUSTOM)
            .customHeight(this.level2Page ? this.realHeight > this.panelHeight ? this.panelHeight : this.realHeight :
                this.titleHeight + 
                this.rowHeight * (this.targetInfoArray.length > this.maxLengthOneLine ? 2 : 1) + this.rowTopPadding)
            .backgroundColor($r('sys.color.ohos_id_color_panel_bg'))
            .dragBar(false)
            .width(this.maxLengthOneLine === TargetsUpperLimitOneLine.BREAKPOINT_4_COLUMN ? '100%' : 480)
        }
        .width('100%')
        .backgroundColor($r('sys.color.ohos_id_color_mask_thin'))
    }

    @Builder
    displayUIExtension(uiExtension: UIExtensionInfo | null) {
        UIExtensionComponent({
            bundleName: uiExtension?.bundleName,
            abilityName: uiExtension?.abilityName,
            flags: wantConstant.Flags.FLAG_AUTH_READ_URI_PERMISSION,
            parameters: uiExtension?.parameters
        })
            .width('100%')
            .height('100%')
            .onRelease((releaseCode) => {
                this.session.terminateSelf();
                storage.clear();
            })
            .onError((err) => {
                console.error(TAG, `Start ui extension with err: ${err}`)
                this.session.terminateSelf();
                storage.clear();
            })
            .onResult((data) => {
                this.session.terminateSelfWithResult({ resultCode: data.code, want: data.want });
                storage.clear();
            })
    }

    build() {
        Row() {
            if (!this.displayException) {
                if (this.openUIExtension) {
                    this.displayUIExtension(this.uiExtensionInfo);
                } else {
                    this.displayPanel();
                }
            }
        }
        .height('100%')
        .width('100%')
        .backgroundColor($r('sys.color.ohos_id_color_mask_thin'))
    }
}