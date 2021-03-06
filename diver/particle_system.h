
#pragma once

#include "../vendor/supermarket-engine/engine/external_include.h"
#include "../vendor/supermarket-engine/engine/math.h"
#include "../vendor/supermarket-engine/engine/renderer.h"
#include "../vendor/supermarket-engine/engine/time.h"
#include "../vendor/supermarket-engine/engine/vecutil.h"

struct ParticleSystem {
    struct Particle {
        glm::vec2 position;
        glm::vec2 velocity;
        float angle = 0.f;
        float angularVelocity = 0.f;

        glm::vec4 colorStart = glm::vec4{1.f};
        glm::vec4 colorEnd = glm::vec4{1.f};

        glm::vec2 size;
        float sizeStart = 1.f;
        float sizeEnd = 1.f;

        float lifetime = 1.f;
        float lifeRemaining = 0.f;

        bool active = false;

        void render() {
            float life = lifeRemaining / lifetime;
            float sizeMult = lerp(sizeEnd, sizeStart, life);

            glm::vec4 color = lerp(colorEnd, colorStart, life);
            color.a = color.a * life;

            // computing angle transforms are expensive so
            // if the angle is under thresh, just render it square
            if (angle <= 5.f) {
                Renderer::drawQuad(position, size * sizeMult, color, "white");
            } else {
                Renderer::drawQuadRotated(position, size * sizeMult,
                                          glm::radians(angle), color, "white");
            }
        }
    };
    std::vector<Particle> pool;
    int poolIndex = 999;

    ParticleSystem() { pool.resize(1000); }

    void emit(Particle p) {
        Particle& particle = pool[poolIndex];
        particle.active = true;

        particle.position = p.position;
        particle.velocity = p.velocity;

        particle.angularVelocity = p.angularVelocity;
        particle.angle = p.angle;

        particle.colorStart = p.colorStart;
        particle.colorEnd = p.colorEnd;

        particle.lifetime = p.lifetime;
        particle.lifeRemaining = p.lifetime;

        particle.size = p.size;
        particle.sizeStart = p.sizeStart;
        particle.sizeEnd = p.sizeEnd;

        poolIndex = --poolIndex % pool.size();
    }

    void onUpdate(Time dt) {
        for (auto& particle : pool) {
            if (!particle.active) continue;
            if (particle.lifeRemaining <= 0.f) {
                particle.active = false;
                continue;
            }
            particle.lifeRemaining -= dt.s();
            particle.position += particle.velocity * dt.s();
            particle.angle += particle.angularVelocity * dt.s();
        }
    }

    void render() {
        Renderer::drawQuad(glm::vec2{0.f}, glm::vec2{1.f},
                           glm::vec4{0.3, 0.2, 0.8, 1.f}, "white");
        for (auto& particle : pool) {
            if (!particle.active) continue;
            particle.render();
        }
    }
};

