#pragma once

// TODO move this to engine
#include "custom_fmt.h"
//

#include "../vendor/supermarket-engine/engine/entity.h"
//
#include "../vendor/supermarket-engine/engine/app.h"
#include "../vendor/supermarket-engine/engine/camera.h"
#include "../vendor/supermarket-engine/engine/globals.h"
#include "../vendor/supermarket-engine/engine/layer.h"
#include "../vendor/supermarket-engine/engine/pch.hpp"
#include "../vendor/supermarket-engine/engine/renderer.h"

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
    float speed = 1.f;

    Movable(const glm::vec2& position_ = glm::vec2{},
            const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
            const glm::vec4& color_ = glm::vec4{1.f},
            const std::string& textureName_ = "white")
        : Collidable(position_, size_, angle_, color_, textureName_) {}
};

struct Player : public Movable {
    Player(const glm::vec2& position_ = glm::vec2{},
           const glm::vec2& size_ = glm::vec2{1.f}, float angle_ = 0.f,
           const glm::vec4& color_ = glm::vec4{1.f},
           const std::string& textureName_ = "white")
        : Movable(position_, size_, angle_, color_, textureName_) {
        speed = 5.f;
    }

    virtual void onUpdate(Time dt) {
        if (Input::isKeyPressed(Key::getMapping("Left"))) {
            position.x -= speed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Right"))) {
            position.x += speed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Down"))) {
            position.y -= speed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Up"))) {
            position.y += speed * dt;
        }

        auto controller =
            GLOBALS.get_ptr<OrthoCameraController>("diverCameraController");
        controller->camera.position.x = position.x;
        controller->camera.position.y = position.y;
        controller->camera.updateViewMat();
    }
    virtual const char* typeString() const { return "Player"; }
};

struct Enemy : public Movable {
    glm::vec2 velocity;
    glm::vec2 acceleration;
    float maxacc = 0.03f;

    Enemy(const glm::vec2& position_, const glm::vec2& size_, float angle_,
          const glm::vec4& color_, const std::string& textureName_)
        : Movable(position_, size_, angle_, color_, textureName_) {
        speed = 0.05f;
    }

    glm::vec2 toPlayer(std::vector<std::shared_ptr<Enemy>>) {
        auto player = GLOBALS.get<Player>("player");
        return (player.position - position) / 100.f;
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
    }
    virtual const char* typeString() const { return "Enemy"; }
};

struct DiverLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;
    std::shared_ptr<Player> player;

    DiverLayer() : Layer("Diver") {
        isMinimized = true;

        auto appSettings = App::getSettings();

        cameraController.reset(new OrthoCameraController(appSettings.ratio));
        GLOBALS.set<OrthoCameraController>("diverCameraController",
                                           cameraController.get());
        cameraController->setZoomLevel(5.f);

        cameraController->camera.setViewport(
            glm::vec4{0, 0, appSettings.width, appSettings.height});
        cameraController->rotationEnabled = false;
        cameraController->zoomEnabled = false;
        cameraController->movementEnabled = false;

        // 918 × 203 pixels at 16 x 16 with margin 1
        float playerSprite = 16.f;
        Renderer::addTexture("./resources/character_tilesheet.png");
        Renderer::addSubtexture("character_tilesheet", "player", 0, 0,
                                playerSprite, playerSprite);
        Renderer::addSubtexture("character_tilesheet", "player2", 0, 1,
                                playerSprite, playerSprite);
        Renderer::addSubtexture("character_tilesheet", "player3", 1, 1,
                                playerSprite, playerSprite);

        const int num_people_sprites = 3;
        std::array<std::string, num_people_sprites> peopleSprites = {
            "player",
            "player2",
            "player3",
        };

        player = std::make_shared<Player>();
        player->size = {0.6f, 0.6f};
        player->color = glm::vec4{gen_rand_vec3(0.3f, 1.0f), 1.f};
        player->textureName = peopleSprites[0];
        EntityHelper::addEntity(player);
        GLOBALS.set<Player>("player", player.get());

        for (int i = 0; i < 100; i++) {
            EntityHelper::addEntity(std::make_shared<Enemy>(Enemy(
                glm::vec2{randIn(1, 10) * cos(i), randIn(1, 10) * sin(i)},
                glm::vec2{0.6f, 0.6f}, 0,
                glm::vec4{gen_rand_vec3(0.3f, 1.0f), 1.f}, peopleSprites[1])));
        }

        for (int i = 0; i < 100; i++) {
            EntityHelper::addEntity(
                std::make_shared<StaticObject>(StaticObject(  //
                    glm::vec2{-10, i},                        //
                    glm::vec2{0.6f, 0.6f},                    //
                    0,                                        //
                    gen_rand_vec4(0.3f, 1.0f),                //
                    peopleSprites[1]                          //
                    )));

            EntityHelper::addEntity(
                std::make_shared<StaticObject>(StaticObject(  //
                    glm::vec2{10, i},                         //
                    glm::vec2{0.6f, 0.6f},                    //
                    0,                                        //
                    gen_rand_vec4(0.3f, 1.0f),                //
                    peopleSprites[1]                          //
                    )));
        }
    }

    virtual ~DiverLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    void child_updates(Time dt) {
        if (GLOBALS.get<bool>("terminal_closed")) {
            cameraController->onUpdate(dt);
        }

        EntityHelper::forEachEntity([&](auto entity) {  //
            entity->onUpdate(dt);
        });
    }

    void render() {
        Renderer::begin(cameraController->camera);
        // should go underneath entities also

        EntityHelper::forEachEntity([&](auto entity) {  //
            entity->render();
        });

        Renderer::end();
    }

    glm::vec3 getMouseInWorld() {
        auto mouse = Input::getMousePosition();
        auto viewport = glm::vec4{0, 0, App::getSettings().width,
                                  App::getSettings().height};
        return screenToWorld(
            glm::vec3{mouse.x, App::getSettings().height - mouse.y, 0.f},
            cameraController->camera.view, cameraController->camera.projection,
            viewport);
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent&) {
        // glm::vec3 mouseInWorld = getMouseInWorld();
        return false;
    }
    bool onMouseMoved(Mouse::MouseMovedEvent&) {
        // glm::vec3 mouseInWorld = getMouseInWorld();
        return false;
    }

    bool onMouseButtonReleased(Mouse::MouseButtonReleasedEvent&) {
        // glm::vec3 mouseInWorld = getMouseInWorld();
        return false;
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::getMapping("Esc")) {
            // Menu::get().state = Menu::State::Root;
            return true;
        }
        return false;
    }

    virtual void onUpdate(Time dt) override {
        // if (Menu::get().state != Menu::State::Game) return;

        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());
        prof give_me_a_name(__PROFILE_FUNC__);

        child_updates(dt);        // move things around
        render();                 // draw everything
        EntityHelper::cleanup();  // Cleanup dead entities
    }

    virtual void onEvent(Event& event) override {
        // if (Menu::get().state != Menu::State::Game) return;
        // cameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &DiverLayer::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<Mouse::MouseButtonReleasedEvent>(std::bind(
            &DiverLayer::onMouseButtonReleased, this, std::placeholders::_1));
        dispatcher.dispatch<Mouse::MouseMovedEvent>(
            std::bind(&DiverLayer::onMouseMoved, this, std::placeholders::_1));
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&DiverLayer::onKeyPressed, this, std::placeholders::_1));
    }
};

