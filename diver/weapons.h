
#pragma once

#include "../vendor/supermarket-engine/engine/time.h"
#include "particle_system.h"

struct Weapon {
    ParticleSystem ps;
    float dmg;
    float cooldown;
    float timeleft;

    Weapon() {}
    virtual ~Weapon() {}

    virtual ParticleSystem::Particle createParticle() = 0;

    void fire() { ps.emit(createParticle()); }

    void onUpdate(Time dt) {
        timeleft -= dt.s();
        if (timeleft <= 0) {
            timeleft = cooldown;
            fire();
        }
    }
};

struct Dart : public Weapon {
    Dart() {
        dmg = 0.1f;
        cooldown = 0.5f;
        timeleft = cooldown;
    }

    virtual ParticleSystem::Particle createParticle() override {
        auto p = ParticleSystem::Particle();

        return p;
    }
};
