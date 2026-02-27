#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <memory>
#include <vector>

namespace engine::graphics {

class PostProcessManager {
public:
    PostProcessManager(uint32_t width, uint32_t height);

    void update(sf::Time dt);
    void begin();
    void end();
    
    void render(sf::RenderTarget& target);

    sf::RenderTexture& getMainTexture() { return m_mainTexture; }

    void setBloomEnabled(bool enabled) { m_bloomEnabled = enabled; }
    void setBloomThreshold(float threshold) { m_bloomThreshold = threshold; }
    void setBloomIntensity(float intensity) { m_bloomIntensity = intensity; }
    
    void setChromaticAberrationEnabled(bool enabled) { m_chromaticEnabled = enabled; }
    void setChromaticAberrationAmount(float amount) { m_chromaticAmount = amount; }

    void addShake(float intensity, sf::Time duration);

private:
    void applyBloom();
    void applyBlur(sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f direction);
    void applyChromaticAberration();

    sf::RenderTexture m_mainTexture;
    sf::RenderTexture m_pingPongTextures[2];
    sf::RenderTexture m_extractTexture;

    sf::Shader m_bloomExtractShader;
    sf::Shader m_blurShader;
    sf::Shader m_bloomCombineShader;
    sf::Shader m_chromaticAberrationShader;

    bool m_bloomEnabled{true};
    float m_bloomThreshold{0.7f};
    float m_bloomIntensity{1.2f};

    bool m_chromaticEnabled{true};
    float m_chromaticAmount{0.005f};

    float m_shakeIntensity{0.f};
    sf::Time m_shakeDuration{sf::Time::Zero};
    sf::Time m_shakeRemaining{sf::Time::Zero};
    sf::Vector2f m_shakeOffset{0.f, 0.f};

    uint32_t m_width;
    uint32_t m_height;
    bool m_shadersLoaded{false};
};

} // namespace engine::graphics
