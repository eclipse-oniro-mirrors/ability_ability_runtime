/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ability_runtime/modular_object_dispatcher.h"
#include "ipc_cremote_object.h"
#include "ipc_error_code.h"
#include "ipc_inner_object.h"
#include "message_parcel.h"

static const uint32_t CODE_GET_TLB_FD = 0x00FFFF;
static const uint32_t CODE_ADD = 0x001201;
static const uint32_t CODE_SUM_USER_IDS = 0x001202;

static const char16_t EXPECTED_DESCRIPTOR[] = u"ohos.abilityruntime.ModularObjectService";

static bool ValidateInterfaceToken(const OHIPCParcel *data)
{
    auto *msgParcel = data->msgParcel;
    if (msgParcel == nullptr) {
        return false;
    }
    std::u16string remoteDescriptor = msgParcel->ReadInterfaceToken();
    return remoteDescriptor == EXPECTED_DESCRIPTOR;
}

static const char* DEMO_TLB_JSON =
    "{"
    "\"version\":\"1.0\","
    "\"package\":\"com.example.dispatcher\","
    "\"structs\":[{"
        "\"name\":\"UserInfo\",\"dispID\":200,"
        "\"fields\":["
            "{\"name\":\"id\",\"dispID\":201,\"type_info\":{\"type\":\"i32\"}},"
            "{\"name\":\"name\",\"dispID\":202,\"type_info\":{\"type\":\"String\"}}"
        "]"
    "}],"
    "\"interfaces\":[{"
        "\"name\":\"IDataService\",\"descriptor\":\"com.example.dispatcher.IDataService\","
        "\"dispID\":100,\"is_main_service\":true,"
        "\"methods\":["
            "{\"name\":\"add\",\"dispID\":101,\"code\":4609,"
                "\"parameters\":["
                    "{\"name\":\"a\",\"dispID\":102,\"type_info\":{\"type\":\"i32\"}},"
                    "{\"name\":\"b\",\"dispID\":103,\"type_info\":{\"type\":\"i32\"}}"
                "],\"return_type\":{\"type\":\"i32\"}}"
            ",{\"name\":\"sumUserIds\",\"dispID\":104,\"code\":4610,"
                "\"parameters\":["
                    "{\"name\":\"users\",\"dispID\":105,\"type_info\":{\"type\":\"array\"}}"
                "],\"return_type\":{\"type\":\"i32\"}}"
        "]"
    "}]"
    "}";

static int WriteMetadataFd(OHIPCParcel* reply)
{
    char path[] = "/tmp/modispatch_tlb_XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) {
        return OH_IPC_INNER_ERROR;
    }
    unlink(path);
    size_t jsonLen = strlen(DEMO_TLB_JSON);
    if (write(fd, DEMO_TLB_JSON, jsonLen) != (ssize_t)jsonLen) {
        close(fd);
        return OH_IPC_INNER_ERROR;
    }
    lseek(fd, 0, SEEK_SET);
    int ret = OH_IPCParcel_WriteFileDescriptor(reply, fd);
    close(fd);
    return ret;
}

static int ReadI32Variant(const OHIPCParcel* data, int32_t* out)
{
    int32_t vt = 0;
    if (OH_IPCParcel_ReadInt32(data, &vt) != OH_IPC_SUCCESS) {
        return OH_IPC_PARCEL_READ_ERROR;
    }
    if (vt != OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32) {
        return OH_IPC_PARCEL_READ_ERROR;
    }
    return OH_IPCParcel_ReadInt32(data, out);
}

static int ReadStructVariantAndAccumulateId(const OHIPCParcel* data, int32_t* sum)
{
    int32_t vt = 0;
    if (OH_IPCParcel_ReadInt32(data, &vt) != OH_IPC_SUCCESS || vt != OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        return OH_IPC_PARCEL_READ_ERROR;
    }
    if (OH_IPCParcel_ReadString(data) == NULL) {
        return OH_IPC_PARCEL_READ_ERROR;
    }

    int32_t fieldCount = 0;
    if (OH_IPCParcel_ReadInt32(data, &fieldCount) != OH_IPC_SUCCESS || fieldCount < 0) {
        return OH_IPC_PARCEL_READ_ERROR;
    }

    for (int32_t i = 0; i < fieldCount; i++) {
        const char* fieldName = OH_IPCParcel_ReadString(data);
        if (fieldName == NULL) {
            return OH_IPC_PARCEL_READ_ERROR;
        }

        int32_t fieldVt = 0;
        if (OH_IPCParcel_ReadInt32(data, &fieldVt) != OH_IPC_SUCCESS) {
            return OH_IPC_PARCEL_READ_ERROR;
        }

        if (fieldVt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32) {
            int32_t value = 0;
            if (OH_IPCParcel_ReadInt32(data, &value) != OH_IPC_SUCCESS) {
                return OH_IPC_PARCEL_READ_ERROR;
            }
            if (strcmp(fieldName, "id") == 0) {
                *sum += value;
            }
        } else if (fieldVt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
            if (OH_IPCParcel_ReadString(data) == NULL) {
                return OH_IPC_PARCEL_READ_ERROR;
            }
        } else {
            return OH_IPC_PARCEL_READ_ERROR;
        }
    }
    return OH_IPC_SUCCESS;
}

static int HandleAdd(const OHIPCParcel* data, OHIPCParcel* reply)
{
    int32_t a = 0;
    int32_t b = 0;
    if (ReadI32Variant(data, &a) != OH_IPC_SUCCESS || ReadI32Variant(data, &b) != OH_IPC_SUCCESS) {
        return OH_IPC_PARCEL_READ_ERROR;
    }
    if (OH_IPCParcel_WriteInt32(reply, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32) != OH_IPC_SUCCESS) {
        return OH_IPC_PARCEL_WRITE_ERROR;
    }
    return OH_IPCParcel_WriteInt32(reply, a + b);
}

static int HandleSumUserIds(const OHIPCParcel* data, OHIPCParcel* reply)
{
    int32_t vt = 0;
    if (OH_IPCParcel_ReadInt32(data, &vt) != OH_IPC_SUCCESS || vt != OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        return OH_IPC_PARCEL_READ_ERROR;
    }

    int32_t elemType = 0;
    int32_t size = 0;
    if (OH_IPCParcel_ReadInt32(data, &elemType) != OH_IPC_SUCCESS ||
        OH_IPCParcel_ReadInt32(data, &size) != OH_IPC_SUCCESS ||
        size < 0 || elemType != OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        return OH_IPC_PARCEL_READ_ERROR;
    }

    int32_t sum = 0;
    for (int32_t i = 0; i < size; i++) {
        if (ReadStructVariantAndAccumulateId(data, &sum) != OH_IPC_SUCCESS) {
            return OH_IPC_PARCEL_READ_ERROR;
        }
    }

    if (OH_IPCParcel_WriteInt32(reply, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32) != OH_IPC_SUCCESS) {
        return OH_IPC_PARCEL_WRITE_ERROR;
    }
    return OH_IPCParcel_WriteInt32(reply, sum);
}

static int OnRemoteRequest(uint32_t code, const OHIPCParcel *data, OHIPCParcel *reply, void *userData)
{
    (void)userData;
    if (!ValidateInterfaceToken(data)) {
        return OH_IPC_INNER_ERROR;
    }

    if (code == CODE_GET_TLB_FD) {
        return WriteMetadataFd(reply);
    }

    int32_t memID = 0;
    int32_t argc = 0;
    if (OH_IPCParcel_ReadInt32(data, &memID) != OH_IPC_SUCCESS ||
        OH_IPCParcel_ReadInt32(data, &argc) != OH_IPC_SUCCESS) {
        return OH_IPC_PARCEL_READ_ERROR;
    }

    if (code == CODE_ADD && memID == 101 && argc == 2) {
        return HandleAdd(data, reply);
    }
    if (code == CODE_SUM_USER_IDS && memID == 104 && argc == 1) {
        return HandleSumUserIds(data, reply);
    }
    return OH_IPC_CODE_OUT_OF_RANGE;
}

static int CheckRet(const char* step, AbilityRuntime_ErrorCode ret)
{
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        printf("%s failed, ret=%d\n", step, ret);
        return -1;
    }
    return 0;
}

static int PrintMethodMeta(const char* descriptor, const char* interfaceName)
{
    uint32_t methodCount = 0;
    if (CheckRet("GetMethodCount", OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
        descriptor, interfaceName, &methodCount)) != 0) {
        return -1;
    }
    printf("  methodCount=%u\n", methodCount);

    for (uint32_t i = 0; i < methodCount; i++) {
        char methodName[128] = {0};
        uint32_t methodMemId = 0;
        OH_AbilityRuntime_MoDispatcher_Vt returnType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
        uint32_t paramCount = 0;

        if (CheckRet("GetMethodName", OH_AbilityRuntime_TypeDescriptor_GetMethodName(
            descriptor, interfaceName, i, methodName, sizeof(methodName))) != 0) {
            return -1;
        }
        if (CheckRet("GetMethodMemberId", OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
            descriptor, interfaceName, methodName, &methodMemId)) != 0) {
            return -1;
        }
        if (CheckRet("GetMethodReturnType", OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
            descriptor, interfaceName, methodName, &returnType)) != 0) {
            return -1;
        }
        if (CheckRet("GetMethodParamCount", OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
            descriptor, interfaceName, methodName, &paramCount)) != 0) {
            return -1;
        }

        printf("  method[%u]: name=%s memId=%u returnType=%d paramCount=%u\n",
            i, methodName, methodMemId, (int)returnType, paramCount);

        for (uint32_t p = 0; p < paramCount; p++) {
            char paramName[128] = {0};
            OH_AbilityRuntime_MoDispatcher_Vt paramType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
            if (CheckRet("GetMethodParamName", OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
                descriptor, interfaceName, methodName, p, paramName, sizeof(paramName))) != 0) {
                return -1;
            }
            if (CheckRet("GetMethodParamType", OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
                descriptor, interfaceName, methodName, p, &paramType)) != 0) {
                return -1;
            }
            printf("    param[%u]: name=%s type=%d\n", p, paramName, (int)paramType);
        }
    }
    return 0;
}

static int QueryAndPrintMetadata(OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle typeDesc)
{
    char packageName[128] = {0};
    char version[64] = {0};
    char mainServiceName[128] = {0};

    if (CheckRet("GetPackage", OH_AbilityRuntime_TypeDescriptor_GetPackage(
        typeDesc, packageName, sizeof(packageName))) != 0) {
        return -1;
    }
    if (CheckRet("GetVersion", OH_AbilityRuntime_TypeDescriptor_GetVersion(
        typeDesc, version, sizeof(version))) != 0) {
        return -1;
    }
    if (CheckRet("GetMainServiceInterfaceName", OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
        typeDesc, mainServiceName, sizeof(mainServiceName))) != 0) {
        return -1;
    }

    printf("package=%s version=%s mainService=%s\n", packageName, version, mainServiceName);

    uint32_t interfaceCount = 0;
    if (CheckRet("GetInterfaceCount", OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
        typeDesc, &interfaceCount)) != 0) {
        return -1;
    }
    printf("interfaceCount=%u\n", interfaceCount);

    for (uint32_t i = 0; i < interfaceCount; i++) {
        char interfaceName[128] = {0};
        char descriptor[2048] = {0};
        bool isCallback = false;
        uint32_t interfaceMemId = 0;

        if (CheckRet("GetInterfaceName", OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
            typeDesc, i, interfaceName, sizeof(interfaceName))) != 0) {
            return -1;
        }
        if (CheckRet("GetInterfaceMemberId", OH_AbilityRuntime_TypeDescriptor_GetInterfaceMemberId(
            typeDesc, interfaceName, &interfaceMemId)) != 0) {
            return -1;
        }
        if (CheckRet("GetInterfaceDescriptor", OH_AbilityRuntime_TypeDescriptor_GetInterfaceDescriptor(
            typeDesc, interfaceName, descriptor, sizeof(descriptor))) != 0) {
            return -1;
        }
        if (CheckRet("GetInterfaceIsCallback", OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
            typeDesc, interfaceName, &isCallback)) != 0) {
            return -1;
        }

        printf("interface[%u]: name=%s memId=%u isCallback=%s\n",
            i, interfaceName, interfaceMemId, isCallback ? "true" : "false");

        if (PrintMethodMeta(descriptor, interfaceName) != 0) {
            return -1;
        }
    }

    uint32_t structCount = 0;
    if (CheckRet("GetStructCount", OH_AbilityRuntime_TypeDescriptor_GetStructCount(
        typeDesc, &structCount)) != 0) {
        return -1;
    }
    printf("structCount=%u\n", structCount);

    for (uint32_t i = 0; i < structCount; i++) {
        char structName[128] = {0};
        uint32_t fieldCount = 0;

        if (CheckRet("GetStructName", OH_AbilityRuntime_TypeDescriptor_GetStructName(
            typeDesc, i, structName, sizeof(structName))) != 0) {
            return -1;
        }
        if (CheckRet("GetStructFieldCount", OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
            typeDesc, structName, &fieldCount)) != 0) {
            return -1;
        }

        printf("struct[%u]: name=%s fieldCount=%u\n", i, structName, fieldCount);
        for (uint32_t f = 0; f < fieldCount; f++) {
            char fieldName[128] = {0};
            OH_AbilityRuntime_MoDispatcher_Vt fieldType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
            if (CheckRet("GetStructFieldName", OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
                typeDesc, structName, f, fieldName, sizeof(fieldName))) != 0) {
                return -1;
            }
            if (CheckRet("GetStructFieldType", OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
                typeDesc, structName, fieldName, &fieldType)) != 0) {
                return -1;
            }
            printf("  field[%u]: name=%s type=%d\n", f, fieldName, (int)fieldType);
        }
    }
    return 0;
}

int main(void)
{
    OHIPCRemoteStub* stub = OH_IPCRemoteStub_Create("com.example.dispatcher.IDataService", OnRemoteRequest, NULL, NULL);
    if (stub == NULL) {
        printf("create stub failed\n");
        return -1;
    }

    OHIPCParcel* bridge = OH_IPCParcel_Create();
    OH_IPCParcel_WriteRemoteStub(bridge, stub);
    OH_IPCParcel_RewindReadPosition(bridge, 0);
    OHIPCRemoteProxy* proxy = OH_IPCParcel_ReadRemoteProxy(bridge);
    OH_IPCParcel_Destroy(bridge);
    if (proxy == NULL) {
        printf("create proxy failed\n");
        OH_IPCRemoteStub_Destroy(stub);
        return -1;
    }

    OH_AbilityRuntime_MoDispatcherHandle dispatcher = NULL;
    if (CheckRet("CreateInstance", OH_AbilityRuntime_MoDispatcher_CreateInstance(proxy, &dispatcher)) != 0) {
        OH_IPCRemoteProxy_Destroy(proxy);
        OH_IPCRemoteStub_Destroy(stub);
        return -1;
    }

    uint32_t hasType = 0;
    if (CheckRet("HasTypeDescriptor", OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(dispatcher, &hasType)) != 0) {
        return -1;
    }
    printf("hasTypeDescriptor=%u\n", hasType);

    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle typeDesc = NULL;
    if (CheckRet("GetTypeDescriptor", OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(dispatcher, &typeDesc)) != 0) {
        return -1;
    }

    if (QueryAndPrintMetadata(typeDesc) != 0) {
        return -1;
    }

    const char* names[2] = {"add", "sumUserIds"};
    uint32_t memIds[2] = {0};
    if (CheckRet("QueryMemIDsOfNames", OH_AbilityRuntime_MoDispatcher_QueryMemIDsOfNames(
        dispatcher, names, 2, memIds)) != 0) {
        return -1;
    }
    printf("add memId=%u, sumUserIds memId=%u\n", memIds[0], memIds[1]);

    OH_AbilityRuntime_MoDispatcher_Variant addArgs[2];
    addArgs[0].vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32;
    addArgs[0].u.i32Val = 7;
    addArgs[1].vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32;
    addArgs[1].u.i32Val = 5;

    OH_AbilityRuntime_MoDispatcher_Inputparams addInput = { addArgs, 2, NULL, 0 };
    OH_AbilityRuntime_MoDispatcher_Variant addResult;
    if (CheckRet("CallMethod(add)", OH_AbilityRuntime_MoDispatcher_CallMethod(
        dispatcher, memIds[0], &addInput, &addResult)) != 0) {
        return -1;
    }
    printf("add result=%d\n", addResult.u.i32Val);

    OH_AbilityRuntime_MoDispatcher_StructHandle u1 = NULL;
    OH_AbilityRuntime_MoDispatcher_StructHandle u2 = NULL;
    if (CheckRet("Struct_Create(u1)", OH_AbilityRuntime_MoDispatcher_Struct_Create("UserInfo", &u1)) != 0) {
        return -1;
    }
    if (CheckRet("Struct_Create(u2)", OH_AbilityRuntime_MoDispatcher_Struct_Create("UserInfo", &u2)) != 0) {
        return -1;
    }

    OH_AbilityRuntime_MoDispatcher_Variant id;
    id.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32;
    OH_AbilityRuntime_MoDispatcher_Variant name;
    name.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING;

    id.u.i32Val = 11;
    name.u.bstrVal = "Tom";
    if (CheckRet("Struct_SetField(u1.id)", OH_AbilityRuntime_MoDispatcher_Struct_SetField(u1, "id", &id)) != 0) {
        return -1;
    }
    if (CheckRet("Struct_SetField(u1.name)", OH_AbilityRuntime_MoDispatcher_Struct_SetField(u1, "name", &name)) != 0) {
        return -1;
    }

    id.u.i32Val = 29;
    name.u.bstrVal = "Jerry";
    if (CheckRet("Struct_SetField(u2.id)", OH_AbilityRuntime_MoDispatcher_Struct_SetField(u2, "id", &id)) != 0) {
        return -1;
    }
    if (CheckRet("Struct_SetField(u2.name)", OH_AbilityRuntime_MoDispatcher_Struct_SetField(u2, "name", &name)) != 0) {
        return -1;
    }

    OH_AbilityRuntime_MoDispatcher_ArrayHandle users = NULL;
    if (CheckRet("Array_Create", OH_AbilityRuntime_MoDispatcher_Array_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT, 2, &users)) != 0) {
        return -1;
    }

    OH_AbilityRuntime_MoDispatcher_Variant userVar;
    userVar.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT;
    userVar.u.pstructVal = u1;
    if (CheckRet("Array_Set(0)", OH_AbilityRuntime_MoDispatcher_Array_Set(users, 0, &userVar)) != 0) {
        return -1;
    }

    userVar.u.pstructVal = u2;
    if (CheckRet("Array_Set(1)", OH_AbilityRuntime_MoDispatcher_Array_Set(users, 1, &userVar)) != 0) {
        return -1;
    }

    OH_AbilityRuntime_MoDispatcher_Variant sumArg;
    sumArg.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY;
    sumArg.u.parrayVal = users;
    OH_AbilityRuntime_MoDispatcher_Inputparams sumInput = { &sumArg, 1, NULL, 0 };
    OH_AbilityRuntime_MoDispatcher_Variant sumResult;

    if (CheckRet("CallMethod(sumUserIds)", OH_AbilityRuntime_MoDispatcher_CallMethod(
        dispatcher, memIds[1], &sumInput, &sumResult)) != 0) {
        return -1;
    }
    printf("sumUserIds result=%d\n", sumResult.u.i32Val);

    OH_AbilityRuntime_MoDispatcher_Array_Release(&users);
    OH_AbilityRuntime_MoDispatcher_Struct_Release(&u1);
    OH_AbilityRuntime_MoDispatcher_Struct_Release(&u2);
    OH_AbilityRuntime_TypeDescriptor_Release(&typeDesc);
    OH_AbilityRuntime_MoDispatcher_Release(&dispatcher);
    OH_IPCRemoteProxy_Destroy(proxy);
    OH_IPCRemoteStub_Destroy(stub);
    return 0;
}
