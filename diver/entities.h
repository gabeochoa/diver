
#pragma once

// TODO move this to engine
#include "custom_fmt.h"
//

#include "../vendor/supermarket-engine/engine/app.h"
#include "../vendor/supermarket-engine/engine/entity.h"
//

#include "weapons.h"

struct Experience : public Entity {
    int amount = 1;

    Experience(const glm::vec2& position_ = glm::vec2{},
               const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
               const glm::vec4& color_ = glm::vec4{1.f},
               const std::string& textureName_ = "white")
        : Entity(position_, size_, angle_, color_, textureName_) {}
    virtual const char* typeString() const { return "Experience"; }
};

struct Collidable : public Entity {
    Collidable(const glm::vec2& position_ = glm::vec2{},
               const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
               const glm::vec4& color_ = glm::vec4{1.f},
               const std::string& textureName_ = "white")
        : Entity(position_, size_, angle_, color_, textureName_) {}
};

struct StaticObject : public Collidable {
    StaticObject(const glm::vec2& position_ = glm::vec2{},
                 const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
                 const glm::vec4& color_ = glm::vec4{1.f},
                 const std::string& textureName_ = "white")
        : Collidable(position_, size_, angle_, color_, textureName_) {}

    virtual const char* typeString() const { return "StaticObject"; }
};

struct Movable : public Collidable {
    float maxhealth = 100.f;
    float health;

    float speed = 1.f;

    Movable(const glm::vec2& position_ = glm::vec2{},
            const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
            const glm::vec4& color_ = glm::vec4{1.f},
            const std::string& textureName_ = "white")
        : Collidable(position_, size_, angle_, color_, textureName_) {
        health = maxhealth;
    }

    void renderHealthBar() {
        float healthpct = health / maxhealth;
        Renderer::drawQuad(glm::vec2{position.x, position.y - 0.1f},  //
                           glm::vec2{healthpct * size.x, 0.1f},       //
                           glm::vec4{0.8, 0.1, 0.f, 1.f}, "white");
    }

    virtual void render(const RenderOptions& ro = RenderOptions()) {
        Entity::render(ro);
        renderHealthBar();
    }
};

struct Player : public Movable {
    struct Upgrade {
        std::string title;
        std::string description;
        std::function<void(void)> apply;
    };

    std::vector<std::shared_ptr<Weapon>> weapons;
    int facing = 1;

    int level = 1;
    float expNeededForNextLevel;
    float experience;

    Player(const glm::vec2& position_ = glm::vec2{},
           const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
           const glm::vec4& color_ = glm::vec4{1.f},
           const std::string& textureName_ = "white")
        : Movable(position_, size_, angle_, color_, textureName_) {
        speed = 5.f;
        experience = 0.f;

        weapons.push_back(std::make_shared<Bubbles>(Bubbles(this)));
        // weapons.push_back(std::make_shared<Dart>(Dart(this)));
        // weapons.push_back(std::make_shared<Spear>(Spear(this)));
    }

    std::array<Upgrade, 3> getUpgradeOptions() {
        return std::array<Upgrade, 3>{
            Upgrade{
                .title = "Option 1",
                .description = "description for option 1",
                .apply = []() { log_info("pressed option 1"); },
            },
            Upgrade{
                .title = "Option 2",
                .description = "description for option 2",
                .apply = []() { log_info("pressed option 2"); },
            },
            Upgrade{
                .title = "Option 3",
                .description = "description for option 3",
                .apply = []() { log_info("pressed option 3"); },
            },
        };
    }

    virtual void onUpdate(Time dt) {
        if (Input::isKeyPressed(Key::getMapping("Left"))) {
            position.x -= speed * dt;
            facing = 0;
        }
        if (Input::isKeyPressed(Key::getMapping("Right"))) {
            position.x += speed * dt;
            facing = 1;
        }
        if (Input::isKeyPressed(Key::getMapping("Down"))) {
            position.y -= speed * dt;
            facing = 2;
        }
        if (Input::isKeyPressed(Key::getMapping("Up"))) {
            position.y += speed * dt;
            facing = 3;
        }

        position = glm::vec2{
            fmax(-10, fmin(10, position.x)),
            fmax(-100, fmin(100, position.y)),
        };

        auto controller =
            GLOBALS.get_ptr<OrthoCameraController>("diverCameraController");
        controller->camera.position.x = position.x;
        controller->camera.position.y = position.y;
        controller->camera.updateViewMat();

        for (auto w : weapons) {
            w->onUpdate(dt);
        }
        expNeededForNextLevel = 1 * std::pow(level, 2.5f);
    }

    virtual const char* typeString() const { return "Player"; }

    virtual void render(const RenderOptions& ro = RenderOptions()) {
        Movable::render(ro);

        for (auto w : weapons) {
            w->render();
        }
    }
};

struct Enemy : public Movable {
    glm::vec2 velocity;
    glm::vec2 acceleration;
    float maxacc = 0.03f;
    float dmg = 1.f;
    float expAmount;

    Enemy(const glm::vec2& position_, const glm::vec2& size_, float angle_,
          const glm::vec4& color_, const std::string& textureName_)
        : Movable(position_, size_, angle_, color_, textureName_) {
        speed = 0.05f;
        expAmount = 1.f;
    }

    glm::vec2 toPlayer(std::vector<std::shared_ptr<Enemy>>) {
        auto player = GLOBALS.get<Player>("player");
        return (player.position - position) / 50.f;
    }

    glm::vec2 separate(std::vector<std::shared_ptr<Enemy>> enemies) {
        float desiredSep = glm::length(size) * 2.f;
        glm::vec2 steer;
        int count = 0;
        for (auto e : enemies) {
            float d = glm::distance(position, e->position);
            if (d > 0 && d < desiredSep) {
                steer += (glm::normalize(position - e->position) / d);
                count++;
            }
        }
        if (count) steer /= count;
        if (glm::length(steer) > 0) {
            steer = limit((glm::normalize(steer) * speed) - velocity, maxacc);
        }
        return steer;
    }

    glm::vec2 alignWith(std::vector<std::shared_ptr<Enemy>> enemies) {
        float viewDist = glm::length(size) * 10.f;
        glm::vec2 sum;
        glm::vec2 steer;
        int count = 0;
        for (auto e : enemies) {
            float d = glm::distance(position, e->position);
            if (d > 0 && d < viewDist) {
                sum += e->velocity;
                count++;
            }
        }
        if (count) {
            sum /= count;
            steer = limit((glm::normalize(sum) * speed) - velocity, maxacc);
        }
        return steer;
    }

    glm::vec2 seek(glm::vec2 target) {
        auto desired = target - position;
        return limit((glm::normalize(desired) * speed) - velocity, maxacc);
    }

    glm::vec2 cohesion(std::vector<std::shared_ptr<Enemy>> enemies) {
        float viewDist = glm::length(size) * 10.f;
        glm::vec2 sum;
        int count = 0;
        for (auto e : enemies) {
            float d = glm::distance(position, e->position);
            if (d > 0 && d < viewDist) {
                sum += e->velocity;
                count++;
            }
        }
        if (count) {
            sum /= count;
            return seek(sum);
        }
        return glm::vec2{0.f};
    }

    void rules(std::vector<std::shared_ptr<Enemy>> enemies) {
        auto player = toPlayer(enemies);
        auto sep = separate(enemies);
        auto align = alignWith(enemies);
        auto unity = cohesion(enemies);

        player *= 2.f;
        sep *= 4.f;
        align *= 1.f;
        unity *= 1.f;

        acceleration += player;
        acceleration += sep;
        acceleration += align;
        acceleration += unity;
    }

    inline glm::vec2 limit(glm::vec2 v, float mx) {
        if (glm::length(v) > mx) {
            v = glm::normalize(v) * mx;
        }
        return v;
    }

    virtual void onUpdate(Time dt) {
        Movable::onUpdate(dt);
        rules(EntityHelper::getEntitiesInRange<Enemy>(position, 10.f));
        velocity += acceleration;
        velocity = limit(velocity, speed);
        position += velocity;
        acceleration *= 0;

        if (health <= 0) {
            cleanup = true;
            EntityHelper::addEntity(std::make_shared<Experience>(
                Experience(position, glm::vec2{0.1f}, 0.f,
                           glm::vec4{0.f, 0.9, 0.3, 1.f}, "white")));
        }
    }

    virtual const char* typeString() const { return "Enemy"; }
};

