/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_HISYSEVENT_REPORT_H
#define OHOS_ABILITY_RUNTIME_HISYSEVENT_REPORT_H

#include "hisysevent_c.h"
#include <string>

namespace OHOS {
namespace AAFwk {
constexpr uint8_t SYSTEM_PARAM_MAX_LEN = 64;
class HisyseventReport {
public:
    HisyseventReport()
    {
    }
    HisyseventReport(int32_t len)
    {
        if (len > SYSTEM_PARAM_MAX_LEN) {
            return;
        }
        this->params_ = new (std::nothrow) HiSysEventParam[SYSTEM_PARAM_MAX_LEN];
    }
    ~HisyseventReport()
    {
        delete[] params_;
    }

    void InsertParam(const char* name, bool value);
    void InsertParam(const char* name, int8_t value);
    void InsertParam(const char* name, uint8_t value);
    void InsertParam(const char* name, int16_t value);
    void InsertParam(const char* name, uint16_t value);
    void InsertParam(const char* name, int32_t value);
    void InsertParam(const char* name, uint32_t value);
    void InsertParam(const char* name, int64_t value);
    void InsertParam(const char* name, uint64_t value);
    void InsertParam(const char* name, float value);
    void InsertParam(const char* name, double value);
    void InsertParam(const char* name, std::string value);
    void InsertParam(const char* name, char* value);
    void InsertParam(const char* name, const char* value);
    void InsertParam(const char* name, const std::vector<int32_t> &value);
    void InsertParam(const char* name, std::vector<char*> &value);
    void InsertParam(const char* name, const std::vector<uint64_t> &value);
    int32_t Report(const char* domain, const char* event, HiSysEventEventType type);

private:
    void SetParamName(HiSysEventParam& param, const char* name);

    std::vector<std::unique_ptr<char[]>> paramBuffers_;
    HiSysEventParam* params_;
    int32_t length_ = SYSTEM_PARAM_MAX_LEN;
    int32_t pos_ = 0;
};
} // namespace AAFwk
} // namespace OHOS

#endif // #define OHOS_ABILITY_RUNTIME_HISYSEVENT_REPORT_H
