/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

namespace wantConstant {
  export enum Params {
    DLP_PARAMS_SANDBOX = 'ohos.dlp.params.sandbox',
    DLP_PARAMS_BUNDLE_NAME = 'ohos.dlp.params.bundleName',
    DLP_PARAMS_MODULE_NAME = 'ohos.dlp.params.moduleName',
    DLP_PARAMS_ABILITY_NAME = 'ohos.dlp.params.abilityName',
    DLP_PARAMS_INDEX = 'ohos.dlp.params.index',
    HIDE_SENSITIVE_TYPE = 'ohos.media.params.hideSensitiveType',
    ABILITY_BACK_TO_OTHER_MISSION_STACK = 'ability.params.backToOtherMissionStack',
    ABILITY_RECOVERY_RESTART = 'ohos.ability.params.abilityRecoveryRestart',
    CONTENT_TITLE_KEY = 'ohos.extra.param.key.contentTitle',
    SHARE_ABSTRACT_KEY = 'ohos.extra.param.key.shareAbstract',
    SHARE_URL_KEY = 'ohos.extra.param.key.shareUrl',
    SUPPORT_CONTINUE_PAGE_STACK_KEY = 'ohos.extra.param.key.supportContinuePageStack',
    SUPPORT_CONTINUE_SOURCE_EXIT_KEY = 'ohos.extra.param.key.supportContinueSourceExit',
    ASSERT_FAULT_SESSION_ID = 'ohos.ability.params.asssertFaultSessionId',
    SHOW_MODE_KEY = 'ohos.extra.param.key.showMode',
    PARAMS_STREAM = 'ability.params.stream',
    APP_CLONE_INDEX_KEY = 'ohos.extra.param.key.appCloneIndex',
    CALLER_REQUEST_CODE = 'ohos.extra.param.key.callerRequestCode',
    PAGE_PATH = 'ohos.param.atomicservice.pagePath',
    ROUTER_NAME = 'ohos.param.atomicservice.routerName',
    PAGE_SOURCE_FILE = 'ohos.param.atomicservice.pageSourceFile',
    BUILD_FUNCTION = 'ohos.param.atomicservice.buildFunction',
    SUB_PACKAGE_NAME = 'ohos.param.atomicservice.subpackageName',
    APP_INSTANCE_KEY = 'ohos.extra.param.key.appInstance',
    CREATE_APP_INSTANCE_KEY = 'ohos.extra.param.key.createAppInstance',
    CALLER_APP_CLONE_INDEX = 'ohos.param.callerAppCloneIndex',
    LAUNCH_REASON_MESSAGE = 'ohos.params.launchReasonMessage',
    ATOMIC_SERVICE_SHARE_ROUTER = 'ohos.params.atomicservice.shareRouter',
    DESTINATION_PLUGIN_ABILITY = 'ohos.params.pluginAbility',
    APP_LAUNCH_TRUSTLIST = 'ohos.params.appLaunchTrustList',
    ABILITY_UNIFIED_DATA_KEY = 'ohos.param.ability.udKey',
    UI_EXTENSION_ROOT_TOKEN = 'ohos.param.uiExtension.rootHostToken',
  }

  export enum Flags {
    FLAG_AUTH_READ_URI_PERMISSION = 0x00000001,
    FLAG_AUTH_WRITE_URI_PERMISSION = 0x00000002,
    FLAG_AUTH_PERSISTABLE_URI_PERMISSION = 0x00000040,
    FLAG_INSTALL_ON_DEMAND = 0x00000800,
    FLAG_ABILITY_ON_COLLABORATE = 0x00002000,
    FLAG_START_WITHOUT_TIPS = 0x40000000
  }

  export enum ShowMode {
    WINDOW = 0,
    EMBEDDED_FULL = 1,
    EMBEDDED_HALF = 2
  }

  export enum Action {
    ACTION_SEND_TO_DATA = 'ohos.want.action.sendToData'
  }
}

export default wantConstant;