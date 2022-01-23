
#include "weapons.h"

#include <memory>

#include "../vendor/supermarket-engine/engine/entity.h"
#include "entities.h"

void Weapon::fire() {
    EntityHelper::addEntity(                                //
        std::make_shared<Projectile>(                       //
            Projectile(                                     //
                this,                                       // owner
                glm::vec2{owner->speed + projectileSpeed},  // vel
                getAngle(owner),                            // angularVel
                range,                                      // range
                // entity stuff
                owner->position,         // position
                projectileSize,          // size
                getTextureAngle(owner),  // texture angle
                glm::vec4{1.f},          // color
                getTextureName()         // texturename
                )                        //
            ));
}

float Dart::getAngle(Player* player) {
    if (!player) return 0;
    if (player->facing == 0) return 270;  // left
    if (player->facing == 1) return 90;   // right
    if (player->facing == 2) return 180;  // down
    if (player->facing == 3) return 0;    // up
    return 0;
}

float Dart::getTextureAngle(Player* player) {
    if (!player) return 0;
    if (player->facing == 1) return 180;  // right
    if (player->facing == 2) return 90;   // down
    if (player->facing == 3) return 270;  // up
    return 0;
}

float Spear::getAngle(Player*) { return projectileIndex % 2 == 0 ? 90 : 270; }
float Bubbles::getAngle(Player*) { return (projectileIndex * 36) % 360; }

