#include "ConfigUpdaters.h"
#include "ConfigMigrators.h"
#include <libultraship/log/luslog.h>

namespace SOH {
ConfigVersion1Updater::ConfigVersion1Updater() : ConfigVersionUpdater(1) {
}

void ConfigVersion1Updater::Update(Ship::Config* conf) {

    for (Migration migration : version1Migrations) {
        if (migration.action == MigrationAction::Rename) {
            // LUSLOG_INFO("ConfigVersionUpdaterV1: From: %s, To: %s", migration.from.c_str(),
            // migration.to.value().c_str());
            CVarCopy(migration.from.c_str(), migration.to.value().c_str());
        }
        CVarClear(migration.from.c_str());
    }
}

} // namespace SOH