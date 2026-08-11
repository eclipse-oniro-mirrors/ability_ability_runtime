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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "insight_intent_rdb_data_mgr.h"

namespace OHOS {
namespace AbilityRuntime {
bool g_mockQueryDataRet = true;
bool g_mockQueryDataBeginWithKeyRet = true;
bool g_mockInsertDataRet = true;
bool g_mockDeleteDataRet = true;
bool g_mockDeleteDataBeginWithKeyRet = true;

void MockQueryData(bool mockRet)
{
    g_mockQueryDataRet = mockRet;
}


void MockQueryDataBeginWithKey(bool mockRet)
{
    g_mockQueryDataBeginWithKeyRet = mockRet;
}

void MockInsertData(bool mockRet)
{
    g_mockInsertDataRet = mockRet;
}

void MockDeleteData(bool mockRet)
{
    g_mockDeleteDataRet = mockRet;
}

void MockDeleteDataBeginWithKey(bool mockRet)
{
    g_mockDeleteDataBeginWithKeyRet = mockRet;
}

}
}

namespace OHOS {
namespace AbilityRuntime {
InsightIntentRdbDataMgr::InsightIntentRdbDataMgr()
{}

InsightIntentRdbDataMgr::~InsightIntentRdbDataMgr()
{}

bool InsightIntentRdbDataMgr::QueryData(const std::string &key, std::string &value)
{
    if (g_mockQueryDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::QueryDataBeginWithKey(const std::string &key,
    std::unordered_map<std::string, std::string> &datas)
{
    if (g_mockQueryDataBeginWithKeyRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::InsertData(const std::string &key, const std::string &value)
{
    if (g_mockInsertDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::DeleteData(const std::string &key)
{
    if (g_mockDeleteDataRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::DeleteDataBeginWithKey(const std::string &key)
{
    if (g_mockDeleteDataBeginWithKeyRet) {
        return true;
    }
    return false;
}

bool InsightIntentRdbDataMgr::UpdateData(const std::string &key, const std::string &value)
{
    return true;
}

bool InsightIntentRdbDataMgr::QueryAllData(std::unordered_map<std::string, std::string> &datas)
{
    return true;
}

void InsightIntentRdbDataMgr::BackupRdb()
{
    return;
}

IntentRdbOpenCallback::IntentRdbOpenCallback(const IntentRdbConfig &intentRdbConfig)
{
}

int32_t IntentRdbOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnUpgrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnDowngrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::onCorruption(std::string databaseFile)
{
    return NativeRdb::E_OK;
}
}
}