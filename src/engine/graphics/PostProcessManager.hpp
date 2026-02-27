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

    void setBlurStrength(float strength) { m_blurStrength = strength; }
    void setRadialBlur(sf::Vector2f center, float strength) { m_radialCenter = center; m_radialStrength = strength; }

    void updateAudioData(float amplitude, float bass, const std::vector<float>& fft);

    void addShake(float intensity, sf::Time duration);

    void checkShaderReload();

private:
    bool loadShaders();
    void applyBloom();
    void applyBlur(sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f direction);
    void applyChromaticAberration();
    void applyExternalBlur();

    sf::RenderTexture m_mainTexture;
    sf::RenderTexture m_pingPongTextures[2];
    sf::RenderTexture m_extractTexture;
    sf::RenderTexture m_externalBlurTexture;

    sf::Shader m_bloomExtractShader;
    sf::Shader m_blurShader;
    sf::Shader m_bloomCombineShader;
    sf::Shader m_chromaticAberrationShader;
    sf::Shader m_backgroundShader;
    sf::Shader m_radialBlurShader;

    bool m_bloomEnabled{true};
    float m_bloomThreshold{0.7f};
    float m_bloomIntensity{1.2f};

    bool m_chromaticEnabled{true};
    float m_chromaticAmount{0.005f};

    float m_blurStrength{0.f};
    float m_radialStrength{0.f};
    sf::Vector2f m_radialCenter{640.f, 360.f};

    float m_amplitude{0.f};
    float m_bass{0.f};
    float m_fft[64]{0.f};
    float m_time{0.f};

    float m_shakeIntensity{0.f};
    sf::Time m_shakeDuration{sf::Time::Zero};
    sf::Time m_shakeRemaining{sf::Time::Zero};
    sf::Vector2f m_shakeOffset{0.f, 0.f};

    uint32_t m_width;
    uint32_t m_height;
    bool m_shadersLoaded{false};

    struct ShaderFileInfo {
        std::string path;
        long long lastModified{0};
    };
    std::vector<ShaderFileInfo> m_shaderFiles;
    sf::Time m_reloadTimer{sf::Time::Zero};
};

} // namespace engine::graphics
