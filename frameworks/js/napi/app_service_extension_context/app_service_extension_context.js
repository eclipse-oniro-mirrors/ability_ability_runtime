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

let ExtensionContext = requireNapi('application.ExtensionContext');
let hilog = requireNapi('hilog');
let domainID = 0xD001320;
let TAG = 'JSENV';

class AppServiceExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }

  connectServiceExtensionAbility(want, options) {
    hilog.sLogI(domainID, TAG, 'connectServiceExtensionAbility');
    return this.__context_impl__.connectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbility(connection) {
    hilog.sLogI(domainID, TAG, 'disconnectServiceExtensionAbility');
    return this.__context_impl__.disconnectServiceExtensionAbility(connection);
  }

  terminateSelf() {
    hilog.sLogI(domainID, TAG, 'terminateSelf');
    return this.__context_impl__.terminateSelf();
  }
}

export default AppServiceExtensionContext;
