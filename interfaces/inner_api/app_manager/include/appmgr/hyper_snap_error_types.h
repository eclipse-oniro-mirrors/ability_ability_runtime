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

#ifndef OHOS_ABILITY_RUNTIME_HYPER_SNAP_ERROR_TYPES_H
#define OHOS_ABILITY_RUNTIME_HYPER_SNAP_ERROR_TYPES_H

#include <string>

namespace OHOS {
namespace AppExecFwk {

// HyperSnap error code enumeration (for external use)
enum class HyperSnapErrorCode {
    ERR_OK = 0,                              // Success
    ERR_SYSTEM_INNER = 1,                    // System internal error
    ERR_SNAPSHOT_EXIST = 2,                  // Snapshot already exists
    ERR_PROCESS_IS_RUNNING = 3,              // Process is already running when preparing snapshot
    ERR_SNAPSHOT_PROCESS_IS_DIED = 4,        // Snapshot process died during snapshot creation
    ERR_SNAPSHOT_IS_INTERRUPTED = 5,         // Snapshot creation interrupted by user starting app
    ERR_EXISTS_ILLEGAL_BINDER = 6,           // Illegal binder exists
    ERR_LAST_PROCESS_NOT_FULLY_EXITED = 7,   // Last process not fully exited
};

// Error type enumeration
enum class ErrorType {
    CREATE_SNAPSHOT = 0,      // Snapshot creation failed
    FORK_FROM_SNAPSHOT = 1,   // Fork from snapshot failed
};

// HyperSnap error record structure
struct HyperSnapErrorRecord {
    HyperSnapErrorCode code;           // Error code
    std::string msg;                   // Error message
    std::string occurTimeStamp;        // Timestamp (milliseconds as string)
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_HYPER_SNAP_ERROR_TYPES_H
