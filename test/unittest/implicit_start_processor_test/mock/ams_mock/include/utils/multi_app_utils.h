#ifndef OHOS_ABILITY_RUNTIME_MULTI_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_MULTI_APP_UTILS_H

#include <string>

namespace OHOS {
namespace AAFwk {
class MultiAppUtils {
public:
    static void GetRunningMultiAppIndex(const std::string &bundleName, int32_t uid, int32_t &appIndex);
    static bool GetPreferredAppCloneIndex(const std::string &bundleName, int32_t userId, int32_t &appIndex);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MULTI_APP_UTILS_H
