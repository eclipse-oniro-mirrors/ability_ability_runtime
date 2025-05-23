/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

interface SwiperPhone {
  height: string,
  indicator: boolean,
  contentHeight: string,
  gridColumns: string,
  gridRows: string
}

interface SelectorBtn {
  marginTop:string,
}

interface SelectorPhone {
  swiper:SwiperPhone,
  btn: SelectorBtn
}

interface HapInfo {
  label: string,
  icon: boolean,
  bundle: string,
  ability: string,
  module: string
}

class SelectorDataSource implements IDataSource {
  private list: string[] = []
  private listener: DataChangeListener

  constructor(list: string[]) {
    this.list = list
  }

  totalCount(): number {
    return this.list.length
  }

  getData(index: number): string {
    return this.list[index]
  }

  registerDataChangeListener(listener: DataChangeListener): void {
    this.listener = listener
  }

  unregisterDataChangeListener() {
  }
}


@Entry
@Component
struct SelectorPhoneDialog {
  private phoneShowData: SelectorDataSource = new SelectorDataSource([])
  @StorageLink('oversizeHeight') oversizeHeight: string = 'false';
  @State private selector: SelectorPhone = {
    swiper: {
      height: '100vp',
      indicator: false,
      contentHeight: '100vp',
      gridColumns: '1fr 1fr',
      gridRows: '1fr',
    },
    btn: {
      marginTop: '20vp',
    }
  }
  @State private hapList: HapInfo[] = [];
  private TAG = 'SelectorDialog_Phone_Page';

  @Builder
  elementIconCombine(iconSrc: string, appIndex: string) {
    RelativeContainer() {
      Image(iconSrc)
        .id('iconLab')
        .height(60)
        .width(60)
        .alignSelf(ItemAlign.Center);
      if (appIndex !== '0') {
        Image($r('app.media.app_icon_clone_index_' + appIndex))
          .height(18)
          .width(18)
          .alignRules({
            end: { anchor: 'iconLab', align: HorizontalAlign.End },
            bottom: { anchor: 'iconLab', align: VerticalAlign.Bottom }
          })
      }
    }
    .height(60)
    .width(60)
    .alignSelf(ItemAlign.Center);
  } 

  aboutToAppear(): void {
    console.log(this.TAG, 'dialog page appears');
    this.hapList = globalThis.params.hapList;
    this.getHapListStyle();
  }

  onPageShow() {
    console.info(this.TAG, 'onPageShow called');
    this.getHapListStyle();
  }

  onSelectApp(item) {
    console.log(this.TAG, 'onSelectApp - start ');
    globalThis.abilityWant.bundleName = item.split('#')[0];
    globalThis.abilityWant.abilityName = item.split('#')[1];
    globalThis.abilityWant.moduleName = item.split('#')[4];
    globalThis.abilityWant.parameters['ohos.extra.param.key.appCloneIndex'] = Number(item.split('#')[7]);
    globalThis.selectExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (data, error) => {
      if (error) {
        console.error(this.TAG + ' startAbility finish, error: ' + JSON.stringify(error));
        return;
      }
      console.log(this.TAG + ' startAbility finish, data: ' + JSON.stringify(data));
      globalThis.selectExtensionContext.terminateSelf();
    });
  }

  getHapListStyle() {
    if (this.oversizeHeight === 'false') {
      if (this.hapList.length > 8) {
        this.selector.swiper.height = '220vp';
        this.selector.swiper.indicator = true;
        this.selector.swiper.contentHeight = '200vp';
        this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
        this.selector.swiper.gridRows = '1fr 1fr';
        this.selector.btn.marginTop = '10vp';
      } else if (this.hapList.length > 4) {
        this.selector.swiper.height = '200vp';
        this.selector.swiper.contentHeight = '200vp';
        this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
        this.selector.swiper.gridRows = '1fr 1fr';
      } else if (this.hapList.length > 3) {
        this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
      } else if (this.hapList.length > 2) {
        this.selector.swiper.gridColumns = '1fr 1fr 1fr';
      } else {
        ;
      }
      this.phoneShowData = new SelectorDataSource(globalThis.phoneShowHapList);
    } else {
      this.selector.swiper.gridColumns = '1fr 1fr 1fr 1fr';
      this.selector.swiper.gridRows = '1fr';
      this.phoneShowData = new SelectorDataSource(globalThis.signalRowPhoneShowHapList);
    }
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center }) {
      Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Start }) {
        Text($r('app.string.message_title_selector'))
          .fontSize(22)
          .fontWeight(FontWeight.Medium)
          .textAlign(TextAlign.Start)
          .margin({ top: 25, left: 20 })
      }
      .height(100)

      Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center }) {
        Swiper() {
          LazyForEach(this.phoneShowData, (item: any) => {
            Grid() {
              ForEach(item, (item: any) => {
                GridItem() {
                  Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center }) {
                    if (item.split('#')[3] != '') {
                      if (item.split('#')[3].indexOf('image/json') > -1) {
                        this.elementIconCombine(globalThis.jsonIconMap.get(item.split('#')[0] + ':' +
                          item.split('#')[4] + ':' + item.split('#')[1] + ':' + item.split('#')[7]),
                          item.split('#')[7]);
                      } else {
                        this.elementIconCombine(item.split('#')[3], item.split('#')[7]);
                      }
                    } else {
                      Image($r('app.media.app_icon'))
                        .height(60)
                        .width(60)
                        .alignSelf(ItemAlign.Center);
                    }
                    if (item.split('#')[2] != '') {
                      Text(item.split('#')[2])
                        .fontSize(16)
                        .width(80)
                        .height(23)
                        .textAlign(TextAlign.Center)
                        .alignSelf(ItemAlign.Center)
                        .textOverflow({ overflow: TextOverflow.Ellipsis })
                    } else {
                      Text(item.split('#')[0])
                        .fontSize(16)
                        .width(80)
                        .height(23)
                        .textAlign(TextAlign.Center)
                        .alignSelf(ItemAlign.Center)
                        .textOverflow({ overflow: TextOverflow.Ellipsis })
                    }
                  }
                  .height(180)
                  .onClick(() => {
                    this.onSelectApp(item)
                  })
                }
                .margin({ top: 10 })
                .align(Alignment.Center)
              }, item => item)
            }
            .columnsTemplate(this.selector.swiper.gridColumns)
            .rowsTemplate(this.selector.swiper.gridRows)
            .columnsGap(12)
            .rowsGap(12)
            .margin({ top: 10 })
            .height(300)
          }, item => item)
        }
        .indicatorStyle({ color: '#bebdc0', selectedColor: '#ff326Ce9', size: 4 })
        .align(Alignment.Center)
        .width('90%')
        .indicator(this.phoneShowData.totalCount() > 1)
      }

      Flex({ alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        Text(($r('app.string.message_cancel_selector')))
          .fontSize(20)
          .fontColor('#0A59F7')
          .fontWeight(FontWeight.Regular)
      }
      .width('90%')
      .height(100)
      .backgroundColor('#ffffff')
      .borderRadius(20)
      .margin({ bottom: 20, top: 10 })
      .onClick(() => {
        globalThis.selectExtensionContext.terminateSelf();
      })
    }
    .borderRadius(24)
    .borderWidth(1)
    .align(Alignment.Center)
    .borderColor('#e9e9e9')
    .backgroundColor('#ffffff')
    .width('100%')
    .height('100%')
  }
}