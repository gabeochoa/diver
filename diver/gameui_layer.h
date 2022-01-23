
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
        if (upgradeWindowOpen) {
            auto appSettings = App::getSettings();
            using namespace IUI;
            uicontext->begin(gameUICameraController);

            float windowWidth = appSettings.width * 0.3;
            float windowHeight = appSettings.height * 0.8;

            auto window_location = getPositionSizeForUIRect({
                appSettings.width / 2.f - windowWidth / 2.f,  //
                appSettings.height - windowHeight,            //
                windowWidth,                                  //
                windowHeight,                                 //
            });
            uuid window_id = MK_UUID(id);
            if (window(window_id, WidgetConfig({
                                      .color = blue,
                                      .position = window_location[0],
                                      .size = window_location[1],
                                  })  //
                       )) {
                auto textConfig = WidgetConfig({
                    .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                    .position = convertUIPos({0, 100.f + H1_FS + 1.f}),
                    .size = glm::vec2{H1_FS, H1_FS},
                    .text = "Upgrade",
                    .flipTextY = true,
                });
                text(MK_UUID(id), textConfig);

                auto upgradeOptions =
                    GLOBALS.get<Player>("player").getUpgradeOptions();
                const int numButtons = upgradeOptions.size();

                for (int i = 0; i < numButtons; i++) {
                    if (button(MK_UUID_LOOP(id, i),
                               WidgetConfig(
                                   {.position = convertUIPos(
                                        {appSettings.width / 2,
                                         appSettings.height / 2.f + 32.f * i}),
                                    .color = glm::vec4{1.f},
                                    .size = glm::vec2{3 * 32.f, 32.f},
                                    .text = upgradeOptions[i].title,
                                    .flipTextY = true}))) {
                        upgradeWindowOpen = false;
                        upgradeOptions[i].apply();
                    }
                }
            }

            uicontext->end();
        }
    }

    void render() {
        auto appSettings = App::getSettings();
        gameUICameraController->camera.setProjection(0.f, appSettings.width,
                                                     appSettings.height, 0.f);
        Renderer::begin(gameUICameraController->camera);
        // // // // // // // //
        renderExperienceBar();
        renderUpgradeWindow();
        // // // // // // // //
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

