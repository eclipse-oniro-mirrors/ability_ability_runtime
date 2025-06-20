/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "deeplinkreserveconfig_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "securec.h"

#define private public
#include "deeplink_reserve_config.h"
#undef private

using namespace OHOS::AAFwk;
namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
const std::string CONFIG_PATH = "/etc/ability_runtime/deeplink_reserve_config.json";
const std::string DEFAULT_RESERVE_CONFIG_PATH = "/system/etc/deeplink_reserve_config.json";
const std::string DEEPLINK_RESERVED_URI_NAME = "deepLinkReservedUri";
const std::string BUNDLE_NAME = "bundleName";
const std::string URIS_NAME = "uris";
const std::string SCHEME_NAME = "scheme";
const std::string HOST_NAME = "host";
const std::string PORT_NAME = "port";
const std::string PATH_NAME = "path";
const std::string PATH_START_WITH_NAME = "pathStartWith";
const std::string PATH_REGEX_NAME = "pathRegex";
const std::string TYPE_NAME = "type";
const std::string UTD_NAME = "utd";
const std::string PORT_SEPARATOR = ":";
const std::string SCHEME_SEPARATOR = "://";
const std::string PATH_SEPARATOR = "/";
const std::string PARAM_SEPARATOR = "?";
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    auto &deepLinkReserveConfig = DeepLinkReserveConfig::GetInstance();
    deepLinkReserveConfig.GetConfigPath();
    deepLinkReserveConfig.LoadConfiguration();
    std::string linkString(data, size);
    std::string bundleName(data, size);
    deepLinkReserveConfig.IsLinkReserved(linkString, bundleName);
    ReserveUri reservedUri;
    std::string link(data, size);
    std::string strParam(data, size);
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    reservedUri.scheme = strParam;
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    reservedUri.host = strParam;
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    reservedUri.port = strParam;
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    reservedUri.path = strParam;
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    reservedUri.pathStartWith = strParam;
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    reservedUri.pathRegex = strParam;
    deepLinkReserveConfig.IsUriMatched(reservedUri, link);
    std::vector<ReserveUri> uriList;
    cJSON *jsonUriObject = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonUriObject, "SCHEME_NAME", SCHEME_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "HOST_NAME", HOST_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "PORT_NAME", PORT_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "PATH_NAME", PATH_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "PATH_START_WITH_NAME", PATH_START_WITH_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "PATH_REGEX_NAME", PATH_REGEX_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "TYPE_NAME", TYPE_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_AddStringToObject(jsonUriObject, "UTD_NAME", UTD_NAME.c_str());
    deepLinkReserveConfig.LoadReservedUrilItem(jsonUriObject, uriList);
    cJSON_Delete(jsonUriObject);
    return true;
}

bool DoSomethingInterestingWithMyAPIOne(const char* data, size_t size)
{
    auto &deepLinkReserveConfig1 = DeepLinkReserveConfig::GetInstance();
    std::string filePath(data, size);
    cJSON *jsonBuf = nullptr;
    deepLinkReserveConfig1.ReadFileInfoJson(filePath, jsonBuf);
    cJSON_Delete(jsonBuf);

    cJSON *object = cJSON_CreateObject();
    deepLinkReserveConfig1.LoadReservedUriList(object);

    cJSON_AddStringToObject(object, "DEEPLINK_RESERVED_URI_NAME", DEEPLINK_RESERVED_URI_NAME.c_str());
    deepLinkReserveConfig1.LoadReservedUriList(object);

    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    cJSON_AddNumberToObject(object, "BUNDLE_NAME", userId);
    deepLinkReserveConfig1.LoadReservedUriList(object);

    cJSON *oldBundleNameItem = cJSON_GetObjectItem(object, "BUNDLE_NAME");
    if (oldBundleNameItem != nullptr) {
        cJSON_DetachItemViaPointer(object, oldBundleNameItem);
        cJSON_Delete(oldBundleNameItem);
    }
    cJSON_AddStringToObject(object, "BUNDLE_NAME", BUNDLE_NAME.c_str());
    deepLinkReserveConfig1.LoadReservedUriList(object);

    cJSON *uriArray = cJSON_CreateArray();
    cJSON_AddItemToArray(uriArray, cJSON_CreateString("uri1"));
    cJSON_AddItemToArray(uriArray, cJSON_CreateString("uri2"));
    cJSON_AddItemToArray(uriArray, cJSON_CreateString("uri3"));
    cJSON_AddItemToObject(object, "URIS_NAME", uriArray);
    deepLinkReserveConfig1.LoadReservedUriList(object);

    cJSON *oldUriNameItem = cJSON_GetObjectItem(object, "URIS_NAME");
    if (oldUriNameItem != nullptr) {
        cJSON_DetachItemViaPointer(object, oldUriNameItem);
        cJSON_Delete(oldUriNameItem);
    }
    cJSON_AddStringToObject(object, "URIS_NAME", URIS_NAME.c_str());
    deepLinkReserveConfig1.LoadReservedUriList(object);

    cJSON_Delete(object);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    OHOS::DoSomethingInterestingWithMyAPIOne(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

