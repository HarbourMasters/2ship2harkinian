#include "libultraship/libultraship.h"

namespace SOH {
class ConfigVersion1Updater final : public Ship::ConfigVersionUpdater {
  public:
    ConfigVersion1Updater();
    void Update(Ship::Config* conf);
};

}; // namespace SOH
