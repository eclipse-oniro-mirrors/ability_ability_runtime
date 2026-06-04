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
 
import { IntentInfo } from './IntentContent';
import { DetailItem } from './DetailItem';
import { common, insightIntent, insightIntentDriver, bundleManager } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { BusinessError } from '@kit.BasicServicesKit';
import { format, jsClone, addArrayParams, removeArrayParams, removeFormData } from './format';

const TAG: string = 'IntentDetailPage';
const DOMAIN: number = 0xFF00;

interface ItemsInfo {
  type: string
}

interface propertiesInfo {
  name: string;
  type: string;
  description?: string;
  enum?: Array<string | number>;
  subItem?: propertiesInfo;
  maxLength?: number;
  default?: number | Resource;
  required?: boolean;
  items?: ItemsInfo;
}

interface FormField {
  key: string;
  fatherKey?: string;
  name: string;
  type: string;
  fatherType?: string;
  inputType: string;
  description?: string;
  options?: Array<string | number>;
  subProperties?: FormField[] | FormField[][];
  maxLength?: number;
  default?: number | Resource;
  value?: string | number;
  index?: number;
  required?: boolean;
}

interface UIExtensionInfo {
  type: string;
  name: string;
}

type ObjVal = string | number | null;
type ObjType = Record<string, object>;
type ArrType = ObjType[];

type FormDataType = Record<string, ObjVal | ObjType | ArrType>;

@Extend(UIExtensionComponent)
function extStyle() {
  .width('100%')
  .height('100%')
  .borderRadius(20)
  .shadow({
    radius: 60,
    color: '#33000000',
    offsetX: 0,
    offsetY: 10
  })
  .clip(true)
}

@Component
export struct IntentDetailPage {
  @Consume('articlePathStack') articlePathStack: NavPathStack;
  @State intentDetail: IntentInfo | null = null;
  @State intentDetailMap: Map<string, string> | null = null;
  @State intentDetailEntries: Array<[string, string | string[] | object]> | [] = [];
  @State intentParameters: object | null = null;
  scroller: Scroller = new Scroller();
  @State required: string[] = [];
  @State formData: FormDataType = {};
  @State properties: Record<string, propertiesInfo> = {};
  @State formFields: FormField[] = [];
  @State abilityName: string = '';
  @State uri: string = '';
  @State formName: string = '';
  @State countArr: number[] = [0];
  @State result: string = '';
  @State isShow: boolean = false;
  @State isShowEntry2: boolean = false;
  @State isShowForm: boolean = false;
  @State want: Want = {
    bundleName: 'com.ohos.tools.myapplication',
    abilityName: 'EntryUIExtAbility',
    moduleName: 'entry',
    parameters: {
      'ability.want.params.uiExtensionType': 'share',
      'ohos.insightIntent.executeParam.name': 'TestIntentNameEntry',
      'ohos.insightIntent.executeParam.param': {}
    }
  };
  inputType: Record<string, string> = {
    'string': 'text',
    'number': 'number',
    'enum': 'select',
    'object': 'object',
    'array': 'array',
  };

  aboutToAppear(): void {
    this.intentDetail = this.articlePathStack.getParamByName('intentDetail')[0] as IntentInfo;
    this.intentParameters = this.intentDetail.parameters ?? null;
    this.intentDetailMap = new Map(Object.entries(this.intentDetail));
    this.intentDetailEntries = Array.from(this.intentDetailMap.entries());

    this.required = (this.getAttribute(this.intentDetail?.parameters, 'required') === '') ? [] :
      this.getAttribute(this.intentDetail?.parameters, 'required') as string[];
    this.abilityName = this.getAttribute(this.intentDetail?.subIntentInfo, 'abilityName') as string;
    if (this.intentDetail?.intentType === '@InsightIntentPage') {
      this.abilityName = this.getAttribute(this.intentDetail?.subIntentInfo, 'uiAbility') as string;
    }
    this.uri = this.getAttribute(this.intentDetail?.subIntentInfo, 'uri') as string;
    if (this.intentParameters) {
      this.properties = this.intentParameters['properties'];
      if (this.properties) {
        this.formFields = this.changeProperties(this.properties);
      }
    }
  }

  onBackPress() {
    let context = AppStorage.get('context') as common.UIAbilityContext;
    context.terminateSelf();
  }

  @Provide InputTypeData: Record<string, InputType> = {
    'string': InputType.Normal,
    'number': InputType.Number
  }

  getAttribute(subInfo: object | undefined, attributeName: string): string | string[] {
    if (subInfo && Object.keys(subInfo).includes(attributeName)) {
      return subInfo[attributeName];
    } else {
      return '';
    }
  }

  async getUIExtensionType(bundleName: string, moduleName: string, abilityName: string): Promise<UIExtensionInfo> {
    hilog.info(DOMAIN, TAG, `getUIExtensionType start: bundleName=${bundleName}, moduleName=${moduleName}, abilityName=${abilityName}`);
    const bundleInfo = await bundleManager.getBundleInfo(
      bundleName,
      bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_HAP_MODULE |
      bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY
    );
    hilog.info(DOMAIN, TAG, `getBundleInfo success: bundleName=${bundleInfo.name}`);
    const hapModulesInfo = bundleInfo.hapModulesInfo;
    if (!hapModulesInfo) {
      const errorMsg = `getUIExtensionType failed: hapModulesInfo is null for bundleName=${bundleName}`;
      hilog.error(DOMAIN, TAG, errorMsg);
      throw new Error(errorMsg);
    }

    for (let i = 0; i < hapModulesInfo.length; i++) {
      const moduleInfo = hapModulesInfo[i];
      if (moduleInfo.name === moduleName) {
        const extensionAbilitiesInfo = moduleInfo.extensionAbilitiesInfo;
        if (!extensionAbilitiesInfo) {
          hilog.warn(DOMAIN, TAG, `extensionAbilitiesInfo is null for module: ${moduleName}`);
          continue;
        }

        for (let j = 0; j < extensionAbilitiesInfo.length; j++) {
          const extensionInfo = extensionAbilitiesInfo[j];
          const typeName = extensionInfo.extensionAbilityTypeName;
          if (typeName && typeName !== '') {
            hilog.info(DOMAIN, TAG, `getUIExtensionType success: found UIExtensionAbility ${extensionInfo.name}, type=${typeName}`);
            const result: UIExtensionInfo = {
              type: typeName,
              name: extensionInfo.name
            };
            return result;
          }
        }
      }
    }
    const errorMsg = `getUIExtensionType failed: UIExtensionAbility not found for bundleName=${bundleName}, moduleName=${moduleName}, abilityName=${abilityName}`;
    hilog.error(DOMAIN, TAG, errorMsg);
    throw new Error(errorMsg);
  }

  changeProperties(properties: Record<string, propertiesInfo>, fatherKey?: string, fatherType?: string) {
    const formArr: FormField[] = [];
    Object.keys(properties).forEach((propKey: string, index: number) => {
      const obj = properties[propKey];
      let key = propKey;
      if (fatherKey) {
        if (fatherType && fatherType === 'array') {
          key = `${fatherKey}.array0.${propKey}`;
        } else {
          key = `${fatherKey}.${propKey}`;
        }
      }
      const name = propKey;
      const type = obj.type;
      const inputType = obj.enum ? 'select' : this.inputType[type];
      let subItem: FormField[] | FormField[][] = [];
      if (type === 'object' && Object.keys(obj).includes('properties')) {
        const subProperties = this.findProperties(obj);
        subItem = this.changeProperties(subProperties, propKey) as FormField[];
      } else if (type === 'array' && Object.keys(obj).includes('items')) {
        if (index === 0) {
          this.countArr = [0];
        }
        let subProperties = this.findProperties(obj);

        if (Object.keys(subProperties).length === 0) {
          subProperties = { 'items':  { name: '', type: obj.items?.type ?? 'string'  } as propertiesInfo };
        }

        subItem = [this.changeProperties(subProperties, propKey, 'array')] as FormField[][];
      }
      const description = obj.description;
      const formObj: FormField = {
        key,
        name,
        type,
        inputType,
        description,
      };

      if (obj.enum) {
        formObj.options = obj.enum;
      }
      if (obj.maxLength) {
        formObj.maxLength = obj.maxLength;
      }
      if (obj.default) {
        formObj.default = obj.default;
      }
      if (fatherKey) {
        formObj.fatherKey = fatherKey;
      }
      if (fatherType) {
        formObj.fatherType = fatherType;
        formObj.index = 0;
      }
      if (this.required?.includes(propKey)) {
        formObj.required = true;
      }
      if (subItem.length > 0) {
        if (fatherType && fatherType === 'array') {
          formObj.subProperties = subItem;
        } else {
          formObj.subProperties = subItem;
        }
      }
      formArr.push(formObj);
    })
    return formArr;
  }

  findProperties(obj: object): Record<string, propertiesInfo> {
    let prop: Record<string, propertiesInfo> = {};
    Object.keys(obj).forEach((key: string) => {
      if (key === 'properties') {
        prop = obj['properties'];
      } else if (typeof obj[key] === 'object' && key !== 'propertyNames') {
        prop = this.findProperties(obj[key]);
      }
    })
    return prop;
  }

  async executeLinkIntent(insightIntentParam: Record<string, Object>) {

    let param: insightIntentDriver.ExecuteParam = {
      bundleName: this.intentDetail?.bundleName ?? '',
      moduleName: this.intentDetail?.moduleName ?? '',
      insightIntentName: this.intentDetail?.intentName ?? '',
      insightIntentParam: insightIntentParam,
      executeMode: insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND,
      abilityName: this.abilityName
    };

    if (this.intentDetail?.intentType === '@InsightIntentLink') {
      param.uris = [this.uri];
    }
    if (this.intentDetail?.intentType === '@InsightIntentLink' ||
      this.intentDetail?.intentType === '@InsightIntentPage') {
      param.executeMode = insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND;
    }
    if (this.intentDetail?.intentType === '@InsightIntentFunctionMethod') {
      param.executeMode = insightIntent.ExecuteMode.UI_ABILITY_BACKGROUND;
    }
    if (this.intentDetail?.intentType === '@InsightIntentEntry') {
      param.executeMode = this.intentDetail.subIntentInfo['executeMode'][0];
      hilog.info(DOMAIN, TAG, `InsightIntentEntry detected: intentName=${this.intentDetail.intentName}, executeMode=${param.executeMode}, subIntentInfo=${JSON.stringify(this.intentDetail.subIntentInfo)}`);
    }

    if (this.intentDetail?.intentType === '@InsightIntentEntry' &&
    this.intentDetail.subIntentInfo['executeMode'] && this.intentDetail.subIntentInfo['executeMode'][0] === 2) {
      const uiExtensionInfo = await this.getUIExtensionType(
        param.bundleName,
        param.moduleName,
        param.abilityName
      );
      hilog.info(DOMAIN, TAG, `IntentName: ${this.intentDetail.intentName}, uiExtensionType: ${uiExtensionInfo.type}, uiExtensionName: ${uiExtensionInfo.name}`);
      this.want = {
        bundleName: param.bundleName,
        abilityName: param.abilityName,
        moduleName: param.moduleName,
        parameters: {
          'ability.want.params.uiExtensionType': uiExtensionInfo.type,
          'ohos.insightIntent.executeParam.name': this.intentDetail.intentName,
          'ohos.insightIntent.executeParam.param': param.insightIntentParam,
        }
      };   
      hilog.info(DOMAIN, TAG, `UIExtensionComponent want: ${JSON.stringify(this.want)}`);
      this.isShowEntry2 = true;
      this.isShow = true;
    } else if (this.intentDetail?.intentType === '@InsightIntentForm') {
      this.formName = this.getAttribute(this.intentDetail?.subIntentInfo, 'formName') as string;
      hilog.info(DOMAIN, TAG, `FormComponent: formName=${this.formName}`);
      this.isShow = true;
      this.isShowForm = true;
    } else {
      try {
        let resultData: insightIntent.ExecuteResult = await insightIntentDriver.execute(param);
        this.result = 'code: ' + resultData.code + '\n' + 'result: ' + JSON.stringify(resultData?.result, null, 2);
        if (this.result != '') {
          this.isShow = true;
        }
      } catch (error) {
        this.result = 'code: ' + JSON.stringify(error.code) + '\n' + JSON.stringify(param, null, 2);
        if (this.result != '') {
          this.isShow = true;
        }
      }
    }
  }

  submitForm() {
    const executeParam = format(this.formData);
    let validate: boolean = true;
    this.required?.forEach((key: string) => {
      if (executeParam[key] === undefined || executeParam[key] === null) {
        if (validate) {
          validate = false;
          hilog.error(DOMAIN, TAG, `submitForm validation failed: required field '${key}' is missing`);
        }
      }
    })
    if (validate) {
      this.executeLinkIntent(executeParam as Record<string, Object>);
    }
  }

  @Builder
  resultModule() {
    Column() {
      if (this.isShowEntry2) {
        Column() {
          UIExtensionComponent(this.want)
            .extStyle()
        }
        .height('100%')
        .width('100%')
        .padding({
          top: 28,
          left: 22,
          right: 22,
          bottom: 48
        })
      } else if (this.isShowForm) {
        Column() {
          FormComponent({
            id: '0',
            name: this.formName,
            bundle: this.intentDetail?.bundleName,
            ability: this.abilityName,
            module: this.intentDetail?.moduleName,
          })
            .width('100%')
            .height('80%')
            .borderRadius(20)
            .shadow({
              radius: 60,
              color: '#33000000',
              offsetX: 0,
              offsetY: 10
            })
            .clip(true)
        }
        .padding({
          top: 28,
          left: 22,
          right: 22,
          bottom: 48
        })
        .height('100%')
        .width('100%')
      } else {
        Text(this.result)
          .width('100%')
          .textAlign(TextAlign.Start)
          .fontSize(14)
          .fontColor($r('app.color.font_color_09'))
          .lineHeight(18)
          .padding({
            top: 20,
            left: 28,
            right: 28,
            bottom: 48
          })
      }
    }
    .width('100%')
    .height('100%')
  }

  @Builder
  buildTextInput(field: FormField) {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Start }) {
      Text(field.name)
        .fontSize(16)
        .fontColor($r('app.color.font_color_04'))
        .width(165)
        .height(40)
        .borderRadius(20)
        .padding({ right: 16, left: 16 })
        .margin({ right: 8 })
        .backgroundColor($r('app.color.background_input'))
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .maxLines(1)
      Column() {
        TextInput({
          placeholder: `${field.type} ${field && field.description ? '，' + field.description : ''}`,
          text: `${this.formData[field.key] ?? ''}`
        })
          .maxLength(field.maxLength)
          .width('100%')
          .height(40)
          .fontSize(16)
          .fontColor($r('app.color.font_color_09'))
          .padding({ right: 16, left: 16 })
          .backgroundColor($r('app.color.background_input'))
          .onChange((value: string) => {
            this.formData[field.key] = value;
          })

        if (field.required) {
          Row() {
            Text($r('app.string.Tips_bt'))
              .fontSize(12)
              .lineHeight(16)
              .fontColor($r('app.color.error_color'))
              .padding({ left: 10 })
          }
          .width('100%')
          .padding({ top: 5, right: 10 })
        }
      }
      .flexGrow(1)
    }
    .margin({ bottom: 12 })
  }

  @Builder
  buildNumberInput(field: FormField) {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Start }) {
      Text(field.name)
        .fontSize(16)
        .fontColor($r('app.color.font_color_04'))
        .width(165)
        .height(40)
        .borderRadius(20)
        .padding({ right: 16, left: 16 })
        .margin({ right: 8 })
        .backgroundColor($r('app.color.background_input'))
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .maxLines(1)
      Column() {
        TextInput({
          placeholder: `${field.type} ${field && field.description ? '，' + field.description : ''}`,
          text: `${this.formData[field.key] ?? ''}`
        })
          .maxLength(field.maxLength)
          .type(InputType.Number)
          .width('100%')
          .height(40)
          .fontSize(16)
          .fontColor($r('app.color.font_color_09'))
          .padding({ right: 16, left: 16 })
          .backgroundColor($r('app.color.background_input'))
          .onChange((value: string) => {
            this.formData[field.key] = parseInt(value);
          })

        if (field.required) {
          Row() {
            Text($r('app.string.Tips_bt'))
              .fontSize(12)
              .lineHeight(16)
              .fontColor($r('app.color.error_color'))
              .padding({ left: 10 })
          }
          .width('100%')
          .padding({ top: 5, right: 10 })
        }
      }
      .flexGrow(1)
    }
    .height(50)
    .margin({ bottom: 15 })
  }

  @Builder
  buildSelectInput(field: FormField) {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Start }) {
      Text(field.name)
        .fontSize(16)
        .fontColor($r('app.color.font_color_04'))
        .width(165)
        .height(40)
        .borderRadius(20)
        .padding({ right: 16, left: 16 })
        .margin({ right: 8 })
        .backgroundColor($r('app.color.background_input'))
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .maxLines(1)
      Column() {
        Select(field.options?.map(option => {
          const opt: SelectOption = { value: option.toString() };
          return opt;
        }))
          .width('100%')
          .height(40)
          .font({ size: 16, weight: 400 })
          .fontColor($r('app.color.font_color_09'))
          .padding({ right: 6, left: 8 })
          .backgroundColor($r('app.color.background_input'))
          .optionWidth(224)
          .optionFont({ size: 16, weight: 500 })
          .optionFontColor($r('app.color.font_color_09'))
          .onSelect((index: number, value: string) => {
            const selVal = field.type === 'string' ? value : parseInt(value);
            this.formData[field.key] = selVal;
          })
        Row() {
          if (field.required) {
            Text($r('app.string.Tips_bt'))
              .fontSize(12)
              .lineHeight(16)
              .fontColor($r('app.color.error_color'))
              .padding({ left: 10 })
          }
          Text(field.description)
            .fontSize(12)
            .lineHeight(16)
            .fontColor($r('app.color.font_color_04'))
            .padding({ left: 10 })
        }
        .width('100%')
        .padding({ top: 5, right: 10 })
      }
      .flexGrow(1)
    }
    .margin({ bottom: 20 })
  }

  @Builder
  buildFormFields(formFields: FormField[] | FormField[][]) {
    Column() {
      ForEach(formFields, (field: FormField, index: number) => {
        if (field.inputType === 'text') {
          this.buildTextInput(field)
        } else if (field.inputType === 'number') {
          this.buildNumberInput(field)
        } else if (field.inputType === 'select') {
          this.buildSelectInput(field)
        } else if (field.inputType === 'object') {
          if (field.inputType === 'object' && !field.subProperties) {
            this.buildTextInput(field)
          } else {
            Column() {
              Row() {
                Text(field.name)
                  .fontSize(14)
                  .lineHeight(20)
                  .fontWeight(500)
                  .fontColor($r('app.color.font_color_06'))
                  .width('100%')
                  .padding({ left: 12 })
              }
              .margin({ bottom: 8, top: 16 })

              this.buildFormFields(field.subProperties as FormField[])
            }
          }
        } else if (field.inputType === 'array') {
          ForEach(field.subProperties, (fieldItem: FormField[], arrIndex: number) => {
            Column() {
              Flex({
                direction: FlexDirection.Row,
                justifyContent: FlexAlign.SpaceBetween,
                alignItems: ItemAlign.Center
              }) {
                Text(field.name)
                  .fontSize(14)
                  .fontWeight(500)
                  .fontColor($r('app.color.font_color_06'))
                  .width('100%')
                  .lineHeight(20)
                if (arrIndex === 0) {
                  Text($r('app.string.Btn_tj'))
                    .fontSize(14)
                    .fontColor('rgb(10,89,247)')
                    .fontWeight(500)
                    .lineHeight(19)
                    .onClick(() => {
                      if (field.maxLength && field.subProperties && (field.subProperties?.length >= field.maxLength)) {

                        this.getUIContext().showAlertDialog(
                          {
                            message: `最大节点数不超过${field.maxLength}个`,
                            autoCancel: true,
                            alignment: DialogAlignment.Center,
                            offset: { dx: 0, dy: -20 },
                            gridCount: 3,
                            confirm: {
                              value: $r('app.string.Btn_qr'),
                              action: () => {
                                console.info('Button-clicking callback');
                              }
                            },
                            cancel: () => {
                              console.info('Closed callbacks');
                            }
                          }
                        )
                      } else {
                        this.countArr.push(this.countArr.length);
                        const arrItem: FormField[][] = addArrayParams(field.subProperties, field.name, this.countArr);
                        let fieldArr = jsClone(this.formFields) as FormField[];
                        fieldArr[index].subProperties = arrItem;
                        this.formFields = fieldArr;
                      }
                    })
                } else {
                  Text($r('app.string.Btn_sc'))
                    .fontSize(14)
                    .fontColor('rgb(10,89,247)')
                    .fontWeight(500)
                    .lineHeight(19)
                    .onClick(() => {
                      const newFormData: FormDataType = removeFormData(this.formData, field.name, fieldItem[0].index);
                      this.formData = newFormData;
                      const arrItem: FormField[][] = removeArrayParams(field.subProperties, field.name, arrIndex);
                      let fieldArr = jsClone(this.formFields) as FormField[];
                      fieldArr[index].subProperties = arrItem;
                      this.formFields = fieldArr;
                    })
                }
              }
              .margin({
                top: 16,
                bottom: 8,
                left: 12,
                right: 12
              })

              this.buildFormFields(fieldItem as FormField[])
            }
          })
        }
      })
    }
  }

  build() {
    NavDestination() {
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

        Text($r('app.string.Detail_title_ytts'))
          .margin({ left: 8 })
          .fontSize(20)
          .fontWeight(700)
          .fontColor($r('app.color.font_color_09'))
      }
      .width('100%')
      .height(56)
      .padding({
        top: 14.5,
        right: 16,
        left: 16,
        bottom: 14.5
      })
      .backgroundColor($r('app.color.background_cont'))
      .position({ top: 0, left: 0 })
      .zIndex(100)
      .onClick(() => {
        this.articlePathStack.pop();
      })

      Scroll(this.scroller) {
        Column() {
          DetailItem({ intentDetail: this.intentDetail })

          Column() {
            Text($r('app.string.Detail_subtitle_ytcs'))
              .fontFamily('HarmonyHeiTi-Medium')
              .fontSize(18)
              .fontWeight(700)
              .fontColor($r('app.color.font_color_09'))
              .lineHeight(24)
              .padding({
                top: 24,
                right: 16,
                bottom: 8,
                left: 16
              })
              .margin({ bottom: 12 })
              .textAlign(TextAlign.Start)
              .width('100%')

            if (this.intentParameters !== null) {
              ForEach(this.formFields, (field: FormField, index: number) => {
                if (field.inputType === 'text') {
                  this.buildTextInput(field)
                } else if (field.inputType === 'number') {
                  this.buildNumberInput(field)
                } else if (field.inputType === 'select') {
                  this.buildSelectInput(field)
                } else if (field.inputType === 'object') {
                  if (field.inputType === 'object' && !field.subProperties) {
                    this.buildTextInput(field)
                  } else {
                    Column() {
                      Row() {
                        Text(field.name)
                          .fontSize(14)
                          .lineHeight(20)
                          .fontWeight(500)
                          .fontColor($r('app.color.font_color_06'))
                          .width('100%')
                          .padding({ left: 12 })
                      }
                      .margin({ bottom: 8, top: 16 })

                      this.buildFormFields(field.subProperties as FormField[])
                    }
                  }
                } else if (field.inputType === 'array') {
                  ForEach(field.subProperties, (fieldItem: FormField[], arrIndex: number) => {
                    Column() {
                      Flex({
                        direction: FlexDirection.Row,
                        justifyContent: FlexAlign.SpaceBetween,
                        alignItems: ItemAlign.Center
                      }) {
                        Text(field.name)
                          .fontSize(14)
                          .fontColor($r('app.color.font_color_06'))
                          .width('50%')
                          .lineHeight(20)
                          .fontWeight(500)
                        if (arrIndex === 0) {
                          Text($r('app.string.Btn_tj'))
                            .fontSize(14)
                            .fontColor('rgb(10,89,247)')
                            .fontWeight(500)
                            .lineHeight(19)
                            .onClick(() => {
                              if (field.maxLength && field.subProperties &&
                                (field.subProperties?.length >= field.maxLength)) {

                                this.getUIContext().showAlertDialog(
                                  {
                                    message: `最大节点不超过${field.maxLength}个`,
                                    autoCancel: true,
                                    alignment: DialogAlignment.Center,
                                    offset: { dx: 0, dy: -20 },
                                    gridCount: 3,
                                    confirm: {
                                      value: $r('app.string.Btn_qr'),
                                      action: () => {
                                        console.info('Button-clicking callback');
                                      }
                                    },
                                    cancel: () => {
                                      console.info('Closed callbacks');
                                    }
                                  }
                                )
                              } else {
                                this.countArr.push(this.countArr.length)
                                const arrItem: FormField[][] =
                                  addArrayParams(field.subProperties, field.name, this.countArr);
                                let fieldArr = jsClone(this.formFields) as FormField[];
                                fieldArr[index].subProperties = arrItem;
                                this.formFields = fieldArr;
                              }
                            })
                        } else {
                          Text($r('app.string.Btn_sc'))
                            .fontSize(14)
                            .fontColor('rgb(10,89,247)')
                            .fontWeight(500)
                            .lineHeight(19)
                            .onClick(() => {
                              const newFormData: FormDataType =
                                removeFormData(this.formData, field.name, fieldItem[0].index);
                              this.formData = newFormData;
                              const arrItem: FormField[][] =
                                removeArrayParams(field.subProperties, field.name, arrIndex);
                              let fieldArr = jsClone(this.formFields) as FormField[];
                              fieldArr[index].subProperties = arrItem;
                              this.formFields = fieldArr;
                            })
                        }
                      }
                      .margin({
                        top: 16,
                        bottom: 8,
                        left: 12,
                        right: 12
                      })

                      this.buildFormFields(fieldItem as FormField[])
                    }
                  })
                }
              })
            }

          }
          .padding({ bottom: 86 })
          .margin({ left: 16, right: 16 })
        }
        .width('100%')
        .alignItems(HorizontalAlign.Start)
      }
      .align(Alignment.TopStart)
      .constraintSize({ minHeight: '100%' })
      .edgeEffect(EdgeEffect.Spring)
      .scrollable(ScrollDirection.Vertical)
      .scrollBar(BarState.Off)
      .backgroundColor($r('app.color.background_cont'))
      .width('100%')
      .height('100%')
      .padding({ top: 56, bottom: 0 })
      .clip(true)

      Row() {
        Button($r('app.string.Detail_btn_zxyt'), { stateEffect: true })
          .width('100%')
          .fontSize(16)
          .fontWeight(500)
          .backgroundColor($r('app.color.btn_background'))
          .borderRadius(20)
          .height(40)
          .onClick(() => {
            this.submitForm();
          })
          .bindSheet($$this.isShow, this.resultModule(), {
            height: SheetSize.MEDIUM,
            blurStyle: BlurStyle.Thick,
            title: { title: $r('app.string.Detail_stitle_zxjg') },
            preferType: SheetType.BOTTOM,
          })
      }
      .width('100%')
      .padding({
        top: 16,
        left: 16,
        right: 16,
        bottom: 6
      })
      .position({ bottom: 0, left: 0 })
      .backgroundColor($r('app.color.background_cont'))
    }
    .hideTitleBar(true)
    .width('100%')
    .height('100%')
    .backgroundColor($r('app.color.background_cont'))
    .clip(true)
  }
}