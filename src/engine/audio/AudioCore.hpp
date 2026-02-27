#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/System/Time.hpp>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <mutex>

namespace engine::audio {

class AudioCore {
public:
    AudioCore();
    ~AudioCore();

    bool loadMusic(const std::string& path);
    void play();
    void pause();
    void stop();

    void setVolume(float volume);
    [[nodiscard]] float getVolume() const;

    void update();

    [[nodiscard]] sf::Time getPlaybackPosition() const;
    [[nodiscard]] sf::Time getSmoothedPosition() const;
    [[nodiscard]] bool isPlaying() const;

    void setPlaybackSpeed(float speed);
    [[nodiscard]] float getPlaybackSpeed() const;

    [[nodiscard]] float getAmplitude() const { return m_amplitude; }
    [[nodiscard]] const std::vector<float>& getFftData() const { return m_fftResult; }

private:
    void processAudio(const float* input, std::size_t frameCount, unsigned int channelCount);
    void performFFT();

    std::unique_ptr<sf::Music> m_music;
    sf::Time m_lastPolledPosition;
    sf::Time m_smoothedPosition;
    float m_playbackSpeed{1.0f};
    bool m_isPlaying{false};

    // Audio Analysis
    mutable std::mutex m_audioMutex;
    std::vector<float> m_sampleBuffer;
    std::vector<float> m_fftResult;
    float m_amplitude{0.f};
};

} // namespace engine::audio
