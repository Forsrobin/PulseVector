#include "PostProcessManager.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <fmt/core.h>
#include <filesystem>

namespace engine::graphics {

PostProcessManager::PostProcessManager(uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {
    
    if (!m_mainTexture.resize({width, height})) {
        fmt::print(stderr, "PostProcessManager: Failed to resize main texture\n");
    }
    if (!m_pingPongTextures[0].resize({width, height})) {
        fmt::print(stderr, "PostProcessManager: Failed to resize pingpong 0\n");
    }
    if (!m_pingPongTextures[1].resize({width, height})) {
        fmt::print(stderr, "PostProcessManager: Failed to resize pingpong 1\n");
    }
    if (!m_extractTexture.resize({width, height})) {
        fmt::print(stderr, "PostProcessManager: Failed to resize extract texture\n");
    }
    if (!m_externalBlurTexture.resize({width, height})) {
        fmt::print(stderr, "PostProcessManager: Failed to resize external blur texture\n");
    }

    // Initialize shader file tracking
    m_shaderFiles = {
        {"shaders/bloom_extract.frag", 0},
        {"shaders/blur.frag", 0},
        {"shaders/bloom_combine.frag", 0},
        {"shaders/chromatic_aberration.frag", 0},
        {"shaders/audio_reactive_bg.frag", 0},
        {"shaders/radial_blur.frag", 0}
    };

    m_shadersLoaded = loadShaders();
}

bool PostProcessManager::loadShaders() {
    bool s1 = m_bloomExtractShader.loadFromFile("shaders/default.vert", "shaders/bloom_extract.frag");
    bool s2 = m_blurShader.loadFromFile("shaders/default.vert", "shaders/blur.frag");
    bool s3 = m_bloomCombineShader.loadFromFile("shaders/default.vert", "shaders/bloom_combine.frag");
    bool s4 = m_chromaticAberrationShader.loadFromFile("shaders/default.vert", "shaders/chromatic_aberration.frag");
    bool s5 = m_backgroundShader.loadFromFile("shaders/default.vert", "shaders/audio_reactive_bg.frag");
    bool s6 = m_radialBlurShader.loadFromFile("shaders/default.vert", "shaders/radial_blur.frag");

    bool allLoaded = s1 && s2 && s3 && s4 && s5 && s6;

    if (allLoaded) {
        sf::Vector2f res(static_cast<float>(m_width), static_cast<float>(m_height));

        // Configure shaders
        m_bloomExtractShader.setUniform("u_texture", sf::Shader::CurrentTexture);
        m_bloomExtractShader.setUniform("resolution", res);

        m_blurShader.setUniform("u_texture", sf::Shader::CurrentTexture);
        m_blurShader.setUniform("resolution", res);
        
        m_bloomCombineShader.setUniform("u_texture", sf::Shader::CurrentTexture);
        m_bloomCombineShader.setUniform("resolution", res);
        
        m_chromaticAberrationShader.setUniform("u_texture", sf::Shader::CurrentTexture);
        m_chromaticAberrationShader.setUniform("resolution", res);

        m_backgroundShader.setUniform("resolution", res);

        m_radialBlurShader.setUniform("u_texture", sf::Shader::CurrentTexture);
        m_radialBlurShader.setUniform("resolution", res);

        // Update timestamps
        for (auto& file : m_shaderFiles) {
            try {
                file.lastModified = std::filesystem::last_write_time(file.path).time_since_epoch().count();
            } catch (...) {}
        }
    }

    return allLoaded;
}

void PostProcessManager::checkShaderReload() {
    bool needsReload = false;
    for (auto& file : m_shaderFiles) {
        try {
            if (std::filesystem::exists(file.path)) {
                auto currentMod = std::filesystem::last_write_time(file.path).time_since_epoch().count();
                if (currentMod > file.lastModified) {
                    needsReload = true;
                    break;
                }
            }
        } catch (...) {}
    }

    if (needsReload) {
        fmt::print("PostProcessManager: Shader change detected! Reloading...\n");
        if (loadShaders()) {
            fmt::print("PostProcessManager: Shaders reloaded successfully.\n");
            m_shadersLoaded = true;
        } else {
            fmt::print(stderr, "PostProcessManager: Shader reload failed.\n");
        }
    }
}

void PostProcessManager::updateAudioData(float amplitude, float bass, const std::vector<float>& fft) {
    m_amplitude = amplitude;
    m_bass = bass;
    for (size_t i = 0; i < 64 && i < fft.size(); ++i) {
        m_fft[i] = fft[i];
    }
}

void PostProcessManager::update(sf::Time dt) {
    m_time += dt.asSeconds();
    
    // Check for shader reloads every 0.5s to avoid excessive file I/O
    m_reloadTimer += dt;
    if (m_reloadTimer >= sf::seconds(0.5f)) {
        checkShaderReload();
        m_reloadTimer = sf::Time::Zero;
    }

    if (m_shakeRemaining > sf::Time::Zero) {
        m_shakeRemaining -= dt;
        if (m_shakeRemaining <= sf::Time::Zero) {
            m_shakeOffset = {0.f, 0.f};
        } else {
            float factor = m_shakeRemaining.asSeconds() / m_shakeDuration.asSeconds();
            float currentIntensity = m_shakeIntensity * factor;
            m_shakeOffset.x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.f - 1.f) * currentIntensity;
            m_shakeOffset.y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.f - 1.f) * currentIntensity;
        }
    }

    if (m_radialStrength > 0.f) {
        m_radialStrength -= dt.asSeconds() * 2.0f; // Rapid decay
        if (m_radialStrength < 0.f) m_radialStrength = 0.f;
    }
}

void PostProcessManager::addShake(float intensity, sf::Time duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeRemaining = duration;
}

void PostProcessManager::begin() {
    m_mainTexture.clear(sf::Color(20, 20, 40)); // Dark navy background fallback

    if (m_shadersLoaded) {
        m_backgroundShader.setUniform("time", m_time);
        m_backgroundShader.setUniform("amplitude", m_amplitude);
        m_backgroundShader.setUniform("bass", m_bass);
        m_backgroundShader.setUniformArray("fft", m_fft, 64);

        sf::RectangleShape quad({static_cast<float>(m_width), static_cast<float>(m_height)});
        m_mainTexture.draw(quad, &m_backgroundShader);
    }
}

void PostProcessManager::end() {
    m_mainTexture.display();
}

void PostProcessManager::render(sf::RenderTarget& target) {
    if (!m_shadersLoaded) {
        sf::Sprite mainSprite(m_mainTexture.getTexture());
        mainSprite.setPosition(m_shakeOffset);
        target.draw(mainSprite);
        return;
    }

    // Pipeline: Bloom -> Chromatic Aberration -> External Blur (Restart)

    // 1. Bloom
    if (m_bloomEnabled) {
        applyBloom();
        m_bloomCombineShader.setUniform("bloomTexture", m_pingPongTextures[0].getTexture());
        m_bloomCombineShader.setUniform("intensity", m_bloomIntensity);
        
        m_extractTexture.clear(sf::Color::Transparent);
        sf::Sprite mainSprite(m_mainTexture.getTexture());
        m_extractTexture.draw(mainSprite, &m_bloomCombineShader);
        m_extractTexture.display();
    } else {
        m_extractTexture.clear(sf::Color::Transparent);
        sf::Sprite mainSprite(m_mainTexture.getTexture());
        m_extractTexture.draw(mainSprite);
        m_extractTexture.display();
    }

    // 2. Chromatic Aberration
    if (m_chromaticEnabled) {
        applyChromaticAberration();
    } else {
        m_pingPongTextures[1].clear(sf::Color::Transparent);
        sf::Sprite sprite(m_extractTexture.getTexture());
        m_pingPongTextures[1].draw(sprite);
        m_pingPongTextures[1].display();
    }

    // 3. External Blur (for Restart)
    if (m_blurStrength > 0.01f) {
        applyExternalBlur();
        sf::Sprite finalSprite(m_externalBlurTexture.getTexture());
        finalSprite.setPosition(m_shakeOffset);
        
        if (m_radialStrength > 0.01f) {
            m_radialBlurShader.setUniform("center", m_radialCenter);
            m_radialBlurShader.setUniform("strength", m_radialStrength);
            target.draw(finalSprite, &m_radialBlurShader);
        } else {
            target.draw(finalSprite);
        }
    } else {
        sf::Sprite finalSprite(m_pingPongTextures[1].getTexture());
        finalSprite.setPosition(m_shakeOffset);
        
        if (m_radialStrength > 0.01f) {
            m_radialBlurShader.setUniform("center", m_radialCenter);
            m_radialBlurShader.setUniform("strength", m_radialStrength);
            target.draw(finalSprite, &m_radialBlurShader);
        } else {
            target.draw(finalSprite);
        }
    }
}

void PostProcessManager::applyExternalBlur() {
    // We use ping-pong 0 as intermediate
    m_blurShader.setUniform("direction", sf::Vector2f(m_blurStrength / static_cast<float>(m_width), 0.f));
    m_pingPongTextures[0].clear(sf::Color::Transparent);
    sf::Sprite s1(m_pingPongTextures[1].getTexture());
    m_pingPongTextures[0].draw(s1, &m_blurShader);
    m_pingPongTextures[0].display();

    m_blurShader.setUniform("direction", sf::Vector2f(0.f, m_blurStrength / static_cast<float>(m_height)));
    m_externalBlurTexture.clear(sf::Color::Transparent);
    sf::Sprite s2(m_pingPongTextures[0].getTexture());
    m_externalBlurTexture.draw(s2, &m_blurShader);
    m_externalBlurTexture.display();
}

void PostProcessManager::applyBloom() {
    m_bloomExtractShader.setUniform("threshold", m_bloomThreshold);
    
    m_extractTexture.clear(sf::Color::Transparent);
    sf::Sprite mainSprite(m_mainTexture.getTexture());
    m_extractTexture.draw(mainSprite, &m_bloomExtractShader);
    m_extractTexture.display();

    // Blur horizontal
    applyBlur(m_extractTexture, m_pingPongTextures[0], {1.f, 0.f});
    // Blur vertical
    applyBlur(m_pingPongTextures[0], m_pingPongTextures[1], {0.f, 1.f});
    
    // Result is in m_pingPongTextures[1], copy to [0]
    m_pingPongTextures[0].clear(sf::Color::Transparent);
    sf::Sprite blurSprite(m_pingPongTextures[1].getTexture());
    m_pingPongTextures[0].draw(blurSprite);
    m_pingPongTextures[0].display();
}

void PostProcessManager::applyBlur(sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f direction) {
    m_blurShader.setUniform("direction", direction);
    output.clear(sf::Color::Transparent);
    sf::Sprite sprite(input.getTexture());
    output.draw(sprite, &m_blurShader);
    output.display();
}

void PostProcessManager::applyChromaticAberration() {
    m_chromaticAberrationShader.setUniform("amount", m_chromaticAmount);
    
    m_pingPongTextures[1].clear(sf::Color::Transparent);
    sf::Sprite inputSprite(m_bloomEnabled ? m_extractTexture.getTexture() : m_mainTexture.getTexture());
    
    m_pingPongTextures[1].draw(inputSprite, &m_chromaticAberrationShader);
    m_pingPongTextures[1].display();
}

} // namespace engine::graphics
