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
#include "appcapture_perf.h"
 
#include <mutex>
#include <algorithm>
#include <string>
#include <sstream>
 
#include "hisysevent_report.h"
#include "lperf.h"
#include "hilog_tag_wrapper.h"
 
namespace OHOS {
namespace AppExecFwk {
const int32_t CAPTURE_DURATION = 1000;
const int32_t FREQ = 100;
const int32_t ERROR = -1;
const int32_t NO_ERROR = 0;

AppCapturePerf &AppCapturePerf::GetInstance()
{
    static AppCapturePerf instance_;
    return instance_;
}
 
std::vector<std::string> AppCapturePerf::SplitStr(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
 
int32_t AppCapturePerf::CapturePerf(const FaultData &faultData)
{
    std::lock_guard<std::mutex> lock(singletonMutex_);

    int64_t perfId = 0;
    auto ret = std::from_chars(faultData.timeoutMarkers.c_str(),
        faultData.timeoutMarkers.c_str() + faultData.timeoutMarkers.size(), perfId);
    if (ret.ec != std::errc()) {
        TAG_LOGE(AAFwkTag::APPDFR, "perfId stoi(%{public}s) failed", faultData.timeoutMarkers.c_str());
        return ERROR;
    }

    std::vector<int32_t> tids;
    std::vector<std::string> threads = SplitStr(faultData.errorObject.stack, ',');
    for (uint32_t i = 0; i < threads.size(); i++) {
        if (threads[i] == "") {
            continue;
        }
        int32_t tid = -1;
        auto res = std::from_chars(threads[i].c_str(), threads[i].c_str() + threads[i].size(), tid);
        if (res.ec != std::errc()) {
            TAG_LOGE(AAFwkTag::APPDFR, "tid conversion failed");
        }
        tids.push_back(tid);
    }
    if (tids.empty()) {
        TAG_LOGE(AAFwkTag::APPDFR, "No valid thread IDs found");
        return ERROR;
    }
    int res = 0;
    auto &instance = Developtools::HiPerf::HiPerfLocal::Lperf::GetInstance();
    res = instance.StartProcessStackSampling(tids, FREQ, CAPTURE_DURATION, false);
    std::vector<std::string> perf;
    for (uint32_t i = 0; i < tids.size(); i++) {
        std::string info;
        res = instance.CollectSampleStackByTid(tids[i], info);
        if (res != 0) {
            perf.push_back("");
            continue;
        }
        perf.push_back(info);
    }
    instance.FinishProcessStackSampling();
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(4);
    hisyseventReport->InsertParam("APP_NAME", faultData.errorObject.name);
    hisyseventReport->InsertParam("TIDS", tids);
    std::vector<char*> list;
    list.reserve(perf.size());
    for (const auto& s : perf) {
        list.emplace_back(const_cast<char *>(s.c_str()));
    }
    hisyseventReport->InsertParam("PERF", list);
    hisyseventReport->InsertParam("PERFID", perfId);
    hisyseventReport->Report("AAFWK", "CPU_LOAD_CAPTURE_STACK", HISYSEVENT_STATISTIC);
    return NO_ERROR;
}

}  // namespace AppExecFwk
}  // namespace OHOS