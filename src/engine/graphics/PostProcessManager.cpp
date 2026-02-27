#include "PostProcessManager.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <fmt/core.h>

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

    // Load shaders
    bool s1 = m_bloomExtractShader.loadFromFile("shaders/default.vert", "shaders/bloom_extract.frag");
    bool s2 = m_blurShader.loadFromFile("shaders/default.vert", "shaders/blur.frag");
    bool s3 = m_bloomCombineShader.loadFromFile("shaders/default.vert", "shaders/bloom_combine.frag");
    bool s4 = m_chromaticAberrationShader.loadFromFile("shaders/default.vert", "shaders/chromatic_aberration.frag");

    m_shadersLoaded = s1 && s2 && s3 && s4;

    if (!m_shadersLoaded) {
        fmt::print(stderr, "PostProcessManager: One or more shaders failed to load. Falling back to simple rendering.\n");
    }

    // Configure shaders
    m_bloomExtractShader.setUniform("u_texture", sf::Shader::CurrentTexture);
    m_blurShader.setUniform("u_texture", sf::Shader::CurrentTexture);
    m_blurShader.setUniform("resolution", sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
    
    m_bloomCombineShader.setUniform("u_texture", sf::Shader::CurrentTexture);
    
    m_chromaticAberrationShader.setUniform("u_texture", sf::Shader::CurrentTexture);
}

void PostProcessManager::update(sf::Time dt) {
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
}

void PostProcessManager::addShake(float intensity, sf::Time duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeRemaining = duration;
}

void PostProcessManager::begin() {
    m_mainTexture.clear(sf::Color(20, 20, 40)); // Dark navy background
}

void PostProcessManager::end() {
    m_mainTexture.display();
}

void PostProcessManager::render(sf::RenderTarget& target) {
    sf::Sprite mainSprite(m_mainTexture.getTexture());
    mainSprite.setPosition(m_shakeOffset);

    if (m_shadersLoaded) {
        if (m_bloomEnabled) {
            applyBloom();
            m_bloomCombineShader.setUniform("bloomTexture", m_pingPongTextures[0].getTexture());
            m_bloomCombineShader.setUniform("intensity", m_bloomIntensity);
            
            if (m_chromaticEnabled) {
                m_extractTexture.clear(sf::Color::Transparent);
                m_extractTexture.draw(mainSprite, &m_bloomCombineShader);
                m_extractTexture.display();
                
                applyChromaticAberration();
                sf::Sprite finalSprite(m_pingPongTextures[1].getTexture());
                finalSprite.setPosition(m_shakeOffset);
                target.draw(finalSprite);
            } else {
                mainSprite.setPosition(m_shakeOffset);
                target.draw(mainSprite, &m_bloomCombineShader);
            }
        } else if (m_chromaticEnabled) {
            applyChromaticAberration();
            sf::Sprite finalSprite(m_pingPongTextures[1].getTexture());
            finalSprite.setPosition(m_shakeOffset);
            target.draw(finalSprite);
        } else {
            target.draw(mainSprite);
        }
    } else {
        target.draw(mainSprite);
    }
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
