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
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import { TargetInfo } from '../data/Model';
import { SingleRowComponent } from './SingleRowComponent';
import { TitleComponent } from './TitileComponent';

const TAG = 'SelectorExtensionAbility.ContentDisplayComponent';

@Component
@Preview
export struct ContentDisplayComponent {
    @StorageProp('currentColorMode') @Watch('onColorModeChange') currentMode: number = ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT;
    @Link @Watch('onTargetInfoChanged') targetInfoArray: Array<TargetInfo>;
    @StorageProp('currentLanguage') currentLang: string = '';
    @State targetInfoArraySplit: Array<Array<TargetInfo>> = [];
    @Prop @Watch('onTargetInfoChanged') panelHeight: number;
    @Consume realHeight: Dimension | PanelHeight;
    @State isOverHeight: boolean = false;
    @State scrollerHeight: number = 0;
    private storage?: LocalStorage;
    private scroller: Scroller = new Scroller();
    @Provide titleHeight: number = 0;
    @Provide singleRowHeight: number = 0;
    @Prop @Watch('onTargetInfoChanged') maxLengthOneLine: number;
    @Provide isChange: boolean = false;

    onColorModeChange() {
        let darkMode = this.currentMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ? false : true;
    }

    aboutToAppear() {
        const context: Context = getContext(this);
        this.titleHeight = 56;
        this.getTargetsToDisplay();
    }

    onTargetInfoChanged() {
        this.isChange = true;
        this.getTargetsToDisplay();
    }

    getTargetsToDisplay() {
        const targetInfoArraySplit: TargetInfo[][] = this.splitArray(this.targetInfoArray)
        this.targetInfoArraySplit = this.fillUpTheGapsForOneLine(targetInfoArraySplit);
        if (this.isChange) {
            this.scrollerHeight = (this.panelHeight - this.titleHeight < this
                .singleRowHeight * this.targetInfoArraySplit.length ? this.panelHeight - this.titleHeight
                : this.singleRowHeight * this.targetInfoArraySplit.length) + 16;
            this.realHeight = this.titleHeight + this.scrollerHeight;
        }
    }

    splitArray(arr: Array<TargetInfo>): Array<Array<TargetInfo>> {
        let arraySplit: TargetInfo[][] = [];
        if (this.maxLengthOneLine === 0) {
            arraySplit.push(arr);
            return arraySplit
        }
        let subArrayQuotient = Math.floor(arr.length / this.maxLengthOneLine);
        if (subArrayQuotient === 0) {
            arraySplit.push(arr);
            return arraySplit
        }
        let index = 0;
        for (index = 0; index < subArrayQuotient; index++) {
            arraySplit.push(arr.slice(index * this.maxLengthOneLine, (index + 1) * this.maxLengthOneLine));
        }
        let subArrayListRemainder = arr.length % this.maxLengthOneLine;
        if (subArrayListRemainder !== 0) {
            arraySplit.push(
                arr.slice(index * this.maxLengthOneLine, index * this.maxLengthOneLine + subArrayListRemainder))
        }
        return arraySplit;
    }

    fillUpTheGapsForOneLine(targetInfoArraySplit: Array<Array<TargetInfo>>): Array<Array<TargetInfo>> {
        const targetInfoArray: Array<TargetInfo> | undefined = targetInfoArraySplit.pop();
        if (targetInfoArray === undefined) {
            return targetInfoArraySplit;
        }
        const length = targetInfoArray.length;
        if (length === this.maxLengthOneLine) {
            targetInfoArraySplit.push(targetInfoArray);
            return targetInfoArraySplit;
        }

        for (let index = 0; index < this.maxLengthOneLine - length; index++) {
            targetInfoArray.push(new TargetInfo(null, null, false));
        }
        targetInfoArraySplit.push(targetInfoArray);
        return targetInfoArraySplit;
    }

    build() {
        Column() {
            Row() {
                TitleComponent({ storage: this.storage })
            }
            .justifyContent(FlexAlign.Start)
            .width('100%')
            .padding({
                left: 16,
                right: 16
            })

            Scroll(this.scroller) {
                Column() {
                    ForEach(this.targetInfoArraySplit, (item: Array<TargetInfo>, index?: number) => {
                        SingleRowComponent({ targetInfoArray: item, storage: this.storage })
                    })
                }
                .padding({
                    left: 8,
                    right: 8,
                    top: 16,
                    bottom: 8
                })
            }
            .constraintSize({ maxHeight: this.panelHeight - this.titleHeight })
            .width('100%')
            .scrollable(ScrollDirection.Vertical)
            .scrollBar(BarState.On)
            .edgeEffect(EdgeEffect.None)
        }
        .alignItems(HorizontalAlign.Start)
        .onAreaChange((oldValue: Area, newValue: Area) => {
            if (oldValue.height !== newValue.height) {
                this.scrollerHeight = 
                    (this.panelHeight - this.titleHeight < 
                    this.singleRowHeight * this.targetInfoArraySplit.length ? this.panelHeight - 
                    this.titleHeight : this.singleRowHeight * this.targetInfoArraySplit.length) + 16;
                this.realHeight = this.titleHeight + this.scrollerHeight;
            }
        })
    }
}