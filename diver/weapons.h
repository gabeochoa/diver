
#pragma once

#include "../vendor/supermarket-engine/engine/entity.h"
#include "../vendor/supermarket-engine/engine/time.h"
#include "particle_system.h"

struct Weapon {
    Entity* owner;
    ParticleSystem ps;
    float dmg;
    float cooldown;
    float timeleft;
    float range;

    Weapon(Entity* o) : owner(o) {}
    virtual ~Weapon() {}

    void handleCooldown(Time dt) {
        timeleft -= dt.s();
        if (timeleft <= 0) {
            timeleft = cooldown;
            fire();
        }
    }

    void onUpdate(Time dt) {
        handleCooldown(dt);
        ps.onUpdate(dt);
    }

    void fire() { ps.emit(createParticle()); }
    void render() { ps.render(); }

    virtual ParticleSystem::Particle createParticle() = 0;
};

// TODO support homing
// struct Enemy;
// std::vector<std::shared_ptr<Entity>> enemies;
// float r = 0.f;
// while (enemies.empty()) {
// r += 0.1f;
// enemies =
// EntityHelper::getEntitiesInRange<Enemy>(owner->position, r);
// if (r >= range) {
// break;
// }
// }
//
// if (enemies.empty()) {
// return;
// }

struct Dart : public Weapon {
    Dart(Entity* o) : Weapon(o) {
        dmg = 0.1f;
        cooldown = 0.5f;
        timeleft = cooldown;
        range = 10.f;
    }

    virtual ParticleSystem::Particle createParticle() override;
};
