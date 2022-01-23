
#pragma once

#include "../vendor/supermarket-engine/engine/entity.h"
#include "../vendor/supermarket-engine/engine/time.h"
#include "particle_system.h"

struct Player;

struct Weapon {
    Player* owner;
    float dmg;
    float cooldown;
    float timeleft;
    float range;

    Weapon(Player* o) : owner(o) {}
    virtual ~Weapon() {}

    void handleCooldown(Time dt) {
        timeleft -= dt.s();
        if (timeleft <= 0) {
            timeleft = cooldown;
            fire();
        }
    }

    void onUpdate(Time dt) { handleCooldown(dt); }

    virtual void render() {}
    virtual void fire() = 0;
};

struct Projectile : Entity {
    Weapon* owner;
    glm::vec2 velocity;
    float angularVelocity;

    Projectile(
        // projectile
        Weapon* o, const glm::vec2& velocity_ = glm::vec2{},
        const float angularVelocity_ = 0.f,

        // entity stuff
        const glm::vec2& position_ = glm::vec2{},
        const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
        const glm::vec4& color_ = glm::vec4{1.f},
        const std::string& textureName_ = "white")
        : Entity(position_, size_, angle_, color_, textureName_) {
        owner = o;
        velocity = velocity_;
        angularVelocity = angularVelocity_;
    }

    void onUpdate(Time dt) {
        angle += angularVelocity * dt.s();
        auto movement = glm::vec2{velocity.x * sin(glm::radians(angle)),
                                  velocity.y * cos(glm::radians(angle))};
        position += movement * dt.s();
    }

    virtual const char* typeString() const { return "Projectile"; }
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
    Dart(Player* o) : Weapon(o) {
        dmg = 10.f;
        cooldown = 0.5f;
        timeleft = cooldown;
        range = 10.f;
    }

    virtual void fire() override;
};
