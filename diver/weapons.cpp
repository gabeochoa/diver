
#include "weapons.h"

#include <memory>

#include "entities.h"

ParticleSystem::Particle Dart::createParticle() {
    auto p = ParticleSystem::Particle();
    p.position = owner->position;
    p.velocity = glm::vec2{0.1f, 0.1f};

    auto getAngle = [](Entity* owner) -> float {
        Player* player = dynamic_cast<Player*>(owner);
        if (!player) return 0;

        switch (player->facing) {
            case 1:
                return 90;
            case 2:
                return 180;
            case 3:
                return 270;
            case 0:
            default:
                return 0;
        }
    };

    p.angle = getAngle(owner);

    p.size = glm::vec2{0.1f};
    p.lifetime = 1.f;
    return p;
}
