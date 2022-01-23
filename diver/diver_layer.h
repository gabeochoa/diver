#pragma once

#include "../vendor/supermarket-engine/engine/app.h"
#include "../vendor/supermarket-engine/engine/camera.h"
#include "../vendor/supermarket-engine/engine/globals.h"
#include "../vendor/supermarket-engine/engine/layer.h"
#include "../vendor/supermarket-engine/engine/pch.hpp"
#include "../vendor/supermarket-engine/engine/renderer.h"

struct DiverLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;
    std::shared_ptr<Player> player;

    std::array<std::string, 3> fishSprites = {
        "fish0",
        "fish1",
        "fish2",
    };
    int MAX_ENEMIES = 100;

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

        float playerSprite = 32.f;
        Renderer::addTexture("./resources/tilesheet.png");
        Renderer::addSubtexture("tilesheet", "player", 0, 0, playerSprite,
                                playerSprite);
        Renderer::addSubtexture("tilesheet", "fish0", 0, 1, playerSprite,
                                playerSprite);
        Renderer::addSubtexture("tilesheet", "fish1", 1, 1, playerSprite,
                                playerSprite);
        Renderer::addSubtexture("tilesheet", "fish2", 2, 1, playerSprite,
                                playerSprite);

        player = std::make_shared<Player>();
        player->size = {0.6f, 0.6f};
        player->color = glm::vec4{gen_rand_vec3(0.3f, 1.0f), 1.f};
        player->textureName = "player";
        EntityHelper::addEntity(player);
        GLOBALS.set<Player>("player", player.get());

        for (int i = -50; i < 50; i++) {
            EntityHelper::addEntity(
                std::make_shared<StaticObject>(StaticObject(  //
                    glm::vec2{-10, i},                        //
                    glm::vec2{0.6f, 0.6f},                    //
                    0,                                        //
                    gen_rand_vec4(0.3f, 1.0f),                //
                    fishSprites[1]                            //
                    )));

            EntityHelper::addEntity(
                std::make_shared<StaticObject>(StaticObject(  //
                    glm::vec2{10, i},                         //
                    glm::vec2{0.6f, 0.6f},                    //
                    0,                                        //
                    gen_rand_vec4(0.3f, 1.0f),                //
                    fishSprites[1]                            //
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
            return EntityHelper::ForEachFlow::None;
        });

        EntityHelper::forEach<Enemy>([&](auto enemy) {  //
            if (aabb(enemy->getRect(), player->getRect())) {
                player->health -= enemy->dmg;
            }
            return EntityHelper::ForEachFlow::None;
        });

        EntityHelper::forEach<Experience>([&](auto exp) {  //
            if (aabb(exp->getRect(), player->getRect())) {
                player->experience += exp->amount;
                exp->cleanup = true;
            }
            return EntityHelper::ForEachFlow::None;
        });

        EntityHelper::forEach<Projectile>([&](auto proj) {
            if (proj->traveled > proj->range) {
                proj->cleanup = true;
                return EntityHelper::ForEachFlow::Continue;
            }
            EntityHelper::forEach<Enemy>([&](auto enemy) {  //
                if (aabb(proj->getRect(), enemy->getRect())) {
                    enemy->health -= proj->owner->dmg;
                    proj->cleanup = true;
                    return EntityHelper::ForEachFlow::Break;
                }
                return EntityHelper::ForEachFlow::None;
            });
            return EntityHelper::ForEachFlow::None;
        });

        int numEnemies = EntityHelper::numEntitiesOfType<Enemy>();
        while (numEnemies < MAX_ENEMIES) {
            EntityHelper::addEntity(std::make_shared<Enemy>(
                Enemy(glm::vec2{randIn(10, 100) * cos(numEnemies),
                                randIn(10, 100) * sin(numEnemies)},
                      glm::vec2{0.6f, 0.6f}, 0,
                      glm::vec4{gen_rand_vec3(0.3f, 1.0f), 1.f},
                      fishSprites[numEnemies % fishSprites.size()])));
            numEnemies += 1;
        }
    }

    void render() {
        Renderer::begin(cameraController->camera);
        // should go underneath entities also

        EntityHelper::forEachEntity([&](auto entity) {  //
            entity->render();
            return EntityHelper::ForEachFlow::None;
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

        if (player->experience >= player->expNeededForNextLevel) {
            player->experience -= player->expNeededForNextLevel;
            player->level += 1;
            // TODO show upgrade ui
        }

        if (player->health > 0) {
            child_updates(dt);  // move things around
        }

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

