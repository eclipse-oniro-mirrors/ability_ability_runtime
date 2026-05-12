/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @addtogroup AbilityRuntime
 * @{
 *
 * @brief Provide definition of C interface for modular object dispatcher late binding mechanism.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

/**
 * @file modular_object_dispatcher.h
 *
 * @brief Declare common types and interfaces for modular object dispatcher.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef OHOS_ABILITY_RUNTIME_C_MO_DISPATCHER_H
#define OHOS_ABILITY_RUNTIME_C_MO_DISPATCHER_H

#include <stdbool.h>
#include "ability_runtime_common.h"
#include "ipc_cparcel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_TypeDescriptor structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_TypeDescriptor;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher_TypeDescriptor.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_TypeDescriptor* OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle;

 /**
  * @brief Enumerates variant types.
  *
  * @since 26.0.0
  */
typedef enum {
    /** @description Empty value. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY = 0,
    /** @description Void value. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID = 1,
    /** @description Boolean value. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL = 2,
    /** @description 8-bit signed integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8 = 3,
    /** @description 16-bit signed integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16 = 4,
    /** @description 32-bit signed integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32 = 5,
    /** @description 64-bit signed integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64 = 6,
    /** @description 8-bit unsigned integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8 = 7,
    /** @description 16-bit unsigned integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16 = 8,
    /** @description 32-bit unsigned integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32 = 9,
    /** @description 64-bit unsigned integer. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64 = 10,
    /** @description 32-bit floating point. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32 = 11,
    /** @description 64-bit floating point. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64 = 12,
    /** @description String (UTF-8). */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING = 13,
    /** @description Array. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY = 14,
    /** @description Vector. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR = 15,
    /** @description Set. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET = 16,
    /** @description Map. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP = 17,
    /** @description Struct. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT = 18,
    /** @description Ipc Remote Proxy. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY = 19,
    /** @description Ipc Remote Stub. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB = 20,
    /** @description Enum value. */
    OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM = 21,
} OH_AbilityRuntime_MoDispatcher_ValueType;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_TypeInfo structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_TypeInfo;

/**
 * @brief Defines the parameter type descriptor for modular object dispatcher.
 *
 * Describes the type of a parameter or return value using a tagged union.
 * For map types, use u.mapType.keyType and u.mapType.pValueType;
 * for array types, use u.arrayType.pElementType and u.arrayType.size;
 * for vector/set types, use u.pElementType;
 * for other custom types, use u.idlType.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_TypeInfo {
    OH_AbilityRuntime_MoDispatcher_ValueType vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    union {
        struct {
            OH_AbilityRuntime_MoDispatcher_ValueType keyType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
            struct OH_AbilityRuntime_MoDispatcher_TypeInfo *pValueType;
        } mapType;
        struct {
            struct OH_AbilityRuntime_MoDispatcher_TypeInfo *pElementType;
            uint32_t size;
        } arrayType;
        struct OH_AbilityRuntime_MoDispatcher_TypeInfo *pElementType;
        char* idlType;
    } u;
} OH_AbilityRuntime_MoDispatcher_TypeInfo;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher* OH_AbilityRuntime_MoDispatcherHandle;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_Array structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_Array;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher_Array.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_Array* OH_AbilityRuntime_MoDispatcher_ArrayHandle;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_Vector structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_Vector;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher_Vector.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_Vector* OH_AbilityRuntime_MoDispatcher_VectorHandle;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_Set structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_Set;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher_Set.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_Set* OH_AbilityRuntime_MoDispatcher_SetHandle;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_Map structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_Map;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher_Map.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_Map* OH_AbilityRuntime_MoDispatcher_MapHandle;

/**
 * @brief Forward declaration of OH_AbilityRuntime_MoDispatcher_Struct structure.
 *
 * @since 26.0.0
 */
struct OH_AbilityRuntime_MoDispatcher_Struct;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_MoDispatcher_Struct.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_MoDispatcher_Struct* OH_AbilityRuntime_MoDispatcher_StructHandle;

/**
 * @brief Defines a variant structure using union + type tag for ABI compatibility.
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Variant type.
     */
    OH_AbilityRuntime_MoDispatcher_ValueType vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    /**
     * @brief Reserved field 1 for future use.
     */
    uint64_t reserved1;
    /**
     * @brief Reserved field 2 for future use.
     */
    uint64_t reserved2;
    /**
     * @brief Reserved field 3 for future use.
     */
    uint64_t reserved3;
    /**
     * @brief Variant value data.
     */
    union {
        /**
         * @brief Empty value.
         */
        void *pvoidVal;
        /**
         * @brief Boolean value.
         */
        bool boolVal;
        /**
         * @brief 8-bit signed integer.
         */
        int8_t i8Val;
        /**
         * @brief 16-bit signed integer.
         */
        int16_t i16Val;
        /**
         * @brief 32-bit signed integer.
         */
        int32_t i32Val;
        /**
         * @brief 64-bit signed integer.
         */
        int64_t i64Val;
        /**
         * @brief 8-bit unsigned integer.
         */
        uint8_t u8Val;
        /**
         * @brief 16-bit unsigned integer.
         */
        uint16_t u16Val;
        /**
         * @brief 32-bit unsigned integer.
         */
        uint32_t u32Val;
        /**
         * @brief 64-bit unsigned integer.
         */
        uint64_t u64Val;
        /**
         * @brief 32-bit floating point.
         */
        float f32Val;
        /**
         * @brief 64-bit floating point.
         */
        double f64Val;
        /**
         * @brief Enum value.
         */
        int32_t enumVal;
        /**
         * @brief String value (UTF-8) pointer.
         */
        char* bstrVal;
        /**
         * @brief Array handle.
         */
        OH_AbilityRuntime_MoDispatcher_ArrayHandle parrayVal;
        /**
         * @brief Vector handle.
         */
        OH_AbilityRuntime_MoDispatcher_VectorHandle pvectorVal;
        /**
         * @brief Set handle.
         */
        OH_AbilityRuntime_MoDispatcher_SetHandle psetVal;
        /**
         * @brief Map handle.
         */
        OH_AbilityRuntime_MoDispatcher_MapHandle pmapVal;
        /**
         * @brief Struct handle.
         */
        OH_AbilityRuntime_MoDispatcher_StructHandle pstructVal;
        /**
         * @brief Ipc remote proxy handle.
         */
        OHIPCRemoteProxy *premoteProxyVal;
        /**
         * @brief Ipc remote stub handle.
         */
        OHIPCRemoteStub *premoteStubVal;
    } u;
} OH_AbilityRuntime_MoDispatcher_Variant;

/**
 * @brief Defines a parameter structure for method invocation.
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Array of argument variants.
     */
    OH_AbilityRuntime_MoDispatcher_Variant* rgvarg;
    /**
     * @brief Number of arguments.
     */
    uint32_t cArgs;
} OH_AbilityRuntime_MoDispatcher_InputParams;

/**
 * @brief Clear TypeInfo resources.
 *
 * Recursively release any heap resources held by a TypeInfo struct
 * (idlType strings, child TypeInfo nodes for map/array/vector/set types).
 * After clearing, all pointers are set to NULL but the TypeInfo struct itself is not freed
 * (it is typically stack-allocated by the caller).
 *
 * TypeInfo_Clear must NOT be called on a shallow copy of another TypeInfo.
 * If TypeInfo t2 = t1 is performed, only clear one of them.
 *
 * @param pTypeInfo Indicates a pointer to TypeInfo to clear.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_TypeInfo_Clear(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* pTypeInfo);

/**
 * @brief Clear variant resources.
 *
 * Release any resources held by the variant (strings, container handles, etc.).
 * After clearing, the variant is reset to VT_EMPTY with all fields zeroed.
 *
 * Ownership rules:
 * - When a Variant is passed to a function (e.g. Array_Set, Map_Put), the function
 *   performs a deep copy. The caller retains ownership of the original Variant and
 *   is responsible for freeing its own resources (e.g. free(bstrVal) for strings,
 *   Release for container handles).
 * - When a Variant is returned from a function (e.g. Array_Get, Map_Get, CallMethod),
 *   the function performs a deep copy and the caller owns the returned Variant.
 *   The caller must call Variant_Clear exactly once to release the resources.
 * - Simple types (bool, i32, f64, etc.) do not hold heap resources and do not
 *   require Variant_Clear, though calling it is harmless.
 * - Variant_Clear must NOT be called on a shallow copy of another Variant.
 *   If Variant v2 = v1 is performed, only clear one of them.
 *
 * @param pVariant Indicates a pointer to variant to clear.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Variant_Clear(
    OH_AbilityRuntime_MoDispatcher_Variant* pVariant);

/**
 * @brief Create a modular object dispatcher instance from an IPC remote proxy.
 *
 * @param remoteProxy Indicates IPC remote proxy handle obtained from connectExtension.
 * @param ppMoDispatcher Indicates a pointer to receive modular object dispatcher handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if remoteProxy or ppMoDispatcher is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CreateInstance(
    OHIPCRemoteProxy* remoteProxy, OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher);

/**
 * @brief Release modular object dispatcher instance.
 *
 * @param ppMoDispatcher Indicates a pointer to modular object dispatcher handle to release.
 *                   After release, handle will be set to NULL.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Release(OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher);

/**
 * @brief Check if type library is supported.
 *
 * @param pMoDispatcher Indicates modular object dispatcher handle.
 * @param pctinfo Indicates a pointer to receive support status (1 if supported, 0 if not supported).
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMoDispatcher or pctinfo is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t* pctinfo);

/**
 * @brief Get type descriptor.
 *
 * @param pMoDispatcher Indicates modular object dispatcher handle.
 * @param ppTypeDescriptor Indicates a pointer to receive type descriptor handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMoDispatcher or ppTypeDescriptor is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED} if send request failed.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher,
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle* ppTypeDescriptor);

/**
 * @brief Query member IDs of names.
 *
 * @param pMoDispatcher Indicates modular object dispatcher handle.
 * @param rgszNames Indicates array of property or method names.
 * @param cNames Indicates number of names.
 * @param pMemID Indicates pointer to receive member IDs (MemberIDs).
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMoDispatcher or rgszNames or pMemID is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if name not found.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, const char** rgszNames, uint32_t cNames,
    uint32_t* pMemID);

/**
 * @brief Call a method.
 *
 * @param pMoDispatcher Indicates modular object dispatcher handle.
 * @param memID Indicates method member ID (MemberID).
 * @param pInputParams Indicates parameter structure containing arguments.
 * @param pResult Indicates pointer to receive result variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMoDispatcher or pInputParams or pResult is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if method not found.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if parameter type mismatches.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED} if send request failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CallMethod(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t memID,
    OH_AbilityRuntime_MoDispatcher_InputParams* pInputParams,
    OH_AbilityRuntime_MoDispatcher_Variant* pResult);

// ========== TypeDescriptor Interfaces ==========

/**
 * @brief Release TypeDescriptor instance.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_TypeDescriptor_Release(OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle *pTypeDescriptor);

/**
 * @brief Get version.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrVersion Indicates a buffer to receive version.
 * @param cMaxVersion Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrVersion is null,
 *         or cMaxVersion is 0.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetVersion(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrVersion, uint32_t cMaxVersion);

/**
 * @brief Get total number of interfaces.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pcInterfaces Indicates a pointer to receive interface count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pcInterfaces is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcInterfaces);

/**
 * @brief Get interface name by index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param index Indicates interface index.
 * @param pbstrName Indicates a buffer to receive interface name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null, or cMaxName is 0, or index is out of range.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Check if interface is a callback type.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrName Indicates interface name.
 * @param pIsCallback Indicates a pointer to receive callback flag (true if callback, false otherwise).
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName or pIsCallback is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, bool* pIsCallback);

/**
 * @brief Get main service interface name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrName Indicates a buffer to receive main service interface name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null, or cMaxName is 0.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if main service interface not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get method count from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pcMethods Indicates a pointer to receive method count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName
 *         or pcMethods is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t* pcMethods);

/**
 * @brief Get method name by index from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param index Indicates method index.
 * @param pbstrName Indicates a buffer to receive method name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *         pbstrName is null, or cMaxName is 0, or index is out of range.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t index, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get method MemberID by name from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param pMemID Indicates a pointer to receive method MemberID.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *              pbstrMethodName or pMemID is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pMemID);

/**
 * @brief Get method return type by name from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param pReturnType Indicates a pointer to receive return type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *         pbstrMethodName or pReturnType is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    OH_AbilityRuntime_MoDispatcher_TypeInfo* pReturnType);

/**
 * @brief Get method parameter count by name from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param pcParams Indicates a pointer to receive parameter count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName
 *         or pbstrMethodName or pcParams is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pcParams);

/**
 * @brief Get method parameter type by name and index from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param iParamIndex Indicates parameter index.
 * @param pParamType Indicates a pointer to receive parameter type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *         pbstrMethodName or pParamType is null, or iParamIndex is out of range.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, OH_AbilityRuntime_MoDispatcher_TypeInfo* pParamType);

/**
 * @brief Get method parameter name by name and index from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param iParamIndex Indicates parameter index.
 * @param pbstrName Indicates a buffer to receive parameter name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName
 *         or pbstrMethodName or pbstrName is null, or iParamIndex is out of range.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, char* pbstrName, uint32_t cMaxName);

// ========== Enum Queries ==========

/**
 * @brief Get enum count.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pcEnums Indicates a pointer to receive enum count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pcEnums is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcEnums);

/**
 * @brief Get enum name by index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param index Indicates enum index.
 * @param pbstrName Indicates a buffer to receive enum name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null, or cMaxName is 0,
 *         or index is out of range.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index,
    char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get enum value count by enum name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrEnumName Indicates enum name.
 * @param pcValues Indicates a pointer to receive enum value count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrEnumName or pcValues is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if enum not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t* pcValues);

/**
 * @brief Get enum value name by enum name and value index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrEnumName Indicates enum name.
 * @param iValueIndex Indicates enum value index.
 * @param pbstrValueName Indicates a buffer to receive enum value name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrEnumName or
 *         pbstrValueName is null, or iValueIndex is out of range.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t iValueIndex, char* pbstrValueName, uint32_t cMaxName);

/**
 * @brief Get enum value by enum name and value name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrEnumName Indicates enum name.
 * @param pbstrValueName Indicates enum value name.
 * @param pValue Indicates a pointer to receive enum value.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrEnumName or
 *         pbstrValueName or pValue is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if enum value not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    const char* pbstrValueName, int32_t* pValue);

/**
 * @brief Get struct count.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pcStructs Indicates a pointer to receive struct count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pcStructs is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcStructs);

/**
 * @brief Get struct name by index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param index Indicates struct index.
 * @param pbstrName Indicates a buffer to receive struct name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null,
 *         or cMaxName is 0, or index is out of range.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index,
    char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get struct field count by struct name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrStructName Indicates struct name.
 * @param pcFields Indicates a pointer to receive field count.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrStructName or pcFields is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if struct not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t* pcFields);

/**
 * @brief Get struct field name by struct name and field index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrStructName Indicates struct name.
 * @param iFieldIndex Indicates field index.
 * @param pbstrFieldName Indicates a buffer to receive field name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrStructName or
 *         pbstrFieldName is null, or iFieldIndex is out of range.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t iFieldIndex, char* pbstrFieldName, uint32_t cMaxName);

/**
 * @brief Get struct field type by struct name and field name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrStructName Indicates struct name.
 * @param pbstrFieldName Indicates field name.
 * @param pFieldType Indicates a pointer to receive field type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrStructName or
 *         pbstrFieldName or pFieldType is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if field not found.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    const char* pbstrFieldName, OH_AbilityRuntime_MoDispatcher_TypeInfo* pFieldType);

// ========== Complex Type Operations ==========

/**
 * @brief Create an array instance.
 *
 * @param elementType Indicates element type.
 * @param size Indicates initial array size.
 * @param ppArray Indicates a pointer to receive array handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if ppArray is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Create(
    OH_AbilityRuntime_MoDispatcher_TypeInfo *elementType, uint32_t size,
    OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray);

/**
 * @brief Get array element type.
 *
 * @param pArray Indicates array handle.
 * @param pElementType Indicates a pointer to receive element type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pElementType is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetElementType(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType);

/**
 * @brief Set an array element value.
 *
 * @param pArray Indicates array handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pValue is null, or index is out of bounds.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if element type mismatches.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Set(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Get an array element value.
 *
 * @param pArray Indicates array handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to receive element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pValue is null, or index is out of bounds.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Get(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Get array size.
 *
 * @param pArray Indicates array handle.
 * @param pSize Indicates a pointer to receive array size.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pSize is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetSize(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t* pSize);

/**
 * @brief Release array instance.
 *
 * @param ppArray Indicates array handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Array_Release(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray);

/**
 * @brief Create a vector instance.
 *
 * @param elementType Indicates element type.
 * @param ppVector Indicates a pointer to receive vector handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if ppVector is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Create(
    OH_AbilityRuntime_MoDispatcher_TypeInfo *elementType, OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector);

/**
 * @brief Get vector element type.
 *
 * @param pVector Indicates vector handle.
 * @param pElementType Indicates a pointer to receive element type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pElementType is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetElementType(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType);

/**
 * @brief Add an element to vector.
 *
 * @param pVector Indicates vector handle.
 * @param pValue Indicates a pointer to element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pValue is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if element type mismatches.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Add(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, const OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Get a vector element value.
 *
 * @param pVector Indicates vector handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to receive element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pValue is null, or index is out of bounds.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Get(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Get vector size.
 *
 * @param pVector Indicates vector handle.
 * @param pSize Indicates a pointer to receive vector size.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pSize is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetSize(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t* pSize);

/**
 * @brief Clear vector.
 *
 * @param pVector Indicates vector handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Clear(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector);

/**
 * @brief Release vector instance.
 *
 * @param ppVector Indicates vector handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Vector_Release(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector);

/**
 * @brief Create a set instance.
 *
 * @param elementType Indicates element type.
 * @param ppSet Indicates a pointer to receive set handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if ppSet is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Create(
    OH_AbilityRuntime_MoDispatcher_TypeInfo *elementType, OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet);

/**
 * @brief Get set element type.
 *
 * @param pSet Indicates set handle.
 * @param pElementType Indicates a pointer to receive element type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pElementType is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetElementType(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType);

/**
 * @brief Add an element to set.
 *
 * @param pSet Indicates set handle.
 * @param pValue Indicates a pointer to element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if element type mismatches.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Add(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Remove an element from set.
 *
 * @param pSet Indicates set handle.
 * @param pValue Indicates a pointer to element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Remove(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Check if an element exists in set.
 *
 * @param pSet Indicates set handle.
 * @param pValue Indicates a pointer to element value variant.
 * @param pExists Indicates a pointer to receive existence flag.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue or pExists is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Contains(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists);

/**
 * @brief Get set size.
 *
 * @param pSet Indicates set handle.
 * @param pSize Indicates a pointer to receive set size.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pSize is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetSize(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t* pSize);

/**
 * @brief Get a set element value by index.
 *
 * The returned variant is a deep copy owned by the caller.
 * Caller must call {@link OH_AbilityRuntime_MoDispatcher_Variant_Clear} to release it.
 *
 * @param pSet Indicates set handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to receive element value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue is null, or index is out of bounds.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetAt(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Clear set.
 *
 * @param pSet Indicates set handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Clear(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet);

/**
 * @brief Release set instance.
 *
 * @param ppSet Indicates set handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Set_Release(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet);

/**
 * @brief Create a map instance.
 *
 * @param keyType Indicates key type.
 * @param valueType Indicates value type.
 * @param ppMap Indicates a pointer to receive map handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if ppMap is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Create(
    OH_AbilityRuntime_MoDispatcher_ValueType keyType, OH_AbilityRuntime_MoDispatcher_TypeInfo *valueType,
    OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap);

/**
 * @brief Get map key type.
 *
 * @param pMap Indicates map handle.
 * @param pKeyType Indicates a pointer to receive key type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKeyType is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetKeyType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_ValueType* pKeyType);

/**
 * @brief Get map value type.
 *
 * @param pMap Indicates map handle.
 * @param pValueType Indicates a pointer to receive value type.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pValueType is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetValueType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_TypeInfo* pValueType);

/**
 * @brief Put a key-value pair into map.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @param pValue Indicates a pointer to value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey or pValue is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if key or value type mismatches.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Put(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Get a value from map by key.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @param pValue Indicates a pointer to receive value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey or pValue is null, or key is
 *         not found in map.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Get(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Remove a key-value pair from map.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Remove(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey);

/**
 * @brief Check if a key exists in map.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @param pExists Indicates a pointer to receive existence flag.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey or pExists is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_ContainsKey(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists);

/**
 * @brief Get map size.
 *
 * @param pMap Indicates map handle.
 * @param pSize Indicates a pointer to receive map size.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pSize is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetSize(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t* pSize);

/**
 * @brief Get a map key by index.
 *
 * The returned variant is a deep copy owned by the caller.
 * Caller must call {@link OH_AbilityRuntime_MoDispatcher_Variant_Clear} to release it.
 *
 * @param pMap Indicates map handle.
 * @param index Indicates entry index.
 * @param pKey Indicates a pointer to receive key variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey is null, or index is out of bounds.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetKeyAt(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pKey);

/**
 * @brief Get a map value by index.
 *
 * The returned variant is a deep copy owned by the caller.
 * Caller must call {@link OH_AbilityRuntime_MoDispatcher_Variant_Clear} to release it.
 *
 * @param pMap Indicates map handle.
 * @param index Indicates entry index.
 * @param pValue Indicates a pointer to receive value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pValue is null, or index is out of bounds.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetValueAt(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Clear map.
 *
 * @param pMap Indicates map handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap is null.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Clear(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap);

/**
 * @brief Release map instance.
 *
 * @param ppMap Indicates map handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Map_Release(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap);

/**
 * @brief Create a struct instance.
 *
 * @param structName Indicates struct name from type metadata.
 * @param ppStruct Indicates a pointer to receive struct handle.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if structName or ppStruct is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_Create(
    const char* structName, OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct);

/**
 * @brief Get struct name.
 *
 * @param pStruct Indicates struct handle.
 * @param pbstrName Indicates a buffer to receive struct name.
 * @param cMaxName Indicates size of buffer.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pStruct or pbstrName is null, or cMaxName is 0.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetName(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Set a struct field value.
 *
 * @param pStruct Indicates struct handle.
 * @param szName Indicates field name.
 * @param pValue Indicates a pointer to field value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pStruct or szName or pValue is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if field is not found in struct.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if field type mismatches.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_SetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Get a struct field value.
 *
 * @param pStruct Indicates struct handle.
 * @param szName Indicates field name.
 * @param pValue Indicates a pointer to receive field value variant.
 * @return Returns error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pStruct or szName or pValue is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if field is not found in struct.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue);

/**
 * @brief Release struct instance.
 *
 * @param ppStruct Indicates struct handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_MoDispatcher_Struct_Release(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct);
#ifdef __cplusplus
}
#endif

#endif // OHOS_ABILITY_RUNTIME_C_MO_DISPATCHER_H
