/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under License is distributed on an "AS IS" BASIS,
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

#ifndef OHOS_ABILITY_RUNTIME_C_MOD_OBJ_DISPATCHER_H
#define OHOS_ABILITY_RUNTIME_C_MOD_OBJ_DISPATCHER_H

#include <stdbool.h>
#include "ability_runtime_common.h"
#include "ipc_cparcel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher_TypeDescriptor.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher_TypeDescriptor*
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle;

 /**
  * @brief Enumerates variant types.
  *
  * @since 26.0.0
  */
typedef enum {
    /**
     * @brief Empty value.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY = 0,
    /**
     * @brief Void value.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID = 1,
    /**
     * @brief Boolean value.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL = 2,
    /**
     * @brief 8-bit signed integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8 = 3,
    /**
     * @brief 16-bit signed integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16 = 4,
    /**
     * @brief 32-bit signed integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32 = 5,
    /**
     * @brief 64-bit signed integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64 = 6,
    /**
     * @brief 8-bit unsigned integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8 = 7,
    /**
     * @brief 16-bit unsigned integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16 = 8,
    /**
     * @brief 32-bit unsigned integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32 = 9,
    /**
     * @brief 64-bit unsigned integer.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64 = 10,
    /**
     * @brief 32-bit floating point.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32 = 11,
    /**
     * @brief 64-bit floating point.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64 = 12,
    /**
     * @brief String (UTF-8).
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING = 13,
    /**
     * @brief Array.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY = 14,
    /**
     * @brief Vector.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR = 15,
    /**
     * @brief Set.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET = 16,
    /**
     * @brief Map.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP = 17,
    /**
     * @brief Struct.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT = 18,
    /**
     * @brief Ipc Remote Proxy.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY = 19,
    /**
     * @brief Ipc Remote Stub.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB = 20,
    /**
     * @brief Enum value.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM = 21,
} OH_AbilityRuntime_ModObjDispatcher_ValueType;

/**
 * @brief Defines the parameter type descriptor for modular object dispatcher.
 *
 * Describes the type of a parameter or return value using a tagged union.
  * for array types, use u.arrayType.pElementType and u.arrayType.size;
  * for vector/set types, use u.pElementType;
  * for struct/proxy/stub/enum types, use u.idlType.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModObjDispatcher_TypeInfo {
    /**
     * @brief Type tag that determines which union member is valid.
     * @since 26.0.0
     */
    OH_AbilityRuntime_ModObjDispatcher_ValueType vt;
    /**
     * @brief Union of type-specific metadata. The valid member is determined by {@link vt}.
     * @since 26.0.0
     */
    union {
        /**
         * @brief Map type metadata.
         *         Used when vt is {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP}.
         *
         * @since 26.0.0
         */
        struct {
            /**
             * @brief Key type of the map. Only basic types are supported.
             *        Container types (ARRAY, VECTOR, SET, MAP) and complex types
             *        (STRUCT, IPC_REMOTE_PROXY, IPC_REMOTE_STUB) are not supported.
             *
             * @since 26.0.0
             */
            OH_AbilityRuntime_ModObjDispatcher_ValueType keyType;
            /**
             * @brief Pointer to the value type descriptor.
             *         Must be released by {@link OH_AbilityRuntime_ModObjDispatcher_TypeInfo_Clear}.
             *
             * @since 26.0.0
             */
            OH_AbilityRuntime_ModObjDispatcher_TypeInfo *pValueType;
        } mapType;
        /**
         * @brief Array type metadata.
         *         Used when vt is {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY}.
         *
         * @since 26.0.0
         */
        struct {
            /**
             * @brief Pointer to the element type descriptor.
             *         Must be released by {@link OH_AbilityRuntime_ModObjDispatcher_TypeInfo_Clear}.
             *
             * @since 26.0.0
             */
            struct OH_AbilityRuntime_ModObjDispatcher_TypeInfo *pElementType;
            /**
             * @brief Fixed array size.
             * @since 26.0.0
             */
            uint32_t size;
        } arrayType;
        /**
         * @brief Pointer to the element type descriptor.
         *         Used when vt is {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR}
         *         or {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET}.
         *         Must be released by {@link OH_AbilityRuntime_ModObjDispatcher_TypeInfo_Clear}.
         *
         * @since 26.0.0
         */
        OH_AbilityRuntime_ModObjDispatcher_TypeInfo *pElementType;
        /**
         * @brief IDL type name string (heap-allocated).
         *         Used when vt is {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT},
         *         {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY},
         *         {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB},
         *         or {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM}.
         *         Must be released by {@link OH_AbilityRuntime_ModObjDispatcher_TypeInfo_Clear}.
         *
         * @since 26.0.0
         */
        char* idlType;
    } u;
} OH_AbilityRuntime_ModObjDispatcher_TypeInfo;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher* OH_AbilityRuntime_ModObjDispatcherHandle;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher_Array.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher_Array* OH_AbilityRuntime_ModObjDispatcher_ArrayHandle;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher_Vector.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher_Vector* OH_AbilityRuntime_ModObjDispatcher_VectorHandle;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher_Set.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher_Set* OH_AbilityRuntime_ModObjDispatcher_SetHandle;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher_Map.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher_Map* OH_AbilityRuntime_ModObjDispatcher_MapHandle;

/**
 * @brief Defines a pointer to OH_AbilityRuntime_ModularObjectDispatcher_Struct.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectDispatcher_Struct* OH_AbilityRuntime_ModObjDispatcher_StructHandle;

/**
 * @brief Defines a variant structure using union + type tag for ABI compatibility.
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Variant type.
     * @since 26.0.0
     */
    OH_AbilityRuntime_ModObjDispatcher_ValueType vt;
    /**
     * @brief Reserved field 1 for future use.
     * @since 26.0.0
     */
    uint64_t reserved1;
    /**
     * @brief Reserved field 2 for future use.
     * @since 26.0.0
     */
    uint64_t reserved2;
    /**
     * @brief Reserved field 3 for future use.
     * @since 26.0.0
     */
    uint64_t reserved3;
    /**
     * @brief Variant value data.
     * @since 26.0.0
     */
    union {
        /**
         * @brief Empty value.
         * @since 26.0.0
         */
        void *pvoidVal;
        /**
         * @brief Boolean value.
         * @since 26.0.0
         */
        bool boolVal;
        /**
         * @brief 8-bit signed integer.
         * @since 26.0.0
         */
        int8_t i8Val;
        /**
         * @brief 16-bit signed integer.
         * @since 26.0.0
         */
        int16_t i16Val;
        /**
         * @brief 32-bit signed integer.
         * @since 26.0.0
         */
        int32_t i32Val;
        /**
         * @brief 64-bit signed integer.
         * @since 26.0.0
         */
        int64_t i64Val;
        /**
         * @brief 8-bit unsigned integer.
         * @since 26.0.0
         */
        uint8_t u8Val;
        /**
         * @brief 16-bit unsigned integer.
         * @since 26.0.0
         */
        uint16_t u16Val;
        /**
         * @brief 32-bit unsigned integer.
         * @since 26.0.0
         */
        uint32_t u32Val;
        /**
         * @brief 64-bit unsigned integer.
         * @since 26.0.0
         */
        uint64_t u64Val;
        /**
         * @brief 32-bit floating point.
         * @since 26.0.0
         */
        float f32Val;
        /**
         * @brief 64-bit floating point.
         * @since 26.0.0
         */
        double f64Val;
        /**
         * @brief Enum value.
         * @since 26.0.0
         */
        int32_t enumVal;
        /**
         * @brief String value (UTF-8) pointer.
         * @since 26.0.0
         */
        char* bstrVal;
        /**
         * @brief Array handle.
         * @since 26.0.0
         */
        OH_AbilityRuntime_ModObjDispatcher_ArrayHandle parrayVal;
        /**
         * @brief Vector handle.
         * @since 26.0.0
         */
        OH_AbilityRuntime_ModObjDispatcher_VectorHandle pvectorVal;
        /**
         * @brief Set handle.
         * @since 26.0.0
         */
        OH_AbilityRuntime_ModObjDispatcher_SetHandle psetVal;
        /**
         * @brief Map handle.
         * @since 26.0.0
         */
        OH_AbilityRuntime_ModObjDispatcher_MapHandle pmapVal;
        /**
         * @brief Struct handle.
         * @since 26.0.0
         */
        OH_AbilityRuntime_ModObjDispatcher_StructHandle pstructVal;
        /**
         * @brief Ipc remote proxy handle.
         * @since 26.0.0
         */
        OHIPCRemoteProxy *premoteProxyVal;
        /**
         * @brief Ipc remote stub handle.
         * @since 26.0.0
         */
        OHIPCRemoteStub *premoteStubVal;
    } u;
} OH_AbilityRuntime_ModObjDispatcher_Variant;

/**
 * @brief Defines a parameter structure for method invocation.
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Array of argument variants.
     * @since 26.0.0
     */
    OH_AbilityRuntime_ModObjDispatcher_Variant* rgvarg;
    /**
     * @brief Number of arguments.
     * @since 26.0.0
     */
    uint32_t cArgs;
} OH_AbilityRuntime_ModObjDispatcher_InputParams;

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
void OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pTypeInfo);

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
void OH_AbilityRuntime_ModObjDispatcher_VariantClear(
    OH_AbilityRuntime_ModObjDispatcher_Variant* pVariant);

/**
 * @brief Create a modular object dispatcher instance from an IPC remote proxy for the main service interface.
 *
 * The type library metadata will be lazily loaded from the remote service on the first call
 * that requires it, such as HasTypeDescriptor, QueryMainServiceInterfaceMemIDsOfNames, or CallMethod.
 *
 * @param remoteProxy Indicates IPC remote proxy handle obtained from connectExtension.
 * @param ppModObjDispatcher Indicates a pointer to receive modular object dispatcher handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if remoteProxy or ppModObjDispatcher is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_CreateMainServiceInstance(
    OHIPCRemoteProxy* remoteProxy, OH_AbilityRuntime_ModObjDispatcherHandle* ppModObjDispatcher);

/**
 * @brief Create a sub-instance dispatcher bound to a mainService dispatcher.
 *
 * The sub-instance shares the mainService dispatcher's metadata but uses its own
 * IPC proxy. When CallMethod is invoked on the sub-instance, method metadata is
 * resolved from the mainService dispatcher and the call is sent through subProxy.
 *
 * @param mainServiceDispatcher Indicates mainService dispatcher handle.
 * @param subProxy Indicates IPC remote proxy for the non-mainService interface.
 * @param ppModObjDispatcher Indicates a pointer to receive the created sub-instance dispatcher handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if mainServiceDispatcher or subProxy
 *      or ppModObjDispatcher is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_CreateSubInstance(
    OH_AbilityRuntime_ModObjDispatcherHandle mainServiceDispatcher,
    OHIPCRemoteProxy* subProxy, OH_AbilityRuntime_ModObjDispatcherHandle* ppModObjDispatcher);

/**
 * @brief Release modular object dispatcher instance.
 *
 * @param ppModObjDispatcher Indicates a pointer to modular object dispatcher handle to release.
 *                   After release, handle will be set to NULL.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ModObjDispatcher_Release(OH_AbilityRuntime_ModObjDispatcherHandle* ppModObjDispatcher);

/**
 * @brief Check if the type library metadata is available from the remote service.
 *
 * @param pModObjDispatcher Indicates modular object dispatcher handle.
 * @param pctinfo Indicates a pointer to receive support status (1 if supported, 0 if not supported).
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pModObjDispatcher or pctinfo is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED} if send request failed.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID} if type library metadata is invalid.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_HasTypeDescriptor(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher, uint32_t* pctinfo);

/**
 * @brief Get type descriptor for querying interface metadata information.
 *
 * The type descriptor provides access to type library metadata including interfaces,
 * methods, enums, and structs defined in the remote service's type library.
 * Must call {@link OH_AbilityRuntime_TypeDescriptor_Release} to release the handle when no longer needed.
 *
 * @param pModObjDispatcher Indicates modular object dispatcher handle.
 * @param ppTypeDescriptor Indicates a pointer to receive type descriptor handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pModObjDispatcher or ppTypeDescriptor is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED} if send request failed.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID} if type library metadata is invalid.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_GetTypeDescriptor(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher,
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle* ppTypeDescriptor);

/**
 * @brief Query member IDs of method names in the main service interface.
 *
 * The returned member IDs can be used as the memID parameter in
 * {@link OH_AbilityRuntime_ModObjDispatcher_CallMethod}.
 *
 * @param pModObjDispatcher Indicates modular object dispatcher handle.
 * @param rgszNames Indicates array of method names.
 * @param cNames Indicates number of names.
 * @param pMemID Indicates pointer to receive member IDs (MemberIDs).
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pModObjDispatcher or rgszNames or pMemID is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED} if send request failed.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID} if type library metadata is invalid.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if name not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher, const char** rgszNames, uint32_t cNames,
    uint32_t* pMemID);

/**
 * @brief Call a method.
 *
 * @param pModObjDispatcher Indicates modular object dispatcher handle.
 * @param memID Indicates method member ID (MemberID).
 * @param pInputParams Indicates parameter structure containing arguments.
 * @param pResult Indicates pointer to receive result variant.
 * @param pMethodErrCode Indicates a pointer to receive the error code returned by the remote method.
 *                   0 if the method executed successfully, non-zero if the method returned an error.
 *                   This is independent of the framework-level return value.
 *                   This parameter can be NULL if the caller does not need the method-level error code.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if IPC call is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pModObjDispatcher or pInputParams or
 *      pResult is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if method not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if parameter type mismatches.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED} if send request failed.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID} if type library metadata is invalid.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_CallMethod(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher, uint32_t memID,
    OH_AbilityRuntime_ModObjDispatcher_InputParams* pInputParams,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pResult,
    int32_t* pMethodErrCode);

// ========== TypeDescriptor Interfaces ==========

/**
 * @brief Release TypeDescriptor instance.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_TypeDescriptor_Release(OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle *pTypeDescriptor);

/**
 * @brief Get version of the type library.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrVersion Indicates a buffer to receive version string.
 * @param cMaxVersion Indicates size of buffer in bytes, including the null terminator.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or
 *      pbstrVersion is null, or cMaxVersion is 0.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetVersion(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrVersion, uint32_t cMaxVersion);

/**
 * @brief Get total number of interfaces.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pcInterfaces Indicates a pointer to receive interface count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pcInterfaces is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcInterfaces);

/**
 * @brief Get interface name by index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param index Indicates interface index, ranging from 0 to
 *              (interface count - 1) obtained from {@link OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount}.
 * @param pbstrName Indicates a buffer to receive interface name.
 * @param cMaxName Indicates size of buffer in bytes, including the null terminator.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null,
 *      or cMaxName is 0, or index is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index,
    char* pbstrName, uint32_t cMaxName);

/**
 * @brief Check if interface is a callback type.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrName Indicates interface name.
 * @param pIsCallback Indicates a pointer to receive callback flag (true if callback, false otherwise).
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName or
 *      pIsCallback is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface not found.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, bool* pIsCallback);

/**
 * @brief Get main service interface name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrName Indicates a buffer to receive main service interface name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null,
 *      or cMaxName is 0.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get method count from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pcMethods Indicates a pointer to receive method count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or
 *      pbstrInterfaceName or pcMethods is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface not found.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t* pcMethods);

/**
 * @brief Get method name by index from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param index Indicates method index, ranging from 0 to (method count - 1)
 *              obtained from {@link OH_AbilityRuntime_TypeDescriptor_GetMethodCount}.
 * @param pbstrName Indicates a buffer to receive method name.
 * @param cMaxName Indicates size of buffer in bytes, including the null terminator.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *      pbstrName is null, or cMaxName is 0, or index is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t index, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get method MemberID by name from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param pMemID Indicates a pointer to receive method MemberID.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *      pbstrMethodName or pMemID is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pMemID);

/**
 * @brief Get method return type by name from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param pReturnType Indicates a pointer to receive return type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *      pbstrMethodName or pReturnType is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pReturnType);

/**
 * @brief Get method parameter count by name from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param pcParams Indicates a pointer to receive parameter count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *      pbstrMethodName or pcParams is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pcParams);

/**
 * @brief Get method parameter type by name and index from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param iParamIndex Indicates parameter index.
 * @param pParamType Indicates a pointer to receive parameter type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *      pbstrMethodName or pParamType is null, or iParamIndex is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pParamType);

/**
 * @brief Get method parameter name by name and index from interface.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrInterfaceName Indicates interface name.
 * @param pbstrMethodName Indicates method name.
 * @param iParamIndex Indicates parameter index.
 * @param pbstrName Indicates a buffer to receive parameter name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrInterfaceName or
 *      pbstrMethodName or pbstrName is null, or iParamIndex is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if interface or method not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, char* pbstrName, uint32_t cMaxName);

// ========== Enum Queries ==========

/**
 * @brief Get enum count.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pcEnums Indicates a pointer to receive enum count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pcEnums is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcEnums);

/**
 * @brief Get enum name by index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param index Indicates enum index.
 * @param pbstrName Indicates a buffer to receive enum name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null,
 *      or cMaxName is 0, or index is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index,
    char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get enum value count by enum name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrEnumName Indicates enum name.
 * @param pcValues Indicates a pointer to receive enum value count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrEnumName or
 *      pcValues is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if enum not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t* pcValues);

/**
 * @brief Get enum value name by enum name and value index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrEnumName Indicates enum name.
 * @param iValueIndex Indicates enum value index.
 * @param pbstrValueName Indicates a buffer to receive enum value name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrEnumName or pbstrValueName
 *      is null, or iValueIndex is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if enum not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t iValueIndex, char* pbstrValueName, uint32_t cMaxName);

/**
 * @brief Get enum value by enum name and value name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrEnumName Indicates enum name.
 * @param pbstrValueName Indicates enum value name.
 * @param pValue Indicates a pointer to receive enum value.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrEnumName or
 *      pbstrValueName or pValue is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if enum value not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    const char* pbstrValueName, int32_t* pValue);

/**
 * @brief Get struct count.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pcStructs Indicates a pointer to receive struct count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pcStructs is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcStructs);

/**
 * @brief Get struct name by index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param index Indicates struct index.
 * @param pbstrName Indicates a buffer to receive struct name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrName is null, or cMaxName is 0,
 *      or index is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index,
    char* pbstrName, uint32_t cMaxName);

/**
 * @brief Get struct field count by struct name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrStructName Indicates struct name.
 * @param pcFields Indicates a pointer to receive field count.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrStructName or
 *      pcFields is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if struct not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t* pcFields);

/**
 * @brief Get struct field name by struct name and field index.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrStructName Indicates struct name.
 * @param iFieldIndex Indicates field index.
 * @param pbstrFieldName Indicates a buffer to receive field name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrStructName or
 *      pbstrFieldName is null, or iFieldIndex is out of range.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if struct not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t iFieldIndex, char* pbstrFieldName, uint32_t cMaxName);

/**
 * @brief Get struct field type by struct name and field name.
 *
 * @param pTypeDescriptor Indicates TypeDescriptor handle.
 * @param pbstrStructName Indicates struct name.
 * @param pbstrFieldName Indicates field name.
 * @param pFieldType Indicates a pointer to receive field type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pTypeDescriptor or pbstrStructName or
 *      pbstrFieldName or pFieldType is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if field not found.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    const char* pbstrFieldName, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pFieldType);

/**
 * @brief Create an array instance.
 *
 * @param elementType Indicates element type.
 * @param size Indicates initial array size.
 * @param ppArray Indicates a pointer to receive array handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if elementType or ppArray is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo *elementType, uint32_t size,
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle* ppArray);

/**
 * @brief Get array element type.
 *
 * @param pArray Indicates array handle.
 * @param pElementType Indicates a pointer to receive element type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pElementType is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType);

/**
 * @brief Set an array element value.
 *
 * @param pArray Indicates array handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pValue is null,
 *      or index is out of bounds.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if element type mismatches.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArraySet(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, uint32_t index,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Get an array element value.
 *
 * @param pArray Indicates array handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to receive element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pValue is null,
 *      or index is out of bounds.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayGet(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Get array size.
 *
 * @param pArray Indicates array handle.
 * @param pSize Indicates a pointer to receive array size.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pArray or pSize is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayGetSize(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, uint32_t* pSize);

/**
 * @brief Release array instance.
 *
 * @param ppArray Indicates array handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle* ppArray);

/**
 * @brief Create a vector instance.
 *
 * @param elementType Indicates element type.
 * @param ppVector Indicates a pointer to receive vector handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if elementType or ppVector is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo *elementType,
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle* ppVector);

/**
 * @brief Get vector element type.
 *
 * @param pVector Indicates vector handle.
 * @param pElementType Indicates a pointer to receive element type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pElementType is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType);

/**
 * @brief Add an element to vector.
 *
 * @param pVector Indicates vector handle.
 * @param pValue Indicates a pointer to element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pValue is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if element type mismatches.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorAdd(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Get a vector element value.
 *
 * @param pVector Indicates vector handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to receive element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pValue is null,
 *      or index is out of bounds.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorGet(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Get vector size.
 *
 * @param pVector Indicates vector handle.
 * @param pSize Indicates a pointer to receive vector size.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector or pSize is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorGetSize(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, uint32_t* pSize);

/**
 * @brief Clear vector.
 *
 * @param pVector Indicates vector handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pVector is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorClear(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector);

/**
 * @brief Release vector instance.
 *
 * @param ppVector Indicates vector handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ModObjDispatcher_VectorRelease(OH_AbilityRuntime_ModObjDispatcher_VectorHandle* ppVector);

/**
 * @brief Create a set instance.
 *
 * @param elementType Indicates element type.
 * @param ppSet Indicates a pointer to receive set handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if elementType or ppSet is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo *elementType, OH_AbilityRuntime_ModObjDispatcher_SetHandle* ppSet);

/**
 * @brief Get set element type.
 *
 * @param pSet Indicates set handle.
 * @param pElementType Indicates a pointer to receive element type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pElementType is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType);

/**
 * @brief Add an element to set.
 *
 * @param pSet Indicates set handle.
 * @param pValue Indicates a pointer to element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if element type mismatches.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetAdd(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Remove an element from set.
 *
 * @param pSet Indicates set handle.
 * @param pValue Indicates a pointer to element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetRemove(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Check if an element exists in set.
 *
 * @param pSet Indicates set handle.
 * @param pValue Indicates a pointer to element value variant.
 * @param pExists Indicates a pointer to receive existence flag.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue or pExists is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetContains(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue,
    bool* pExists);

/**
 * @brief Get set size.
 *
 * @param pSet Indicates set handle.
 * @param pSize Indicates a pointer to receive set size.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pSize is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetGetSize(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, uint32_t* pSize);

/**
 * @brief Get a set element value by index.
 *
 * The returned variant is a deep copy owned by the caller.
 * Caller must call {@link OH_AbilityRuntime_ModObjDispatcher_Variant_Clear} to release it.
 *
 * @param pSet Indicates set handle.
 * @param index Indicates element index.
 * @param pValue Indicates a pointer to receive element value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet or pValue is null, or index is out of bounds.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetGetAt(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Clear set.
 *
 * @param pSet Indicates set handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pSet is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetClear(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet);

/**
 * @brief Release set instance.
 *
 * @param ppSet Indicates set handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ModObjDispatcher_SetRelease(OH_AbilityRuntime_ModObjDispatcher_SetHandle* ppSet);

/**
 * @brief Create a map instance.
 *
 * @param keyType Indicates key type. Only basic types are supported, such as
 *               {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL},
 *               {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32},
 *               {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING},
 *               and {@link OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM}.
 *               Container types (ARRAY, VECTOR, SET, MAP) and complex types
 *               (STRUCT, IPC_REMOTE_PROXY, IPC_REMOTE_STUB) are not supported.
 * @param valueType Indicates value type.
 * @param ppMap Indicates a pointer to receive map handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if valueType or ppMap is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapCreate(
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType, OH_AbilityRuntime_ModObjDispatcher_TypeInfo *valueType,
    OH_AbilityRuntime_ModObjDispatcher_MapHandle* ppMap);

/**
 * @brief Get map key type.
 *
 * @param pMap Indicates map handle.
 * @param pKeyType Indicates a pointer to receive key type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKeyType is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetKeyType(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, OH_AbilityRuntime_ModObjDispatcher_ValueType* pKeyType);

/**
 * @brief Get map value type.
 *
 * @param pMap Indicates map handle.
 * @param pValueType Indicates a pointer to receive value type.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pValueType is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetValueType(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pValueType);

/**
 * @brief Put a key-value pair into map.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @param pValue Indicates a pointer to value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey or pValue is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if key or value type mismatches.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapPut(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Get a value from map by key.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @param pValue Indicates a pointer to receive value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey or pValue is null,
 *      or key is not found in map.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGet(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Remove a key-value pair from map.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapRemove(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey);

/**
 * @brief Check if a key exists in map.
 *
 * @param pMap Indicates map handle.
 * @param pKey Indicates a pointer to key variant.
 * @param pExists Indicates a pointer to receive existence flag.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey or pExists is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey,
    bool* pExists);

/**
 * @brief Get map size.
 *
 * @param pMap Indicates map handle.
 * @param pSize Indicates a pointer to receive map size.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pSize is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetSize(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, uint32_t* pSize);

/**
 * @brief Get a map key by index.
 *
 * The returned variant is a deep copy owned by the caller.
 * Caller must call {@link OH_AbilityRuntime_ModObjDispatcher_Variant_Clear} to release it.
 *
 * @param pMap Indicates map handle.
 * @param index Indicates entry index.
 * @param pKey Indicates a pointer to receive key variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pKey is null, or index is out of bounds.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetKeyAt(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pKey);

/**
 * @brief Get a map value by index.
 *
 * The returned variant is a deep copy owned by the caller.
 * Caller must call {@link OH_AbilityRuntime_ModObjDispatcher_Variant_Clear} to release it.
 *
 * @param pMap Indicates map handle.
 * @param index Indicates entry index.
 * @param pValue Indicates a pointer to receive value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap or pValue is null, or index is out of bounds.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetValueAt(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Clear map.
 *
 * @param pMap Indicates map handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pMap is null.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapClear(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap);

/**
 * @brief Release map instance.
 *
 * @param ppMap Indicates map handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ModObjDispatcher_MapRelease(OH_AbilityRuntime_ModObjDispatcher_MapHandle* ppMap);

/**
 * @brief Create a struct instance.
 *
 * @param structName Indicates struct name from type metadata.
 * @param ppStruct Indicates a pointer to receive struct handle.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if structName or ppStruct is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if internal error occurs.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructCreate(
    const char* structName, OH_AbilityRuntime_ModObjDispatcher_StructHandle* ppStruct);

/**
 * @brief Get struct name.
 *
 * @param pStruct Indicates struct handle.
 * @param pbstrName Indicates a buffer to receive struct name.
 * @param cMaxName Indicates size of buffer.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pStruct or pbstrName is null, or cMaxName is 0.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructGetName(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName);

/**
 * @brief Set a struct field value.
 *
 * @param pStruct Indicates struct handle.
 * @param szName Indicates field name.
 * @param pValue Indicates a pointer to field value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pStruct or szName or pValue is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if field is not found in struct.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH} if field type mismatches.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructSetField(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Get a struct field value.
 *
 * @param pStruct Indicates struct handle.
 * @param szName Indicates field name.
 * @param pValue Indicates a pointer to receive field value variant.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if pStruct or szName or pValue is null.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND} if field is not found in struct.</li>
 *      </ul>
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructGetField(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue);

/**
 * @brief Release struct instance.
 *
 * @param ppStruct Indicates struct handle to release.
 * @since 26.0.0
 */
void OH_AbilityRuntime_ModObjDispatcher_StructRelease(OH_AbilityRuntime_ModObjDispatcher_StructHandle* ppStruct);
#ifdef __cplusplus
}
#endif
/** @} */
#endif // OHOS_ABILITY_RUNTIME_C_MOD_OBJ_DISPATCHER_H

