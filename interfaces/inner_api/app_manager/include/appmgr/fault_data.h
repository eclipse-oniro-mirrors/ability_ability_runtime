/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FAULT_DATA_H
#define OHOS_ABILITY_RUNTIME_FAULT_DATA_H

#include <string>

#include "ierror_observer.h"
#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @enum FaultDataType
 * FaultDataType defines the type of FaultData.
 */
enum class FaultDataType {
    UNKNOWN = -1,
    CPP_CRASH,
    JS_ERROR,
    CJ_ERROR,
    APP_FREEZE,
    CPU_LOAD,
    SLEEP_CLEAN,
    PERFORMANCE_CONTROL,
    RESOURCE_CONTROL
};

enum AppStatus {
    APP_STATUS_UNKNOW = -1,
    APP_STATUS_FOREGROUND = 0,
    APP_STATUS_BACKGROUND = 1
};

class AppFreezeType {
public:
    static constexpr char LIFECYCLE_HALF_TIMEOUT[] = "LIFECYCLE_HALF_TIMEOUT";
    static constexpr char LIFECYCLE_HALF_TIMEOUT_WARNING[] = "LIFECYCLE_HALF_TIMEOUT_WARNING";
    static constexpr char LIFECYCLE_TIMEOUT[] = "LIFECYCLE_TIMEOUT";
    static constexpr char LIFECYCLE_TIMEOUT_WARNING[] = "LIFECYCLE_TIMEOUT_WARNING";
    static constexpr char APP_LIFECYCLE_TIMEOUT[] = "APP_LIFECYCLE_TIMEOUT";
    static constexpr char THREAD_BLOCK_3S[] = "THREAD_BLOCK_3S";
    static constexpr char THREAD_BLOCK_6S[] = "THREAD_BLOCK_6S";
    static constexpr char APP_INPUT_BLOCK[] = "APP_INPUT_BLOCK";
    static constexpr char BUSSINESS_THREAD_BLOCK_3S[] = "BUSSINESS_THREAD_BLOCK_3S";
    static constexpr char BUSSINESS_THREAD_BLOCK_6S[] = "BUSSINESS_THREAD_BLOCK_6S";
    static constexpr char BG_FREEZE_WARNING[] = "BG_FREEZE_WARNING";
    static constexpr char BUSINESS_INPUT_BLOCK[] = "BUSINESS_INPUT_BLOCK";
};
/**
 * @struct FaultData
 * FaultData is used to save information about faultdata.
 */
struct FaultData : public Parcelable {
    bool ReadFromParcel(Parcel &parcel);
    bool ReadContent(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    bool WriteContent(Parcel &parcel) const;
    static FaultData *Unmarshalling(Parcel &parcel);
    // error object
    ErrorObject errorObject;
    FaultDataType faultType = FaultDataType::UNKNOWN;
    std::string timeoutMarkers;
    bool waitSaveState = false;
    bool notifyApp = false;
    bool forceExit = false;
    bool needKillProcess = true;
    uint32_t state = 0;
    int32_t eventId = -1;
    uint64_t schedTime = 0;
    uint64_t detectTime = 0;
    int32_t pid = -1;
    int32_t tid = -1;
    uint32_t stuckTimeout = 0;
    int32_t appStatus = -1;
    uint64_t samplerStartTime = 0;
    uint64_t samplerFinishTime = 0;
    int32_t samplerCount = -1;
    sptr<IRemoteObject> token = nullptr;
    std::string appfreezeInfo;
    std::string appRunningUniqueId;
    std::string procStatm;
    bool isInForeground;
    bool isEnableMainThreadSample;
    std::string applicationHeapInfo;
    std::string processLifeTime;
    int32_t markedId = 0;
    int32_t processedId = 0;
    int32_t dispatchedEventId = 0;
};

/**
 * @struct AppFaultDataBySA
 * AppFaultDataBySA is used to save information about faultdata notified by SA.
 */
struct AppFaultDataBySA : public Parcelable {
    bool ReadFromParcel(Parcel &parcel);
    bool ReadContent(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    bool WriteErrorObject(Parcel &parcel) const;
    bool WriteContent(Parcel &parcel) const;
    static AppFaultDataBySA *Unmarshalling(Parcel &parcel);
    bool waitSaveState = false;
    bool notifyApp = false;
    bool forceExit = false;
    bool needKillProcess = true;
    // error object
    ErrorObject errorObject;
    FaultDataType faultType = FaultDataType::UNKNOWN;
    int32_t pid = -1;
    uint32_t state = 0;
    int32_t eventId = -1;
    uint64_t schedTime = 0;
    uint64_t detectTime = 0;
    int32_t appStatus = -1;
    sptr<IRemoteObject> token = nullptr;
    std::string timeoutMarkers;
    std::string appfreezeInfo;
    std::string appRunningUniqueId;
    std::string procStatm;
    bool isInForeground;
    bool isEnableMainThreadSample;
    std::string applicationHeapInfo;
    std::string processLifeTime;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_FAULT_DATA_H