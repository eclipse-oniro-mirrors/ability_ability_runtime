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

#include "database_write_counter.h"
#include "report_data_partition_usage_manager.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t DEFAULT_WRITE_COUNT = 0;
constexpr int32_t DATABASE_WRITE_COUNT = 20;

void DatabaseWriteCounter::ResetWriteCount()
{
    std::lock_guard<std::mutex> lock(writeCountMutex_);
    writeCount_ = DEFAULT_WRITE_COUNT;
}

void DatabaseWriteCounter::UpdateWriteCount(const std::string &dbPath)
{
    std::lock_guard<std::mutex> lock(writeCountMutex_);
    writeCount_++;
    if (writeCount_ >= DATABASE_WRITE_COUNT) {
        ReportDataPartitionUsageManager::SendReportDatabaseReadEvent(dbPath);
        writeCount_ = DEFAULT_WRITE_COUNT;
    }
}

} // namespace AbilityRuntime
} // namespace OHOS