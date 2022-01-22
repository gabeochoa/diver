
#include "weapons.h"

#include <memory>

#include "../vendor/supermarket-engine/engine/entity.h"
#include "entities.h"

void Dart::fire() {
    auto getAngle = [](Entity* owner) -> float {
        Player* player = dynamic_cast<Player*>(owner);
        if (!player) return 0;

        switch (player->facing) {
            case 1:
                return 90;
            case 2:
                return 180;
            case 3:
                return 0;
            case 0:
            default:
                return -90;
        }
    };

    EntityHelper::addEntity(           //
        std::make_shared<Projectile>(  //
            Projectile(                //
                this,                  // owner
                glm::vec2{0.f, 1.f},   // vel
                0.f,                   // angularVel
                // entity stuff
                owner->position,       // position
                glm::vec2{0.1, 0.2f},  // size
                getAngle(owner),       // angle
                glm::vec4{1.f},        // color
                "white"                //
                )                      //
            ));
}
