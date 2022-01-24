
#pragma once

#include "../vendor/supermarket-engine/engine/entity.h"
#include "../vendor/supermarket-engine/engine/time.h"
#include "particle_system.h"

struct Player;

constexpr size_t NUM_WEAPONS = 3;

struct Weapon {
    Player* owner;
    float dmg;       // how much this does, base enemies have 100 health
    float cooldown;  // how much time between firing
    float timeleft;
    float range;            // ? nothing right now TODO
    float projectileSpeed;  // how fast it goes + owner speed
    glm::vec2 projectileSize;
    int projectileIndex = 0;
    std::vector<std::string> textures;
    int level = 0;

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
    virtual float getTextureAngle(Player*) { return 0; }
    virtual void render() {}

    virtual std::string getTextureName() {
        return textures[projectileIndex % textures.size()];
    }

    virtual const char* getName() = 0;
    virtual const char* getDescription() = 0;
    virtual std::function<void(void)> getUpgrade() = 0;
};

struct Projectile : Entity {
    Weapon* owner;
    glm::vec2 velocity;
    float velocityAngle;
    float traveled = 0.f;
    float range;

    Projectile(
        // projectile
        Weapon* o,                                 //
        const glm::vec2& velocity_ = glm::vec2{},  //
        const float velocityAngle_ = 0.f,          //
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
        range = range_;
        velocityAngle = velocityAngle_;
    }

    void onUpdate(Time dt) {
        auto movement =
            glm::vec2{velocity.x * sin(glm::radians(velocityAngle)),
                      velocity.y * cos(glm::radians(velocityAngle))};
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
        range = 8.f;
        projectileSpeed = 3.f;
        projectileSize = glm::vec2{0.2f, 0.4f};

        Renderer::addSubtexture("tilesheet", "dart0", 3, 5, 32.f, 32.f);
        textures.push_back("dart0");
    }
    float getAngle(Player* player) override;
    float getTextureAngle(Player*) override;
    virtual const char* getName() override { return "Dart"; }
    virtual const char* getDescription() override {
        if (level == 0) {
            return "Basic Dart. 10 damage, shoots twice a second";
        }
        log_warn("Need to implement dart level{}", level);
        return "Dart Level not implemented";
    }
    virtual std::function<void(void)> getUpgrade() override {
        // TODO implment level upgrades
        return std::function<void(void)>();
    }
};

struct Spear : public Weapon {
    Spear(Player* o) : Weapon(o) {
        dmg = 20.f;
        cooldown = 1.f;
        timeleft = cooldown;
        range = 2.f;
        projectileSpeed = 1.5f;
        projectileSize = glm::vec2{0.2f, 1.f};

        Renderer::addSubtexture("tilesheet", "spear0", 4, 5, 64.f, 32.f);
        textures.push_back("spear0");
    }
    float getAngle(Player* player) override;
    virtual const char* getName() override { return "Spear"; }
    virtual const char* getDescription() override {
        if (level == 0) {
            return "Basic Spear. 20 damage, shoots once a second";
        }
        log_warn("Need to implement spear level{}", level);
        return "spear Level not implemented";
    }
    virtual std::function<void(void)> getUpgrade() override {
        // TODO implment level upgrades
        return std::function<void(void)>();
    }
};

struct Bubbles : public Weapon {
    Bubbles(Player* o) : Weapon(o) {
        // TODO replace when ready to test
        dmg = 100.f;
        cooldown = 0.1f;
        timeleft = cooldown;
        range = 8.f;
        projectileSpeed = 0.05f;
        projectileSize = glm::vec2{0.2f, 0.2f};

        Renderer::addSubtexture("tilesheet", "bubbles0", 0, 5, 32.f, 32.f);
        Renderer::addSubtexture("tilesheet", "bubbles1", 1, 5, 32.f, 32.f);
        textures.push_back("bubbles0");
        textures.push_back("bubbles1");
    }
    float getAngle(Player* player) override;
    virtual const char* getName() override { return "Bubbles"; }
    virtual const char* getDescription() override {
        if (level == 0) {
            return "Basic Bubbles. 1 damage, shoots ten times a second";
        }
        log_warn("Need to implement bubbles level{}", level);
        return "bubbles Level not implemented";
    }
    virtual std::function<void(void)> getUpgrade() override {
        // TODO implment level upgrades
        return [this]() {
            range *= 2;
            cooldown /= 2.f;
        };
    }
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
