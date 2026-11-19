#ifndef OHOS_ABILITY_RUNTIME_DEEPLINK_RESERVE_CONFIG_H
#define OHOS_ABILITY_RUNTIME_DEEPLINK_RESERVE_CONFIG_H

#include <string>

namespace OHOS {
namespace AAFwk {
class DeepLinkReserveConfig {
public:
    static DeepLinkReserveConfig &GetInstance()
    {
        static DeepLinkReserveConfig instance;
        return instance;
    }
    bool IsLinkReserved(const std::string &uri, std::string &bundleName) { return false; }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DEEPLINK_RESERVE_CONFIG_H
