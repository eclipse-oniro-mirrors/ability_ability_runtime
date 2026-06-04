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

import { application, common, insightIntentDriver } from '@kit.AbilityKit';
import { BusinessError } from '@kit.BasicServicesKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { image } from '@kit.ImageKit';
import { jsClone } from './format';
import { TAG, DOMAIN} from '../pages/Index'
import bundle from '@ohos.bundle.bundleManager';


export interface ItemInfo {
  title: string,
  brief: string
}

export interface IntentInfo extends insightIntentDriver.InsightIntentInfo {
  bundleNameCN: string,
  iconImg: image.PixelMap | ResourceStr | DrawableDescriptor | undefined
}

export interface IntentGrout {
  string: IntentInfo[]
}

interface MaterialResult {
  bundleName: string;
  materialItems: IntentInfo[];
}

@Component
export struct IntentContent {
  @Consume('articlePathStack') articlePathStack: NavPathStack;
  scroller: Scroller = new Scroller();
  @State listMaterial: IntentInfo[] | null = null;
  @State intermediateGroupMaterial: Record<string, IntentInfo[]> = {};
  @State groupMaterial: Record<string, IntentInfo[]> = {};
  context = this.getUIContext().getHostContext();
  @State pixelMap: image.PixelMap | ResourceStr | DrawableDescriptor | undefined = undefined;

  aboutToAppear() {
    this.getInfos();
  }

  onBackPress() {
    this.terminateSelf();
  }

  terminateSelf() {
    try {
      let context = this.getUIContext().getHostContext() as common.UIAbilityContext;
      context.terminateSelf();
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to context.terminateSelf. Cause: %{public}s', JSON.stringify(err));
    }
  }

  async getInfos() {
    try {
      insightIntentDriver.getAllInsightIntentInfo(insightIntentDriver.GetInsightIntentFlag.GET_FULL_INSIGHT_INTENT)
        .then((data) => {
          hilog.info(DOMAIN, TAG, `getAllInsightIntentInfo success: found ${data.length} total intents`);
          this.listMaterial = data as IntentInfo[];
          this.intermediateGroupMaterial = this.listMaterial
            .filter(item => item.developType != insightIntentDriver.DevelopType.CONFIGURATION)
            .reduce((acc, item) => {
              const bundleName: string = item.bundleName;
              if (!acc[bundleName]) {
                acc[bundleName] = [];
              }
              acc[bundleName].push(item);
              return acc;
          }, {} as Record<string, IntentInfo[]>)
          const bundleNames = Object.keys(this.intermediateGroupMaterial);
          const promises = bundleNames.map(async (bundleName) => {
            try {
              const result = await bundle.getApplicationInfo(
                bundleName, bundle.ApplicationFlag.GET_APPLICATION_INFO_WITH_PERMISSION);
              if (result.debug) {
                const materialItems = this.intermediateGroupMaterial[bundleName];
                for (const key of Object.keys(materialItems)) {
                  const iconImg = await this.getAllIntentIcon(materialItems[key].icon, bundleName);
                  materialItems[key].iconImg = iconImg;
                  const appName = await this.setAppName(materialItems[0]);
                  materialItems[key].bundleNameCN = appName;
                }
                const materialResult: MaterialResult = { bundleName, materialItems };
                return materialResult;
              }
            } catch (err) {
              hilog.error(DOMAIN, TAG, 'getApplicationInfo failed. Cause: %{public}s', JSON.stringify(err));
            }
            return null;
          });
          Promise.all(promises).then((results: Array<MaterialResult | null>) => {
            results.forEach((result: MaterialResult | null) => {
              if (result && result.bundleName && result.materialItems) {
                this.groupMaterial[result.bundleName] = result.materialItems;
              }
            });
            this.groupMaterial = jsClone(this.groupMaterial);
            Object.keys(this.groupMaterial).forEach((bundleName: string) => {
              const intents = this.groupMaterial[bundleName];
              hilog.info(DOMAIN, TAG, `Bundle: ${bundleName}, AppName: ${intents[0].bundleNameCN}, Intents: ${intents.length}`);
              intents.forEach((intent: IntentInfo, index: number) => {
                hilog.info(DOMAIN, TAG, `  [${index + 1}] IntentName: ${intent.intentName}, Type: ${intent.intentType}, DisplayName: ${intent.displayName}, Module: ${intent.moduleName}`);
              });
            });
          });
        }).catch((err: BusinessError) => {
          hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errCode: %{public}d', err.code);
          hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo errMessage: %{public}s', err.message);
        });
    } catch (error) {
      hilog.error(DOMAIN, TAG, 'getAllInsightIntentInfo error caught %{public}s', JSON.stringify(error));
    }
  }

  async setAppName(intentDetail: IntentInfo): Promise<string> {
    let bundleName: string = intentDetail.bundleName;
    let moduleName: string = intentDetail.moduleName;
    let context = this.getUIContext().getHostContext();
    let bundleNameCN: string = '';
    try {
      const bundleContext: Context = await application.createModuleContext(context, bundleName, moduleName);
      bundleNameCN = bundleContext.resourceManager.getStringSync(bundleContext.applicationInfo.labelId);
    } catch (error) {
      hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(error));
    }
    return bundleNameCN;
  }

  getDisplayName(bundleName: string): string {
    const materialItems = this.groupMaterial[bundleName];
    if (!materialItems || materialItems.length === 0) {
      return bundleName;
    }
    const appName = materialItems[0].bundleNameCN;
    return appName;
  }

  @Builder
  IntentBlock(domainType: string, material: IntentInfo[]) {
    Column() {
      Text(domainType)
        .fontFamily('HarmonyHeiTi-Medium')
        .fontSize(16)
        .fontWeight(500)
        .fontColor('#67686A')
        .margin({ left: 10, bottom: 8 })
        .textAlign(TextAlign.Start)
        .width('100%')

      List({ space: 12 }) {
        ForEach(material, (item: IntentInfo, index: number) => {
          ListItem() {
            this.IntentBlockLine(item);
          }
          .onClick(() => {
            this.articlePathStack.replacePath({ name: 'intentDetail', param: item });
          })
        }, (item: IntentInfo, index: number) => item.intentName);
      }
      .backgroundColor($r('app.color.background_list'))
      .borderRadius(16)
      .padding({
        top: 4,
        left: 12,
        right: 12,
        bottom: 4
      })
      .divider({
        strokeWidth: 0.5,
        startMargin: 38,
        endMargin: 0,
        color: $r('app.color.border_color_02')
      })

    }
    .width('100%')
    .margin({ top: 10, bottom: 18 })
    .alignItems(HorizontalAlign.Center)
  }

  async getAllIntentIcon(icon: string,
    bundleName: string): Promise<PixelMap | ResourceStr | DrawableDescriptor | undefined> {
    let pixelMap: image.PixelMap | ResourceStr | DrawableDescriptor | undefined = undefined;
    if (icon?.startsWith('$r')) {
      try {
        let resourceStr: string | undefined = icon.match(/['"](.+)['"]/)?.[1];
        if (!resourceStr) {
          return icon;
        }
        let resourceName: string | undefined = resourceStr.split('.').pop();
        if (!resourceName) {
          return icon;
        }
        let context = this.getUIContext().getHostContext();

        const bundleContextPromise: Promise<Context> = application.createBundleContext(context, bundleName);
        bundleContextPromise.catch((error: BusinessError) => {
          hilog.error(DOMAIN, TAG, 'createBundleContext failed. Cause: %{public}s', JSON.stringify(error));
        })
        const bundleContext = await bundleContextPromise;
        const valuePromise: Promise<Uint8Array> = bundleContext.resourceManager.getMediaByName(resourceName);
        valuePromise.catch((error: BusinessError) => {
          hilog.error(DOMAIN, TAG, 'callback getMediaByName failed. Cause: %{public}s', JSON.stringify(error));
        })
        const value: Uint8Array = await valuePromise;
        pixelMap = await this.uint8ArrayToImage(value) ?? undefined;

      } catch (err) {
        hilog.error(DOMAIN, TAG, 'Failed to getAllIntentIcon', JSON.stringify(err));
        return pixelMap;
      }
    } else {
      return '';
    }
    return pixelMap;
  }

  async uint8ArrayToImage(buffer: Uint8Array): Promise<PixelMap | null> {
    let imgIcon: image.PixelMap | null = null;
    let imageSource: image.ImageSource | null = null;
    try {
      imageSource = image.createImageSource(buffer.buffer);
      if (!imageSource) {
        throw new Error('Failed to create ImageSource from buffer.');
      }
      imgIcon = await this.getUint8Array(imageSource);
      return imgIcon;
    } catch (error) {
      hilog.error(DOMAIN, TAG, 'Error in uint8ArrayToImage', JSON.stringify(error));
    } finally {
      if (imageSource) {
        imageSource.release();
      }
    }
    return imgIcon;
  }

  async getUint8Array(imageSource: image.ImageSource): Promise<PixelMap | null> {
    let imageIcon: image.PixelMap | null = null;
    try {
      imageIcon = await imageSource.createPixelMap({
        desiredSize: { width: 24, height: 24 }
      });
      return imageIcon;
    } catch (error) {
      hilog.error(DOMAIN, TAG, 'Error in getUint8Array', JSON.stringify(error));
    }
    return imageIcon;
  }

  @Builder
  IntentBlockLine(intentDetail: IntentInfo) {
    Row() {
      Image(intentDetail.iconImg ?? $r('app.media.defaultIcon'))
        .width(24)
        .height(24)
        .borderRadius(6)
        .clip(true)
        .objectFit(ImageFit.Auto)
        .alt($r('app.media.defaultIcon'))

      Column() {
        Text(intentDetail.displayName)
          .fontFamily('HarmonyHeiTi-Medium')
          .fontSize(16)
          .fontColor('#494947')
          .fontWeight(500)
          .margin({ bottom: 6 })
        Text(intentDetail.displayDescription)
          .fontFamily('HarmonyHeiTi')
          .fontSize(14)
          .fontWeight(400)
          .fontColor('#777775')
      }
      .alignItems(HorizontalAlign.Start)
      .margin({ left: 16 })

      Blank()
      Image($r('app.media.ic_arrow'))
        .width(12)
        .height(24)
    }
    .width('100%')
    .height(64)
    .alignItems(VerticalAlign.Center)
  }

  build() {
    Column() {
      Row() {
        Column() {
          Image($r('app.media.ic_arrow_left'))
            .width(8.68)
            .height(16.48)
            .opacity(0.6)
        }
        .width(40)
        .height(40)
        .borderRadius(20)
        .backgroundColor($r('app.color.btn_bg_005'))
        .justifyContent(FlexAlign.Center)
        .onClick(() => {
          this.terminateSelf();
        })

        Text($r('app.string.Cont_title_syyt'))
          .margin({ left: 8 })
          .fontSize(20)
          .fontWeight(700)
          .fontColor($r('app.color.font_color_09'))

      }
      .alignItems(VerticalAlign.Center)
      .width('100%')
      .height(56)
      .padding({
        top: 14.5,
        right: 16,
        left: 16,
        bottom: 14.5
      })
      .position({ top: 0, left: 0 })
      .zIndex(100)
      .backgroundColor($r('app.color.background_cont'))

      Scroll(this.scroller) {
        Column() {
          ForEach(Object.keys(this.groupMaterial), (bundleName: string) => {
            this.IntentBlock(this.getDisplayName(bundleName), this.groupMaterial[bundleName])
          }, (bundleName: string) => bundleName)

        }
      }
      .align(Alignment.TopStart)
      .padding({
        top: 76,
        right: 20,
        left: 20,
        bottom: 20
      })
      .constraintSize({ minHeight: '100%' })
      .align(Alignment.TopStart)
      .edgeEffect(EdgeEffect.Spring)
      .scrollable(ScrollDirection.Vertical)
      .scrollBar(BarState.Off)
      .width('100%')
      .height('100%')
      .backgroundColor($r('app.color.background_cont'))
      .clip(true)
    }
    .width('100%')
    .height('100%')
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM])
    .backgroundColor($r('app.color.background_cont'))
  }
}