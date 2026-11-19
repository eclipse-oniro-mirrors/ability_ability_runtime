#ifndef OHOS_ABILITY_RUNTIME_MOCK_MULTI_APP_UTILS_STATUS_H
#define OHOS_ABILITY_RUNTIME_MOCK_MULTI_APP_UTILS_STATUS_H

#include <map>
#include <string>

namespace OHOS {
namespace AAFwk {
struct MockMultiAppUtilsStatus {
    static void Reset();

    // bundleName -> preferredAppIndex. When a bundleName is present, the mock
    // GetPreferredAppCloneIndex returns true and sets appIndex to the mapped value.
    // Empty map (default) means GetPreferredAppCloneIndex always returns false, which
    // preserves the original mock behavior.
    static std::map<std::string, int32_t> preferredIndexMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_MULTI_APP_UTILS_STATUS_H
