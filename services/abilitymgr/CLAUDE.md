# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

**abilitymgr** (Ability Manager Service) is the core system service within the OpenHarmony ability_runtime component responsible for managing Ability component lifecycles, scheduling, state management, and cross-process communication. It serves as the central coordinator for all Ability-related operations across both FA (Feature Ability) and Stage framework models.

## Key Architecture Concepts

### Manager-Stub-Service Pattern

The service follows a three-layer architecture:

1. **Manager Layer** - Business logic and state management (e.g., `AbilityConnectManager`, `DataAbilityManager`, `UIExtensionAbilityManager`)
2. **Stub Layer** - IPC interface for client communication via `AbilityManagerStub`
3. **Service Layer** - Core service implementation via `AbilityManagerService` singleton

### Core Service Flow

```
Client Request → AbilityManagerStub → AbilityManagerService → Specific Manager → AbilityRecord/ExtensionRecord
```

### Key Design Patterns

- **Singleton Pattern** - Critical services (AMS) are implemented as singletons using `DECLARE_DELAYED_SINGLETON`
- **Interceptor Pattern** - Pluggable behaviors via `AbilityInterceptorExecuter` for ability start operations
- **Factory Pattern** - `ExtensionRecordFactory` creates different extension types
- **Observer Pattern** - Extensive use of observers for lifecycle and state change notifications

## Directory Structure

```
abilitymgr/
├── include/                   # Public interface headers
│   ├── ability_manager_stub.h       # IPC stub interface
│   ├── ability_manager_service.h     # Main service singleton
│   ├── ability_record.h             # Individual Ability state management
│   ├── ability_connect_manager.h     # Service-type Ability connections
│   ├── data_ability_manager.h        # Data Ability management
│   ├── ui_extension_ability_manager.h  # UI Extension handling
│   ├── common_extension_manager.h    # Common extension functionality
│   ├── mission/                    # Mission/stack management
│   ├── extension_record/            # Extension lifecycle management
│   ├── interceptor/                # Interceptor framework
│   ├── insight_intent/             # Intent profiling system
│   ├── keep_alive/                # Keep-alive services
│   ├── resident_process/           # Resident process management
│   ├── screen_lock/               # Screen lock handling
│   ├── ui_extension_record/       # UI extension records
│   ├── dialog_session/            # Dialog session management
│   ├── scene_board/              # Scene board / UIAbility lifecycle
│   ├── foreground_app_connection_manager/  # Foreground app connections
│   ├── connection_state_manager/   # Connection state tracking
│   ├── modal_system_dialog/        # Modal system dialog handling
│   ├── deeplink_reserve/          # Deep link reservation
│   └── utils/                    # Utility classes
├── src/                       # Implementation files (mirror include/ structure)
├── resource/                   # Configuration JSON files
│   ├── ams_service_config.json       # Main service configuration
│   ├── uiextension_picker_config.json
│   ├── deeplink_reserve_config.json
│   └── [other configs]
├── BUILD.gn                    # GN build configuration
├── abilitymgr.gni             # Source file definitions
└── etc/                       # Additional configuration (appfwk.para)
```

## Build Commands

```bash
# Build from OpenHarmony root
./build.sh --product-name <product> --build-target ability_runtime

# Build abilitymgr specifically
./build.sh --product-name <product> --build-target ams_target

# Build with graphics support
./build.sh --product-name <product> --build-target ability_runtime --define ability_runtime_graphics=true

# Build with specific features
./build.sh --product-name <product> --build-target ability_runtime --define ability_runtime_auto_fill=true
```

### Key Build Targets

- **abilityms** (`libabilityms.so`) - Main ability manager service library
- **wantagent_manager** - Want agent management library
- **mission_list** - Mission management library
- **ams_service_config** - Configuration files group

### Build Configuration Flags

| Flag | Description |
|------|-------------|
| `ability_runtime_graphics` | Enable graphics/window support |
| `ability_runtime_auto_fill` | Enable auto-fill support |
| `ability_runtime_child_process` | Enable child process support |
| `ability_runtime_screenlock_enable` | Enable screen lock integration |
| `background_task_mgr_continuous_task_enable` | Enable continuous task support |
| `ability_command_for_test` | Enable test commands |
| `ability_fault_and_exit_test` | Enable fault injection testing |
| `memmgr_override_enable` | Enable memory manager integration |
| `os_dlp_part_enabled` | Enable DLP (Data Loss Prevention) support |
| `include_app_domain_verify` | Enable app domain verification |
| `resource_schedule_service_enable` | Enable resource scheduling |

## Core Components

### AbilityManagerService (`src/ability_manager_service.cpp`)

The central singleton service that:
- Implements `SystemAbility` for system service registration
- Coordinates all Ability operations
- Manages lifecycle transitions
- Handles multi-user scenarios
- Coordinates with AppManagerService, BundleManager

**Key methods:**
- `StartAbility()`, `TerminateAbility()` - Core lifecycle management
- `ConnectAbility()`, `DisconnectAbility()` - Service connections
- `MoveMissionToFront()`, `MinimizeAbility()` - Mission management

### AbilityRecord (`src/ability_record.cpp`)

Represents a single Ability instance with:
- State management (INITIAL, ACTIVE, INACTIVE, BACKGROUND, TERMINATED)
- Lifecycle coordination via `LifecycleDeal`
- Connection tracking for service abilities
- Want/Intent processing

### AbilityConnectManager (`src/ability_connect_manager.cpp`)

Manages connections to Service-type Abilities:
- Connection lifecycle (Connect, Disconnect, Death handling)
- Callback management
- Multi-connection tracking

### DataAbilityManager (`src/data_ability_manager.cpp`)

Handles Data-type Abilities:
- URI-based data access
- Permission verification
- Cross-process data operations

### Mission Management (`src/mission/`)

- **MissionList** - Application mission stack (LAUNCHER, STANDARD)
- **MissionInfoMgr** - Mission information storage and retrieval
- **MissionDataStorage** - Persistent mission state via RDB
- **MissionListenerController** - Mission state change notifications

### Extension Management

- **ExtensionRecord** (`src/extension_record/`) - Extension lifecycle management
- **ExtensionRecordFactory** - Creates appropriate extension types
- **UIExtensionAbilityManager** - UI Extension specific handling

### Interceptor Framework (`src/interceptor/`)

Provides pluggable behavior for ability starts:

| Interceptor | Purpose |
|-------------|---------|
| `StartOtherAppInterceptor` | Control starting other applications |
| `EcologicalRuleInterceptor` | Enforce ecosystem rules |
| `KioskInterceptor` | Kiosk mode enforcement |
| `ScreenUnlockInterceptor` | Screen lock state checks |
| `ControlInterceptor` | System-level controls |
| `DisposedRuleInterceptor` | Rule-based disposal |
| `BlockAllAppStartInterceptor` | Emergency blocking |
| `AbilityJumpInterceptor` - Jump-related interception |
| `ExtensionControlInterceptor` | Extension-specific controls |
| `CrowdTestInterceptor` | Testing mode control |

**Usage:** Interceptors are registered with `AbilityInterceptorExecuter` and executed in sequence during `StartAbility()` operations.

### Specialized Features

- **InsightIntent** (`src/insight_intent/`) - Intelligent intent processing and profiling
- **KeepAlive** (`src/keep_alive/`) - Ability keep-alive services for critical processes
- **FreeInstall** (`src/free_install_manager.cpp`) - Free installation management
- **DeepLinkReserve** - Deep link reservation system
- **KioskManager** (`src/kiosk_manager.cpp`) - Kiosk mode management
- **ResidentProcessManager** - Resident process lifecycle
- **PendingWantManager** - Pending want/request code management

### Connection Management

- **ConnectionStateManager** (`src/connection_state_manager.cpp`) - Tracks connection state across services
- **ConnectionObserverController** (`src/connection_observer_controller.cpp`) - Connection state change notifications
- **ForegroundAppConnectionManager** (`src/foreground_app_connection_manager/`) - Foreground app connection tracking

### Dialog and Scene Management

- **DialogSessionManager** (`src/dialog_session/`) - Modal dialog session lifecycle
- **ModalSystemDialogUiExtension** (`src/modal_system_dialog/`) - Modal system dialog UI extension
- **UIAbilityLifecycleManager** (`src/scene_board/`) - Scene board UI ability lifecycle
- **StatusBarDelegateManager** (`src/scene_board/`) - Status bar delegation

### Additional Features

- **ImplicitStartProcessor** (`src/implicit_start_processor.cpp`) - Implicit ability start processing (requires graphics)
- **SystemDialogScheduler** (`src/system_dialog_scheduler.cpp`) - System dialog scheduling
- **DeepLinkReserve** (`src/deeplink_reserve/`) - Deep link reservation system
- **AppExitReasonHelper** (`src/app_exit_reason_helper.cpp`) - Application exit reason tracking
- **SAInterceptorManager** (`src/sa_interceptor_manager.cpp`) - System ability interception
- **ReportDataPartitionUsageManager** (`src/report_data_partition_usage_manager.cpp`) - Data partition reporting

## Configuration Files

### Service Configuration (`resource/ams_service_config.json`)

```json
{
    "service_startup_config": {
        "mission_save_time": 43200000,           // Mission persistence interval (ms)
        "root_launcher_restart_max": 15,           // Max launcher restarts
        "resident_restart_max": 3,                // Max resident restarts
        "restart_interval_time": 120000,           // Restart interval (ms)
        "app_not_response_process_timeout_time": 1000,
        "ams_timeout_time": 180,                 // Service timeout (s)
        "device_type": "phone"
    },
    "system_configuration": {
        "system_orientation": "vertical"
    },
    "supportBackToCaller": true,
    "supportSCBCrashReboot": true
}
```

## System Ability Registration

AbilityManagerService registers as a system ability with the framework:
- **Service Name**: `AbilityManagerService`
- **Listens to**: `DISTRIBUTED_SCHED_SA_ID` (Distributed Scheduler)
- **Lifecycle**: Implements `OnStart()` and `OnStop()` for service lifecycle
- **Dependencies**: Coordinates with AppManagerService, BundleManagerService via SystemAbility callbacks

## Development Guidelines

### Logging

Use `hilog_tag_wrapper.h` for consistent logging:

```cpp
#include "hilog_tag_wrapper.h"

TAG_LOGD(AAFwkTag::ABILITYMGR, "Debug: %{public}s", value);
TAG_LOGI(AAFwkTag::ABILITYMGR, "Starting ability: %{public}s", abilityName.c_str());
TAG_LOGW(AAFwkTag::ABILITYMGR, "Warning: operation may fail");
TAG_LOGE(AAFwkTag::ABILITYMGR, "Error: %{public}d", errorCode);
```

**Common tags for abilitymgr:**
- `AAFwkTag::ABILITYMGR` - General service operations
- `AAFwkTag::ABILITY` - Ability lifecycle
- `AAFwkTag::CONNECTION` - Connection management
- `AAFwkTag::UIABILITY` - UI Ability operations
- `AAFwkTag::MISSION` - Mission management

### Error Handling

Standard error codes from `ability_runtime_error_util.h`:

```cpp
#include "ability_runtime_error_util.h"

// Return error codes
return ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_ABILITY_NAME;

// Check and log
if (result != ERR_OK) {
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Operation failed: %{public}d", result);
    return result;
}
```

### Adding a New Interceptor

1. Create header in `include/interceptor/my_interceptor.h`:
```cpp
class MyInterceptor : public IAbilityInterceptor {
public:
    ErrCode DoProcess(AbilityInterceptorParam& param) override;
};
```

2. Implement in `src/interceptor/my_interceptor.cpp`

3. Register with `AbilityInterceptorExecuter` in `AbilityManagerService::Init()`:
```cpp
interceptorExecuter_->AddInterceptor("MyInterceptor",
    std::make_shared<MyInterceptor>());
```

### Adding a New Manager

1. Create header in `include/my_manager.h`
2. Implement in `src/my_manager.cpp`
3. Add member variable to `AbilityManagerService`
4. Initialize in service startup
5. Wire up to `AbilityManagerStub` if IPC is needed
6. Update `BUILD.gn` to include new source files
7. Add to `abilitymgr.gni` source list

### Thread Safety

- Use `std::shared_mutex` for concurrent read/write access
- Use `std::unique_lock` for write operations
- Use `std::shared_lock` for read operations
- Use `std::recursive_mutex` for functions that may re-enter

Example from `AbilityInterceptorExecuter`:
```cpp
std::recursive_mutex interceptorMapLock_;
std::unordered_map<std::string, std::shared_ptr<IAbilityInterceptor>> interceptorMap_;
```

## Utility Files

The `src/utils/` directory contains important helper modules:

- **ability_permission_util.cpp** - Permission checking and verification
- **start_ability_utils.cpp** - Ability start utilities
- **want_utils.cpp** - Want object manipulation
- **uri_utils.cpp** - URI parsing and validation
- **multi_instance_utils.cpp** - Multi-instance ability support
- **main_element_utils.cpp** - Main element extraction
- **app_mgr_util.cpp** - App manager integration utilities
- **hmsf_utils.cpp** - HMS framework utilities
- **modal_system_dialog_util.cpp** - Modal dialog helpers
- **dump_utils.cpp** - Dump/debug output formatting
- **exit_reason_util.cpp** - Exit reason processing
- **update_caller_info_util.cpp** - Caller information updates
- **start_options_utils.cpp** - Start options utilities
- **window_options_utils.cpp** - Window options handling
- **timeout_state_utils.cpp** - Timeout state management
- **state_utils.cpp** - State utilities
- **hidden_start_utils.cpp** - Hidden start support
- **multi_app_utils.cpp** - Multi-application utilities
- **extension_permissions_util.cpp** - Extension permission handling
- **keep_alive_utils.cpp** - Keep-alive support utilities
- **ability_event_util.cpp** - Ability event helpers
- **dms_util.cpp** - DMS (Distributed Manager) integration
- **udmf_utils.cpp** - UDMF (Unified Data Management Framework) integration
- **request_id_util.cpp** - Request ID generation

## IPC Communication

### Key IPC Interfaces

- **AbilityManagerStub** - Receives client requests via IPC
- **AbilitySchedulerProxy/Stub** - Communicates with application processes
- **ConnectionRecord** - Tracks service connections
- **AppScheduler** - Coordinates with AppManagerService

### IPC Patterns

1. **Client → AMS**: `AbilityManagerProxy` → `AbilityManagerStub` → `AbilityManagerService`
2. **AMS → App**: `AbilitySchedulerProxy` → `AbilitySchedulerStub` → Application

## Debugging

### Using aa Command

```bash
# View Ability stack
aa dump -a

# View process info
aa dump -i

# Dump specific mission
hidumper -s AbilityManagerService -a -a

# Enable detailed logs
hilog -b D | grep -i ams
```

### Common Issues

- **Ability won't start**: Check bundle configuration, permissions, and interceptor chain
- **IPC timeout**: Verify service/client build version compatibility
- **Lifecycle state mismatch**: Ensure state transitions follow valid paths
- **Mission not persisting**: Check `mission_save_time` config and RDB initialization

## Related Services

- **AppManagerService** - Process management coordination
- **BundleManagerService** - Package and component discovery
- **WindowManagerService** - Window lifecycle coordination
- **DataShareManager** - Data sharing operations
- **ScreenLockManager** - Screen state (when `ABILITY_RUNTIME_SCREENLOCK_ENABLE` defined)

## Testing

Tests are located in the parent `test/` directory of ability_runtime:

```bash
# Run unit tests
./build.sh --product-name <product> --test-component ability_runtime --test-case unittest

# Run specific abilitymgr tests
./build.sh --product-name <product> --test-component ability_runtime --test-case ability_manager_service_first_test
./build.sh --product-name <product> --test-component ability_runtime --test-case ability_manager_service_third_test
./build.sh --product-name <product> --test-component ability_runtime --test-case ability_manager_service_sixth_test

# Enable test commands (requires rebuild with flag)
./build.sh --product-name <product> --build-target ability_runtime --define ability_command_for_test=true
```

## Feature Flags

The service behavior is controlled by compile-time defines and runtime configuration:

- `SUPPORT_GRAPHICS` - Window/UI functionality
- `SUPPORT_CHILD_PROCESS` - Child process support
- `SUPPORT_AUTO_FILL` - Auto-fill integration
- `ABILITY_RUNTIME_SCREENLOCK_ENABLE` - Screen lock hooks
- `BGTASKMGR_CONTINUOUS_TASK_ENABLE` - Background task support
- `RESOURCE_SCHEDULE_SERVICE_ENABLE` - Resource scheduling
- `WITH_DLP` - DLP (Data Loss Prevention) support
