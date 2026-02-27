#include "AudioCore.hpp"
#include <fmt/core.h>
#include <cmath>
#include <algorithm>

namespace engine::audio {

AudioCore::AudioCore() : m_music(std::make_unique<sf::Music>()) {
    m_fftResult.resize(64, 0.f); // 64 frequency bins
}

AudioCore::~AudioCore() = default;

bool AudioCore::loadMusic(const std::string& path) {
    if (!m_music->openFromFile(path)) {
        fmt::print(stderr, "Failed to load music from {}\\n", path);
        return false;
    }

    // Attach effect processor
    m_music->setEffectProcessor([this](const float* input, std::size_t frameCount, 
                                       float* output, std::size_t outputFrameCount, 
                                       unsigned int channelCount) {
        (void)outputFrameCount;
        // Copy to output so sound plays
        std::copy(input, input + (frameCount * channelCount), output);
        
        // Analyze samples
        this->processAudio(input, frameCount, channelCount);
    });

    return true;
}

void AudioCore::processAudio(const float* input, std::size_t frameCount, unsigned int channelCount) {
    std::lock_guard lock(m_audioMutex);
    
    float sumSquares = 0.f;
    for (std::size_t i = 0; i < frameCount; ++i) {
        // Use first channel for simplicity
        float sample = input[i * channelCount];
        sumSquares += sample * sample;
        
        m_sampleBuffer.push_back(sample);
    }

    // RMS Amplitude
    m_amplitude = std::sqrt(sumSquares / static_cast<float>(frameCount));

    // Keep buffer at reasonable size for FFT (e.g. 1024 samples)
    if (m_sampleBuffer.size() > 1024) {
        m_sampleBuffer.erase(m_sampleBuffer.begin(), m_sampleBuffer.begin() + (m_sampleBuffer.size() - 1024));
    }
}

void AudioCore::performFFT() {
    std::lock_guard lock(m_audioMutex);
    if (m_sampleBuffer.empty()) return;

    // Very crude pseudo-FFT for now until a real FFT lib is integrated or implemented
    // Just mapping samples to bins to have some visual movement
    size_t numBins = m_fftResult.size();
    size_t samplesPerBin = m_sampleBuffer.size() / numBins;

    for (size_t i = 0; i < numBins; ++i) {
        float binSum = 0.f;
        for (size_t j = 0; j < samplesPerBin; ++j) {
            binSum += std::abs(m_sampleBuffer[i * samplesPerBin + j]);
        }
        float targetVal = (samplesPerBin > 0) ? (binSum / static_cast<float>(samplesPerBin)) : 0.f;
        // Smooth transition
        m_fftResult[i] = m_fftResult[i] * 0.8f + targetVal * 0.2f;
    }
}

void AudioCore::play() {
    if (m_music->getStatus() != sf::SoundSource::Status::Playing) {
        m_music->play();
        m_isPlaying = true;
    }
}

void AudioCore::pause() {
    if (m_music->getStatus() == sf::SoundSource::Status::Playing) {
        m_music->pause();
        m_isPlaying = false;
    }
}

void AudioCore::stop() {
    m_music->stop();
    m_isPlaying = false;
    m_lastPolledPosition = sf::Time::Zero;
    m_smoothedPosition = sf::Time::Zero;
}

void AudioCore::setVolume(float volume) {
    m_music->setVolume(volume);
}

float AudioCore::getVolume() const {
    return m_music->getVolume();
}

void AudioCore::update() {
    if (m_isPlaying && m_music->getStatus() == sf::SoundSource::Status::Playing) {
        // Poll exact position from audio hardware
        sf::Time currentPosition = m_music->getPlayingOffset();
        m_lastPolledPosition = currentPosition;
        m_smoothedPosition = currentPosition; 
        
        performFFT();
    }
}

sf::Time AudioCore::getPlaybackPosition() const {
    return m_lastPolledPosition;
}

sf::Time AudioCore::getSmoothedPosition() const {
    return m_smoothedPosition;
}

bool AudioCore::isPlaying() const {
    return m_isPlaying;
}

void AudioCore::setPlaybackSpeed(float speed) {
    m_playbackSpeed = speed;
    m_music->setPitch(speed);
}

float AudioCore::getPlaybackSpeed() const {
    return m_playbackSpeed;
}

} // namespace engine::audio
