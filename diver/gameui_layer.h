
#pragma once

#include "../vendor/supermarket-engine/engine/app.h"
#include "../vendor/supermarket-engine/engine/camera.h"
#include "../vendor/supermarket-engine/engine/globals.h"
#include "../vendor/supermarket-engine/engine/layer.h"
#include "../vendor/supermarket-engine/engine/pch.hpp"
#include "../vendor/supermarket-engine/engine/renderer.h"
#include "../vendor/supermarket-engine/engine/ui.h"
#include "entities.h"

struct GameUILayer : public Layer {
    const glm::vec2 camTopLeft = {35.f, 19.5f};
    const glm::vec2 camBottomRight = {35.f, -18.f};
    glm::vec4 rect = glm::vec4{200.f, 1000.f, 1500.f, 200.f};
    std::shared_ptr<IUI::UIContext> uicontext;
    std::shared_ptr<OrthoCameraController> gameUICameraController;

    bool upgradeWindowOpen;
    std::vector<Player::Upgrade> upgradeOptions;

    const float H1_FS = 64.f;
    const float P_FS = 32.f;

    GameUILayer() : Layer("Game UI") {
        isMinimized = true;

        upgradeWindowOpen = false;
        GLOBALS.set<bool>("gameui_upgrade", &upgradeWindowOpen);

        auto appSettings = App::getSettings();

        gameUICameraController.reset(
            new OrthoCameraController(appSettings.ratio));
        GLOBALS.set<OrthoCameraController>("gameUICameraController",
                                           gameUICameraController.get());

        gameUICameraController->setZoomLevel(20.f);
        gameUICameraController->camera.setViewport(
            {0, 0, appSettings.width, appSettings.height});
        gameUICameraController->movementEnabled = false;
        gameUICameraController->rotationEnabled = false;
        gameUICameraController->zoomEnabled = false;
        gameUICameraController->resizeEnabled = false;

        uicontext.reset(new IUI::UIContext());
        uicontext->init();
    }

    virtual ~GameUILayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    glm::vec2 convertUIPos(glm::vec2 pos, bool flipy = true) {
        auto y = flipy ? App::getSettings().height - pos.y : pos.y;
        return screenToWorld(glm::vec3{pos.x, y, 0.f},
                             gameUICameraController->camera.view,
                             gameUICameraController->camera.projection,
                             gameUICameraController->camera.viewport);
    }

    std::array<glm::vec2, 2> getPositionSizeForUIRect(glm::vec4 uirect) {
        glm::vec2 position = convertUIPos(glm::vec2{uirect.x, uirect.y});
        glm::vec2 size = convertUIPos(glm::vec2{uirect.z, uirect.w});
        return std::array<glm::vec2, 2>{
            position + (size * 0.5f),
            size,
        };
    }

    // TODO add text that shows the numbers
    void renderExperienceBar() {
        auto player = GLOBALS.get<Player>("player");
        float expPct = player.experience / player.expNeededForNextLevel;
        Renderer::drawQuad(glm::vec2{0, App::getSettings().height - 10.f},
                           glm::vec2{expPct * App::getSettings().width, 10.f},
                           glm::vec4{0.1f, 0.8f, 0.f, 1.f}, "white");
    }

    void renderUpgradeWindow() {
        if (!upgradeWindowOpen) {
            return;
        }
        auto appSettings = App::getSettings();
        using namespace IUI;

        const float windowWidth = appSettings.width * 0.3;
        const float windowHeight = appSettings.height * 0.5;

        auto window_location = getPositionSizeForUIRect({
            appSettings.width / 2.f - windowWidth / 2.f,  //
            appSettings.height - windowHeight,            //
            windowWidth,                                  //
            windowHeight,                                 //
        });

        const uuid window_id = MK_UUID(id);
        if (window(window_id, WidgetConfig({
                                  .color = glm::vec4{0.8, 0.3, 0.f, 1.f},
                                  .position = window_location[0],
                                  .size = window_location[1],
                              })  //
                   )) {
            // text doesnt need a uuid so its okay if they all have the same
            // one
            auto textuuid = MK_UUID(id);

            auto player = GLOBALS.get<Player>("player");

            if (upgradeOptions.empty()) {
                auto upgrades = player.getUpgradeOptions();
                upgradeOptions.insert(upgradeOptions.end(), &upgrades[0],
                                      &upgrades[upgrades.size()]);
            }

            float startX = appSettings.width / 2.f - windowWidth / 2.f;

            // TODO it would be cool to add (+10) to the label when hovering
            // over the button
            std::array<std::string, 5> texts = {
                "Weapons",
                // TODO add weapons
                fmt::format("Max Health: {}", player.maxhealth),
                fmt::format("Regen Rate: {}", player.regenRate),
                fmt::format("Speed : {}", player.speed),
                "Current Stats",
            };
            glm::vec2 textPosition = glm::vec2{
                startX,
                appSettings.height - 100.f,
            };
            for (auto t : texts) {
                text(textuuid, WidgetConfig({.position = textPosition,
                                             .color = glm::vec4{1.f},
                                             .size = glm::vec2{24.f},
                                             .text = t,
                                             .flipTextY = true}));
                textPosition.y -= 24.f;
            }

            const int numButtons = upgradeOptions.size();

            for (int i = 0; i < numButtons; i++) {
                float buttonHeight = 3.f * 32.f;
                float startY = appSettings.height / 2.f + buttonHeight * i;

                auto buttonConfig =
                    WidgetConfig({.position = convertUIPos({startX, startY}),
                                  .color = glm::vec4{1.f},
                                  .size = glm::vec2{windowWidth, buttonHeight},
                                  .text = "**********",
                                  .flipTextY = true});

                if (button(MK_UUID_LOOP(id, i), buttonConfig)) {
                    upgradeWindowOpen = false;
                    upgradeOptions[i].apply();
                    upgradeOptions.clear();
                }

                text(textuuid,
                     WidgetConfig({.position = buttonConfig.position +
                                               glm::vec2{0, 36.f},
                                   .color = glm::vec4{0.f, 0.f, 0.f, 1.f},
                                   .size = glm::vec2{32.f, 32.f},
                                   .text = upgradeOptions[i].title,
                                   .flipTextY = true}));

                text(textuuid,
                     WidgetConfig({.position = buttonConfig.position +
                                               glm::vec2{0, 64.f},
                                   .color = glm::vec4{0.f, 0.f, 0.f, 1.f},
                                   .size = glm::vec2{16.f, 16.f},
                                   .text = upgradeOptions[i].description,
                                   .flipTextY = true}));
            }
        }
    }

    void renderTimer() {
        // auto appSettings = App::getSettings();
        using namespace IUI;
        auto textuuid = MK_UUID(id);

        int time_since_start = (int)GLOBALS.get<float>("time_since_start");
        int minutes = time_since_start / 60;
        int seconds = time_since_start % 60;

        text(textuuid, WidgetConfig({.position = glm::vec2{0, 4.f + 32.f},
                                     .color = glm::vec4{0.1f, 0.8f, 0.f, 1.f},
                                     .size = glm::vec2{32.f, 32.f},
                                     .text = fmt::format("Time: {:0>2}:{:0>2}",
                                                         minutes, seconds),
                                     .flipTextY = true}));
    }

    void render() {
        auto appSettings = App::getSettings();
        gameUICameraController->camera.setProjection(0.f, appSettings.width,
                                                     appSettings.height, 0.f);
        Renderer::begin(gameUICameraController->camera);
        uicontext->begin(gameUICameraController);
        // // // // // // // //
        renderTimer();
        renderExperienceBar();
        renderUpgradeWindow();
        // // // // // // // //
        uicontext->end();
        Renderer::end();
    }

    virtual void onUpdate(Time dt) override {
        // if (Menu::get().state != Menu::State::Game) return;

        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());
        prof give_me_a_name(__PROFILE_FUNC__);  //

        gameUICameraController->onUpdate(dt);
        render();  // draw everything
    }

    virtual void onEvent(Event& event) override {
        // if (Menu::get().state != Menu::State::Game) return;
        gameUICameraController->onEvent(event);
    }
};

