
#include "weapons.h"

#include <memory>

#include "../vendor/supermarket-engine/engine/entity.h"
#include "entities.h"

void Weapon::fire() {
    auto getAngle = [](Player* player) -> float {
        if (!player) return 0;

        switch (player->facing) {
            case 0:
                return 270;
            case 1:
                return 90;
            case 2:
                return -180;
            case 3:
            default:
                return 0;
        }
    };

    EntityHelper::addEntity(                                //
        std::make_shared<Projectile>(                       //
            Projectile(                                     //
                this,                                       // owner
                glm::vec2{owner->speed + projectileSpeed},  // vel
                0.f,                                        // angularVel
                range,                                      // range
                // entity stuff
                owner->position,  // position
                projectileSize,   // size
                getAngle(owner),  // angle
                glm::vec4{1.f},   // color
                "white"           //
                )                 //
            ));
}
