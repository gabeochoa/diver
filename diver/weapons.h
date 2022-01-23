
#pragma once

#include "../vendor/supermarket-engine/engine/entity.h"
#include "../vendor/supermarket-engine/engine/time.h"
#include "particle_system.h"

struct Player;

struct Weapon {
    Player* owner;
    float dmg;       // how much this does, base enemies have 100 health
    float cooldown;  // how much time between firing
    float timeleft;
    float range;            // ? nothing right now TODO
    float projectileSpeed;  // how fast it goes + owner speed
    glm::vec2 projectileSize;
    int projectileIndex = 0;

    Weapon(Player* o) : owner(o) {}
    virtual ~Weapon() {}

    void handleCooldown(Time dt) {
        timeleft -= dt.s();
        if (timeleft <= 0) {
            timeleft = cooldown;
            fire();
            projectileIndex++;
        }
    }

    void onUpdate(Time dt) { handleCooldown(dt); }
    void fire();
    virtual float getAngle(Player* player) = 0;
    virtual void render() {}
};

struct Projectile : Entity {
    Weapon* owner;
    glm::vec2 velocity;
    float angularVelocity;
    float traveled = 0.f;
    float range;

    Projectile(
        // projectile
        Weapon* o,                                 //
        const glm::vec2& velocity_ = glm::vec2{},  //
        const float angularVelocity_ = 0.f,        //
        float range_ = 200.f,
        //
        //
        // entity stuff
        const glm::vec2& position_ = glm::vec2{},
        const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
        const glm::vec4& color_ = glm::vec4{1.f},
        const std::string& textureName_ = "white")
        : Entity(position_, size_, angle_, color_, textureName_) {
        owner = o;
        velocity = velocity_;
        angularVelocity = angularVelocity_;
        range = range_;
    }

    void onUpdate(Time dt) {
        angle += angularVelocity * dt.s();
        auto movement = glm::vec2{velocity.x * sin(glm::radians(angle)),
                                  velocity.y * cos(glm::radians(angle))};
        position += movement * dt.s();
        traveled += glm::length(movement) * dt.s();
    }

    virtual const char* typeString() const { return "Projectile"; }
};

struct Dart : public Weapon {
    Dart(Player* o) : Weapon(o) {
        dmg = 10.f;
        cooldown = 0.5f;
        timeleft = cooldown;
        range = 200.f;
        projectileSpeed = 3.f;
        projectileSize = glm::vec2{0.05f, 0.2f};
    }
    float getAngle(Player* player) override;
};

struct Spear : public Weapon {
    Spear(Player* o) : Weapon(o) {
        dmg = 20.f;
        cooldown = 1.f;
        timeleft = cooldown;
        range = 2.f;
        projectileSpeed = 1.5f;
        projectileSize = glm::vec2{0.2f, 1.f};
    }
    float getAngle(Player* player) override;
};

struct Bubbles : public Weapon {
    Bubbles(Player* o) : Weapon(o) {
        dmg = 1.f;
        cooldown = 0.1f;
        timeleft = cooldown;
        range = 200.f;
        projectileSpeed = 0.1f;
        projectileSize = glm::vec2{0.2f, 0.2f};
    }
    float getAngle(Player* player) override;
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
