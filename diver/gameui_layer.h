
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

    const float H1_FS = 64.f;
    const float P_FS = 32.f;

    GameUILayer() : Layer("Game UI") {
        isMinimized = true;

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

    void renderExperienceBar() {
        auto player = GLOBALS.get<Player>("player");
        float expPct = player.experience / player.expNeededForNextLevel;
        Renderer::drawQuad(glm::vec2{0, App::getSettings().height - 10.f},
                           glm::vec2{expPct * App::getSettings().width, 10.f},
                           glm::vec4{0.1f, 0.8f, 0.f, 1.f}, "white");
    }

    void render() {
        auto appSettings = App::getSettings();
        gameUICameraController->camera.setProjection(0.f, appSettings.width,
                                                     appSettings.height, 0.f);
        Renderer::begin(gameUICameraController->camera);

        // // // // // // // //

        renderExperienceBar();

        using namespace IUI;
        uicontext->begin(gameUICameraController);
        uicontext->end();

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

