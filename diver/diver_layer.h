#pragma once

#include "../vendor/supermarket-engine/engine/app.h"
#include "../vendor/supermarket-engine/engine/camera.h"
#include "../vendor/supermarket-engine/engine/entity.h"
#include "../vendor/supermarket-engine/engine/globals.h"
#include "../vendor/supermarket-engine/engine/layer.h"
#include "../vendor/supermarket-engine/engine/pch.hpp"
#include "../vendor/supermarket-engine/engine/renderer.h"

struct Player : public Entity {
    //
    float speed = 1.f;

    Player() : Entity() {}

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
    }
    virtual const char* typeString() const { return "Player"; }
};

struct DiverLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;

    DiverLayer() : Layer("Diver") {
        isMinimized = true;

        auto appSettings = App::getSettings();

        cameraController.reset(new OrthoCameraController(appSettings.ratio));
        GLOBALS.set<OrthoCameraController>("diverCameraController",
                                           cameraController.get());

        cameraController->camera.setViewport(
            glm::vec4{0, 0, appSettings.width, appSettings.height});
        cameraController->rotationEnabled = false;

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

        auto player = Player();
        player.color = gen_rand_vec4(0.3f, 1.0f);
        player.color.w = 1.f;
        player.size = {0.6f, 0.6f};
        player.textureName = peopleSprites[0];
        EntityHelper::addEntity(std::make_shared<Player>(player));
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
        cameraController->onEvent(event);
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

