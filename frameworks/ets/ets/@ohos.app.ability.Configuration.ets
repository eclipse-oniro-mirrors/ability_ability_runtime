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

import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';

export interface Configuration {
  language?: string;
  colorMode?: ConfigurationConstant.ColorMode;
  direction?: ConfigurationConstant.Direction;
  screenDensity?: ConfigurationConstant.ScreenDensity;
  displayId?: long;
  hasPointerDevice?: boolean;
  fontId?: string;
  fontSizeScale?: double;
  fontWeightScale?: double;
  mcc?: string;
  mnc?: string;
  locale?: Intl.Locale;
}

class ConfigurationImpl implements Configuration {
  language?: string;
  colorMode?: ConfigurationConstant.ColorMode;
  direction?: ConfigurationConstant.Direction;
  screenDensity?: ConfigurationConstant.ScreenDensity;
  displayId?: long;
  hasPointerDevice?: boolean;
  fontId?: string;
  fontSizeScale?: double;
  fontWeightScale?: double;
  mcc?: string;
  mnc?: string;
  locale?: Intl.Locale;

  constructor(languageParam: string, colorModeParam:ConfigurationConstant.ColorMode,
  directionParam: ConfigurationConstant.Direction, screenDensityParam: ConfigurationConstant.ScreenDensity,
  displayIdParam: long, hasPointerDeviceParam: boolean, fontIdParam: string, fontSizeScaleParam: double,
  fontWeightScaleParam: double, mccParam: string, mncParam: string, localeParam: Intl.Locale) {
    this.language = languageParam;
    this.colorMode = colorModeParam;
    this.direction = directionParam;
    this.screenDensity = screenDensityParam;
    this.displayId = displayIdParam;
    this.hasPointerDevice = hasPointerDeviceParam;
    this.fontId = fontIdParam;
    this.fontSizeScale = fontSizeScaleParam;
    this.fontWeightScale = fontWeightScaleParam;
    this.mcc = mccParam;
    this.mnc = mncParam;
    this.locale = localeParam;
  }



}
