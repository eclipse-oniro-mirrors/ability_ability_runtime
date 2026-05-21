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

#ifndef OHOS_AAFWK_HILOG_TAG_WRAPPER_H
#define OHOS_AAFWK_HILOG_TAG_WRAPPER_H

namespace OHOS::AAFwk {
enum class AAFwkLogTag : uint32_t {
    DEFAULT = 0xD001300,
};
}

using AAFwkTag = OHOS::AAFwk::AAFwkLogTag;

#define TAG_LOGD(tag, fmt, ...) ((void)0)
#define TAG_LOGI(tag, fmt, ...) ((void)0)
#define TAG_LOGW(tag, fmt, ...) ((void)0)
#define TAG_LOGE(tag, fmt, ...) ((void)0)
#define TAG_LOGF(tag, fmt, ...) ((void)0)

#endif
