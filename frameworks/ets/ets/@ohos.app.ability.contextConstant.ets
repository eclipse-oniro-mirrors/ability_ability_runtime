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

namespace contextConstant {
  export enum AreaMode {
    EL1 = 0,
    EL2 = 1,
    EL3 = 2,
    EL4 = 3,
    EL5 = 4
  }

  export enum ProcessMode {
    NEW_PROCESS_ATTACH_TO_PARENT = 1,
    NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM = 2,
    ATTACH_TO_STATUS_BAR_ITEM = 3
  }
  
  export enum StartupVisibility {
    STARTUP_HIDE = 0,
    STARTUP_SHOW = 1
  }
  
  export enum Scenarios {
    SCENARIO_MOVE_MISSION_TO_FRONT = 0x00000001,
    SCENARIO_SHOW_ABILITY = 0x00000002,
    SCENARIO_BACK_TO_CALLER_ABILITY_WITH_RESULT = 0x00000004
  }

  export enum ContextType {
    APPLICATION_CONTEXT = 0,
 	  ABILITY_STAGE_CONTEXT = 1,
 	  UIABILITY_CONTEXT = 2,
 	  FORM_EXTENSION_CONTEXT = 3,
 	  APP_SERVICE_EXTENSION_CONTEXT = 4,
 	  SERVICE_EXTENSION_CONTEXT = 5,
 	  UI_SERVICE_EXTENSION_CONTEXT = 6,
    AUTO_FILL_EXTENSION_CONTEXT = 7
  }
}

export default contextConstant;
