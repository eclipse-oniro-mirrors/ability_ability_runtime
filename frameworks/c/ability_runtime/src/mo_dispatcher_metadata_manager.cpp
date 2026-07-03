
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mo_dispatcher_metadata_manager.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "message_option.h"
#include "mo_dispatcher_complex_type_manager.h"
#include "nlohmann/json.hpp"
#include "securec.h"
#include "string_ex.h"

namespace OHOS::AbilityRuntime {
namespace {
using Json = nlohmann::json;

bool ReadAllFromFd(int32_t fd, std::string* out)
{
    if (fd < 0 || out == nullptr) {
        return false;
    }
    out->clear();
    char buffer[4096] = {0};
    ssize_t n = 0;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        out->append(buffer, static_cast<size_t>(n));
    }
    close(fd);
    return n == 0;
}

std::string ToLower(const std::string& v)
{
    std::string out = v;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

void InsertNameMap(std::unordered_map<std::string, uint32_t>& map, const std::string& key, uint32_t value)
{
    if (!key.empty() && map.find(key) == map.end()) {
        map.emplace(key, value);
    }
}

void InsertIdMap(std::unordered_map<uint32_t, std::string>& map, uint32_t key, const std::string& value)
{
    if (!value.empty() && map.find(key) == map.end()) {
        map.emplace(key, value);
    }
}

uint32_t GetMemberId(const Json& obj)
{
    if (!obj.is_object() || !obj.contains("memberId")) {
        return 0;
    }
    if (obj["memberId"].is_number_unsigned()) {
        uint32_t memId = obj["memberId"].get<uint32_t>();
        if (memId > 0) {
            return memId;
        }
    }
    return 0;
}

OH_AbilityRuntime_ModObjDispatcher_ValueType MapTypeStringToVt(const std::string& typeName)
{
    const std::string type = ToLower(typeName);
    if (type == "void") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID;
    if (type == "bool" || type == "boolean") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL;
    if (type == "i8" || type == "int8") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8;
    if (type == "i16" || type == "int16") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16;
    if (type == "i32" || type == "int32") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    if (type == "i64" || type == "int64") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64;
    if (type == "u8" || type == "uint8") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8;
    if (type == "u16" || type == "uint16") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16;
    if (type == "u32" || type == "uint32") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32;
    if (type == "u64" || type == "uint64") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64;
    if (type == "f32" || type == "float") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32;
    if (type == "f64" || type == "double") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64;
    if (type == "string") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    if (type == "array") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    if (type == "vector") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    if (type == "set") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
    if (type == "map") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    if (type == "struct") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT;
    if (type == "interface") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY;
    if (type == "enum") return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM;
    return OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
}

bool IsSimpleVtType(OH_AbilityRuntime_ModObjDispatcher_ValueType vt)
{
    switch (vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING:
            return true;
        default:
            return false;
    }
}

} // namespace

// -------- ParseTypeInfoFromJson: validated parsing for metadata --------

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::ParseTypeInfoFromJson(const Json& typeInfoObj,
    std::shared_ptr<MoTypeInfo>& result)
{
    if (!typeInfoObj.is_object() || !typeInfoObj.contains("type") || !typeInfoObj["type"].is_string()) {
        TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: invalid type_info object");
        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
    }

    result = std::make_shared<MoTypeInfo>();
    const std::string typeStr = typeInfoObj["type"].get<std::string>();
    const std::string typeLower = ToLower(typeStr);
    result->vt = MapTypeStringToVt(typeStr);

    if (result->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY) {
        TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: unknown type '%{public}s'", typeStr.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
    }

    if (typeLower == "map") {
        // key_type is mandatory and must be a simple type
        if (!typeInfoObj.contains("key_type") || !typeInfoObj["key_type"].is_object()) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: map missing key_type");
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        auto keyInfo = std::make_shared<MoTypeInfo>();
        auto ret = ParseTypeInfoFromJson(typeInfoObj["key_type"], keyInfo);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
        if (!IsSimpleVtType(keyInfo->vt)) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: map key_type is not simple type");
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        result->mapKeyType = keyInfo->vt;

        // value_type is mandatory
        if (!typeInfoObj.contains("value_type") || !typeInfoObj["value_type"].is_object()) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: map missing value_type");
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        auto valInfo = std::make_shared<MoTypeInfo>();
        ret = ParseTypeInfoFromJson(typeInfoObj["value_type"], valInfo);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
        result->pMapValueType = valInfo;
    } else if (typeLower == "array") {
        // value_type is mandatory
        if (!typeInfoObj.contains("value_type") || !typeInfoObj["value_type"].is_object()) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: array missing value_type");
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        // size is mandatory for fixed-size arrays
        if (!typeInfoObj.contains("size") || !typeInfoObj["size"].is_number_unsigned()) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: array missing size");
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        auto elemInfo = std::make_shared<MoTypeInfo>();
        auto ret = ParseTypeInfoFromJson(typeInfoObj["value_type"], elemInfo);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
        result->pElementType = elemInfo;
        result->arraySize = typeInfoObj["size"].get<uint32_t>();
    } else if (typeLower == "vector" || typeLower == "set") {
        // value_type is mandatory
        if (!typeInfoObj.contains("value_type") || !typeInfoObj["value_type"].is_object()) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: %{public}s missing value_type", typeLower.c_str());
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        auto elemInfo = std::make_shared<MoTypeInfo>();
        auto ret = ParseTypeInfoFromJson(typeInfoObj["value_type"], elemInfo);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
        result->pElementType = elemInfo;
    } else if (typeLower == "enum" || typeLower == "interface" || typeLower == "struct") {
        // idl_type is mandatory
        if (!typeInfoObj.contains("idl_type") || !typeInfoObj["idl_type"].is_string()) {
            TAG_LOGE(AAFwkTag::EXT, "ParseTypeInfoFromJson: %{public}s missing idl_type", typeLower.c_str());
            return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
        }
        result->idlType = typeInfoObj["idl_type"].get<std::string>();
        // Note: idl_type reference validation is done later after all types are parsed
    }

    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

// -------- IsIdlTypeDeclared: check if idl_type references a known type --------

bool ModObjDispatcherMetadataManager::IsIdlTypeDeclared(const std::string& idlType) const
{
    // Check enums
    for (const auto& e : enums_) {
        if (e.name == idlType) {
            return true;
        }
    }
    // Check structs
    for (const auto& s : structs_) {
        if (s.name == idlType) {
            return true;
        }
    }
    // Check interfaces
    for (const auto& iface : interfaces_) {
        if (iface.name == idlType) {
            return true;
        }
    }
    return false;
}

// -------- FillCTypeInfo: fill a C TypeInfo struct from MoTypeInfo (recursive deep copy) --------

void ModObjDispatcherMetadataManager::FillCTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType,
    const std::shared_ptr<MoTypeInfo>& moType)
{
    if (cType == nullptr || moType == nullptr) {
        return;
    }
    moType->FillCTypeInfo(cType);
}

// -------- EnsureLoaded --------

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::EnsureLoaded(OHOS::IRemoteObject* proxy)
{
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "EnsureLoaded: proxy is nullptr");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    std::string jsonText;
    auto ret = RequestMetadataJson(proxy, &jsonText);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "EnsureLoaded: RequestMetadataJson failed, ret=%{public}d", ret);
        return ret;
    }
    ret = ParseMetadata(jsonText);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "EnsureLoaded: ParseMetadata failed, ret=%{public}d", ret);
        return ret;
    }
    loaded_ = true;
    ModObjDispatcherComplexTypeManager::RegisterStructMetadata(structs_);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

// -------- ClearCache --------

void ModObjDispatcherMetadataManager::ClearCache()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return;
    }
    ModObjDispatcherComplexTypeManager::UnregisterStructMetadata(structs_);
    loaded_ = false;
    version_.clear();
    mainServiceInterface_.clear();
    interfaces_.clear();
    enums_.clear();
    structs_.clear();
    nameToMemberId_.clear();
    memberIdToName_.clear();
    memberIdToMethod_.clear();
    TAG_LOGI(AAFwkTag::EXT, "ClearCache: metadata cache cleared");
}

// -------- RequestMetadataJson --------

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::RequestMetadataJson(OHOS::IRemoteObject* proxy,
    std::string* jsonText)
{
    if (proxy == nullptr || jsonText == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "RequestMetadataJson: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    static const char* TLB_PATH = "/data/storage/el2/base/haps/entry/files/tlb.json";

    int fd = open(TLB_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::EXT, "RequestMetadataJson: open tlb.json for write failed, errno=%{public}d", errno);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    MessageParcel dataParcel;
    MessageParcel replyParcel;

    std::u16string descriptor = proxy->GetInterfaceDescriptor();
    if (!dataParcel.WriteInterfaceToken(descriptor)) {
        TAG_LOGE(AAFwkTag::EXT, "RequestMetadataJson: WriteInterfaceToken failed");
        close(fd);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    if (!dataParcel.WriteFileDescriptor(fd)) {
        TAG_LOGE(AAFwkTag::EXT, "RequestMetadataJson: WriteFileDescriptor failed");
        close(fd);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    MessageOption option(MessageOption::TF_SYNC);
    int ipcRet = proxy->SendRequest(IPC_CODE_GET_TLB_FD, dataParcel, replyParcel, option);
    if (ipcRet != 0) {
        TAG_LOGE(AAFwkTag::EXT, "RequestMetadataJson: SendRequest failed, ipcRet=%{public}d", ipcRet);
        close(fd);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    // Server has written JSON to fd, now read it back from the local file
    close(fd);

    fd = open(TLB_PATH, O_RDONLY);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::EXT, "RequestMetadataJson: open tlb.json for read failed, errno=%{public}d", errno);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    ReadAllFromFd(fd, jsonText);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

// -------- ParseMetadata --------

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::ParseMetadata(const std::string& jsonText)
{
    Json root = Json::parse(jsonText, nullptr, false);
    if (root.is_discarded() || !root.is_object()) {
        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: tlb.json parse failed, is_discarded=%{public}d", root.is_discarded());
        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
    }

    interfaces_.clear();
    enums_.clear();
    structs_.clear();
    nameToMemberId_.clear();
    memberIdToName_.clear();
    memberIdToMethod_.clear();
    mainServiceInterface_.clear();

    if (root.contains("version") && root["version"].is_string()) {
        version_ = root["version"].get<std::string>();
    }

    // Track all memberId values to enforce uniqueness
    std::unordered_set<uint32_t> usedIds;

    if (root.contains("enums") && root["enums"].is_array()) {
        for (const auto& enumObj : root["enums"]) {
            if (!enumObj.is_object()) {
                continue;
            }
            MoEnumMeta meta;
            meta.name = enumObj.value("name", "");
            meta.memberId = GetMemberId(enumObj);
            if (meta.memberId == 0) {
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: enum '%{public}s' has invalid memberId", meta.name.c_str());
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            if (usedIds.count(meta.memberId) > 0) {
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: enum '%{public}s' duplicate memberId=%{public}u",
                    meta.name.c_str(), meta.memberId);
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            usedIds.insert(meta.memberId);
            InsertNameMap(nameToMemberId_, meta.name, meta.memberId);
            InsertIdMap(memberIdToName_, meta.memberId, meta.name);
            if (enumObj.contains("values") && enumObj["values"].is_array()) {
                for (const auto& valueObj : enumObj["values"]) {
                    MoEnumValueMeta value;
                    value.name = valueObj.value("name", "");
                    value.value = valueObj.value("value", 0);
                    value.memberId = GetMemberId(valueObj);
                    if (value.memberId > 0 && usedIds.count(value.memberId) > 0) {
                        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: enum value '%{public}s.%{public}s' duplicate "
                            "memberID=%{public}u", meta.name.c_str(), value.name.c_str(), value.memberId);
                        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                    }
                    if (value.memberId > 0) {
                        usedIds.insert(value.memberId);
                    }
                    meta.values.emplace_back(value);
                    InsertNameMap(nameToMemberId_, meta.name + "." + value.name, value.memberId);
                    InsertIdMap(memberIdToName_, value.memberId, meta.name + "." + value.name);
                }
            }
            enums_.emplace_back(std::move(meta));
        }
    }

    if (root.contains("structs") && root["structs"].is_array()) {
        for (const auto& structObj : root["structs"]) {
            if (!structObj.is_object()) {
                continue;
            }
            MoStructMeta meta;
            meta.name = structObj.value("name", "");
            meta.memberId = GetMemberId(structObj);
            if (meta.memberId == 0) {
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: struct '%{public}s' has invalid memberId", meta.name.c_str());
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            if (usedIds.count(meta.memberId) > 0) {
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: struct '%{public}s' duplicate memberID=%{public}u",
                    meta.name.c_str(), meta.memberId);
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            usedIds.insert(meta.memberId);
            InsertNameMap(nameToMemberId_, meta.name, meta.memberId);
            InsertIdMap(memberIdToName_, meta.memberId, meta.name);
            if (structObj.contains("fields") && structObj["fields"].is_array()) {
                for (const auto& fieldObj : structObj["fields"]) {
                    MoStructFieldMeta field;
                    field.name = fieldObj.value("name", "");
                    field.memberId = GetMemberId(fieldObj);
                    if (field.memberId > 0 && usedIds.count(field.memberId) > 0) {
                        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: struct field '%{public}s.%{public}s' duplicate "
                            "memberID=%{public}u", meta.name.c_str(), field.name.c_str(), field.memberId);
                        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                    }
                    if (field.memberId > 0) {
                        usedIds.insert(field.memberId);
                    }
                    if (fieldObj.contains("type_info") && fieldObj["type_info"].is_object()) {
                        std::shared_ptr<MoTypeInfo> fieldTypeInfo;
                        auto ret = ParseTypeInfoFromJson(fieldObj["type_info"], fieldTypeInfo);
                        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                            TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: struct field '%{public}s.%{public}s' type_info "
                                "invalid, ret=%{public}d", meta.name.c_str(), field.name.c_str(), ret);
                            return ret;
                        }
                        field.typeInfo = fieldTypeInfo;
                    }
                    meta.fields.emplace_back(field);
                    InsertNameMap(nameToMemberId_, meta.name + "." + field.name, field.memberId);
                    InsertIdMap(memberIdToName_, field.memberId, meta.name + "." + field.name);
                }
            }
            structs_.emplace_back(std::move(meta));
        }
    }

    if (root.contains("interfaces") && root["interfaces"].is_array()) {
        for (const auto& interfaceObj : root["interfaces"]) {
            if (!interfaceObj.is_object()) {
                continue;
            }
            MoInterfaceMeta interfaceMeta;
            interfaceMeta.name = interfaceObj.value("name", "");
            interfaceMeta.descriptor = interfaceObj.value("descriptor", "");
            interfaceMeta.memberId = GetMemberId(interfaceObj);
            if (interfaceMeta.memberId == 0) {
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: interface '%{public}s' has invalid memberId",
                    interfaceMeta.name.c_str());
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            if (usedIds.count(interfaceMeta.memberId) > 0) {
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: interface '%{public}s' duplicate memberID=%{public}u",
                    interfaceMeta.name.c_str(), interfaceMeta.memberId);
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            usedIds.insert(interfaceMeta.memberId);

            // Parse interface_type field
            uint32_t ifaceTypeVal = interfaceObj.value("interface_type", 0u);
            if (ifaceTypeVal > 2) { // must be 0 (NORMAL), 1 (MAIN_SERVICE), or 2 (CALLBACK)
                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: interface '%{public}s' invalid interface_type=%{public}u",
                    interfaceMeta.name.c_str(), ifaceTypeVal);
                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
            }
            interfaceMeta.interfaceType = static_cast<MoInterfaceType>(ifaceTypeVal);
            interfaceMeta.descriptorJson = interfaceObj.dump();
            InsertNameMap(nameToMemberId_, interfaceMeta.name, interfaceMeta.memberId);
            InsertIdMap(memberIdToName_, interfaceMeta.memberId, interfaceMeta.name);

            if (interfaceObj.contains("methods") && interfaceObj["methods"].is_array()) {
                for (const auto& methodObj : interfaceObj["methods"]) {
                    MoMethodMeta method;
                    method.interfaceName = interfaceMeta.name;
                    method.name = methodObj.value("name", "");
                    method.memberId = GetMemberId(methodObj);
                    if (method.memberId == 0) {
                        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: method '%{public}s.%{public}s' has invalid memberId",
                            interfaceMeta.name.c_str(), method.name.c_str());
                        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                    }
                    if (usedIds.count(method.memberId) > 0) {
                        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: method '%{public}s.%{public}s' duplicate "
                            "memberID=%{public}u", interfaceMeta.name.c_str(), method.name.c_str(), method.memberId);
                        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                    }
                    usedIds.insert(method.memberId);
                    method.ipcCode = methodObj.value("code", 0);
                    method.oneway = methodObj.value("oneway", false);
                    if (methodObj.contains("return_type") && methodObj["return_type"].is_object()) {
                        std::shared_ptr<MoTypeInfo> retTypeInfo;
                        auto ret = ParseTypeInfoFromJson(methodObj["return_type"], retTypeInfo);
                        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                            TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: method '%{public}s.%{public}s' return_type "
                                "invalid, ret=%{public}d", interfaceMeta.name.c_str(), method.name.c_str(), ret);
                            return ret;
                        }
                        method.returnType = retTypeInfo;
                    }
                    if (methodObj.contains("parameters") && methodObj["parameters"].is_array()) {
                        for (const auto& paramObj : methodObj["parameters"]) {
                            MoMethodParamMeta param;
                            param.name = paramObj.value("name", "");
                            if (paramObj.contains("type_info") && paramObj["type_info"].is_object()) {
                                std::shared_ptr<MoTypeInfo> paramTypeInfo;
                                auto ret = ParseTypeInfoFromJson(paramObj["type_info"], paramTypeInfo);
                                if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                                    TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: param '%{public}s.%{public}s.%{public}s' "
                                        "type_info invalid, ret=%{public}d", interfaceMeta.name.c_str(),
                                        method.name.c_str(), param.name.c_str(), ret);
                                    return ret;
                                }
                                param.typeInfo = paramTypeInfo;
                            }
                            method.params.emplace_back(param);
                            const uint32_t paramMemberId = GetMemberId(paramObj);
                            if (paramMemberId > 0 && usedIds.count(paramMemberId) > 0) {
                                TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: param '%{public}s.%{public}s.%{public}s' "
                                    "duplicate memberID=%{public}u", interfaceMeta.name.c_str(), method.name.c_str(),
                                    param.name.c_str(), paramMemberId);
                                return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                            }
                            if (paramMemberId > 0) {
                                usedIds.insert(paramMemberId);
                            }
                            InsertNameMap(nameToMemberId_,
                                interfaceMeta.name + "." + method.name + "." + param.name, paramMemberId);
                            InsertIdMap(memberIdToName_, paramMemberId,
                                interfaceMeta.name + "." + method.name + "." + param.name);
                        }
                    }
                    interfaceMeta.methods.emplace_back(method);
                    memberIdToMethod_[method.memberId] = method;
                    InsertNameMap(nameToMemberId_, interfaceMeta.name + "." + method.name, method.memberId);
                    InsertIdMap(memberIdToName_, method.memberId, interfaceMeta.name + "." + method.name);
                }
            }

            if (interfaceMeta.IsMainService()) {
                if (!mainServiceInterface_.empty()) {
                    TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: duplicate mainService interface, first=%{public}s,"
                        "second=%{public}s", mainServiceInterface_.c_str(), interfaceMeta.name.c_str());
                    return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                }
                mainServiceInterface_ = interfaceMeta.name;
            }
            interfaces_.emplace_back(std::move(interfaceMeta));
        }
    }

    if (mainServiceInterface_.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: mainServiceInterface not found in tlb.json");
        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
    }

    // Validate idl_type references in all parsed type info
    for (const auto& st : structs_) {
        for (const auto& field : st.fields) {
            if (field.typeInfo && !field.typeInfo->idlType.empty()) {
                if (!IsIdlTypeDeclared(field.typeInfo->idlType)) {
                    TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: struct '%{public}s' field '%{public}s' unresolved "
                        "idl_type '%{public}s'", st.name.c_str(), field.name.c_str(), field.typeInfo->idlType.c_str());
                    return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                }
            }
        }
    }
    for (const auto& iface : interfaces_) {
        for (const auto& method : iface.methods) {
            if (method.returnType && !method.returnType->idlType.empty()) {
                if (!IsIdlTypeDeclared(method.returnType->idlType)) {
                    TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: method '%{public}s.%{public}s' return_type unresolved "
                        "idl_type '%{public}s'", iface.name.c_str(), method.name.c_str(),
                        method.returnType->idlType.c_str());
                    return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                }
            }
            for (const auto& param : method.params) {
                if (param.typeInfo && !param.typeInfo->idlType.empty()) {
                    if (!IsIdlTypeDeclared(param.typeInfo->idlType)) {
                        TAG_LOGE(AAFwkTag::EXT, "ParseMetadata: param '%{public}s.%{public}s.%{public}s' unresolved "
                            "idl_type '%{public}s'", iface.name.c_str(), method.name.c_str(), param.name.c_str(),
                            param.typeInfo->idlType.c_str());
                        return ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID;
                    }
                }
            }
        }
    }

    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

// -------- QueryMainServiceInterfaceMemberIds --------

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::QueryMainServiceInterfaceMemberIds(
    const char** names, uint32_t count, uint32_t* memberIds) const
{
    if (names == nullptr || memberIds == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "QueryMainServiceInterfaceMemberIds: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_ || mainServiceInterface_.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "QueryMainServiceInterfaceMemberIds: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (uint32_t i = 0; i < count; i++) {
        if (names[i] == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "QueryMainServiceInterfaceMemberIds: name null at index=%{public}u", i);
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        std::string qualifiedName = mainServiceInterface_ + "." + names[i];
        auto it = nameToMemberId_.find(qualifiedName);
        if (it == nameToMemberId_.end()) {
            TAG_LOGE(AAFwkTag::EXT, "QueryMainServiceInterfaceMemberIds: name not found, name=%{public}s",
                names[i]);
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
        memberIds[i] = it->second;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}


AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodMeta(uint32_t memberId,
    MoMethodMeta* methodMeta) const
{
    if (methodMeta == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMeta: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMeta: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto it = memberIdToMethod_.find(memberId);
    if (it == memberIdToMethod_.end()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMeta: not found, memberId=%{public}u", memberId);
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *methodMeta = it->second;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetVersion(std::string* version) const
{
    if (version == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetVersion: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetVersion: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *version = version_;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMainServiceInterfaceName(std::string* interfaceName) const
{
    if (interfaceName == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMainServiceInterfaceName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMainServiceInterfaceName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    if (mainServiceInterface_.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMainServiceInterfaceName: empty");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *interfaceName = mainServiceInterface_;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetInterfaceCount(uint32_t* count) const
{
    if (count == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *count = static_cast<uint32_t>(interfaces_.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetInterfaceName(uint32_t index, std::string* name) const
{
    if (name == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (index >= interfaces_.size()) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceName: out of range, index=%{public}u, size=%{public}zu",
            index, interfaces_.size());
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *name = interfaces_[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetInterfaceIsCallback(const std::string& interfaceName,
    bool* isCallback) const
{
    if (isCallback == nullptr || interfaceName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceIsCallback: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceIsCallback: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : interfaces_) {
        if (item.name == interfaceName) {
            *isCallback = item.IsCallback();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetInterfaceIsCallback: not found, name=%{public}s", interfaceName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetInterfaceDescriptor(const std::string& interfaceName,
    std::u16string* descriptor) const
{
    if (descriptor == nullptr || interfaceName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceDescriptor: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceDescriptor: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : interfaces_) {
        if (item.name == interfaceName) {
            *descriptor = Str8ToStr16(item.descriptor);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetInterfaceDescriptor: not found, name=%{public}s", interfaceName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetEnumCount(uint32_t* count) const
{
    if (count == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *count = static_cast<uint32_t>(enums_.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetEnumName(uint32_t index, std::string* name) const
{
    if (name == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (index >= enums_.size()) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumName: out of range, index=%{public}u, size=%{public}zu", index, enums_.size());
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *name = enums_[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetEnumValueCount(const std::string& enumName,
    uint32_t* count) const
{
    if (count == nullptr || enumName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : enums_) {
        if (item.name == enumName) {
            *count = static_cast<uint32_t>(item.values.size());
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetEnumValueCount: not found, enum='%{public}s'", enumName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetEnumValueName(const std::string& enumName, uint32_t index,
    std::string* valueName) const
{
    if (valueName == nullptr || enumName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : enums_) {
        if (item.name == enumName) {
            if (index >= item.values.size()) {
                TAG_LOGE(AAFwkTag::EXT, "GetEnumValueName: out of range, index=%{public}u, size=%{public}zu",
                    index, item.values.size());
                return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
            }
            *valueName = item.values[index].name;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetEnumValueName: not found, enum='%{public}s'", enumName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetEnumValue(const std::string& enumName,
    const std::string& valueName, int32_t* value) const
{
    if (value == nullptr || enumName.empty() || valueName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValue: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValue: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : enums_) {
        if (item.name == enumName) {
            for (const auto& enumValue : item.values) {
                if (enumValue.name == valueName) {
                    *value = enumValue.value;
                    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
                }
            }
            TAG_LOGE(AAFwkTag::EXT, "GetEnumValue: not found, enum='%{public}s', value='%{public}s'",
                enumName.c_str(), valueName.c_str());
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetEnumValue: not found, enum='%{public}s'", enumName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetStructCount(uint32_t* count) const
{
    if (count == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *count = static_cast<uint32_t>(structs_.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetStructName(uint32_t index, std::string* name) const
{
    if (name == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (index >= structs_.size()) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructName: out of range, index=%{public}u, size=%{public}zu",
            index, structs_.size());
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *name = structs_[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetStructFieldCount(const std::string& structName,
    uint32_t* count) const
{
    if (count == nullptr || structName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : structs_) {
        if (item.name == structName) {
            *count = static_cast<uint32_t>(item.fields.size());
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetStructFieldCount: not found, struct='%{public}s'", structName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetStructFieldName(const std::string& structName,
    uint32_t index, std::string* fieldName) const
{
    if (fieldName == nullptr || structName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : structs_) {
        if (item.name == structName) {
            if (index >= item.fields.size()) {
                TAG_LOGE(AAFwkTag::EXT, "GetStructFieldName: out of range, index=%{public}u, size=%{public}zu",
                    index, item.fields.size());
                return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
            }
            *fieldName = item.fields[index].name;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetStructFieldName: not found, struct='%{public}s'", structName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetStructFieldType(const std::string& structName,
    const std::string& fieldName, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* fieldType) const
{
    if (fieldType == nullptr || structName.empty() || fieldName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldType: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : structs_) {
        if (item.name == structName) {
            for (const auto& field : item.fields) {
                if (field.name == fieldName) {
                    FillCTypeInfo(fieldType, field.typeInfo);
                    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
                }
            }
            TAG_LOGE(AAFwkTag::EXT, "GetStructFieldType: not found, struct='%{public}s', field='%{public}s'",
                structName.c_str(), fieldName.c_str());
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "GetStructFieldType: not found, struct='%{public}s'", structName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

// -------- Method query instance methods --------

const MoInterfaceMeta* FindInterfaceByName(const std::vector<MoInterfaceMeta>& interfaces,
    const std::string& name)
{
    for (const auto& item : interfaces) {
        if (item.name == name) {
            return &item;
        }
    }
    return nullptr;
}

const MoMethodMeta* FindMethodByName(const std::vector<MoMethodMeta>& methods, const std::string& name)
{
    for (const auto& item : methods) {
        if (item.name == name) {
            return &item;
        }
    }
    return nullptr;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodCount(const std::string& interfaceName,
    uint32_t* count) const
{
    if (count == nullptr || interfaceName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodCount: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *count = static_cast<uint32_t>(iface->methods.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodName(const std::string& interfaceName,
    uint32_t index, std::string* methodName) const
{
    if (methodName == nullptr || interfaceName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodName: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    if (index >= iface->methods.size()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodName: out of range, index=%{public}u, size=%{public}zu",
            index, iface->methods.size());
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *methodName = iface->methods[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodMemberId(const std::string& interfaceName,
    const std::string& methodName, uint32_t* memberId) const
{
    if (memberId == nullptr || interfaceName.empty() || methodName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMemberId: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMemberId: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMemberId: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto* method = FindMethodByName(iface->methods, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMemberId: not found, method='%{public}s'", methodName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *memberId = method->memberId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodReturnType(const std::string& interfaceName,
    const std::string& methodName, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* returnType) const
{
    if (returnType == nullptr || interfaceName.empty() || methodName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodReturnType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodReturnType: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodReturnType: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto* method = FindMethodByName(iface->methods, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodReturnType: not found, method='%{public}s'", methodName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    if (method->returnType) {
        method->returnType->FillCTypeInfo(returnType);
    } else {
        (void)memset_s(returnType, sizeof(OH_AbilityRuntime_ModObjDispatcher_TypeInfo), 0,
            sizeof(OH_AbilityRuntime_ModObjDispatcher_TypeInfo));
        returnType->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodParamCount(const std::string& interfaceName,
    const std::string& methodName, uint32_t* count) const
{
    if (count == nullptr || interfaceName.empty() || methodName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamCount: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamCount: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamCount: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto* method = FindMethodByName(iface->methods, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamCount: not found, method='%{public}s'", methodName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *count = static_cast<uint32_t>(method->params.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodParamType(const std::string& interfaceName,
    const std::string& methodName, uint32_t paramIndex,
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* paramType) const
{
    if (paramType == nullptr || interfaceName.empty() || methodName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamType: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamType: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto* method = FindMethodByName(iface->methods, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamType: not found, method='%{public}s'", methodName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    if (paramIndex >= method->params.size()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamType: out of range, index=%{public}u, size=%{public}zu",
            paramIndex, method->params.size());
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto& param = method->params[paramIndex];
    if (param.typeInfo) {
        param.typeInfo->FillCTypeInfo(paramType);
    } else {
        (void)memset_s(paramType, sizeof(OH_AbilityRuntime_ModObjDispatcher_TypeInfo), 0,
            sizeof(OH_AbilityRuntime_ModObjDispatcher_TypeInfo));
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherMetadataManager::GetMethodParamName(const std::string& interfaceName,
    const std::string& methodName, uint32_t paramIndex, std::string* paramName) const
{
    if (paramName == nullptr || interfaceName.empty() || methodName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: not loaded");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto* iface = FindInterfaceByName(interfaces_, interfaceName);
    if (iface == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: not found, iface='%{public}s'", interfaceName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto* method = FindMethodByName(iface->methods, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: not found, method='%{public}s'", methodName.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    if (paramIndex >= method->params.size()) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: out of range, index=%{public}u, size=%{public}zu",
            paramIndex, method->params.size());
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *paramName = method->params[paramIndex].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

} // namespace OHOS::AbilityRuntime
