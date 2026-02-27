#include "AudioCore.hpp"
#include <fmt/core.h>
#include <cmath>
#include <algorithm>
#include <atomic>

namespace engine::audio {

AudioCore::AudioCore() : m_music(std::make_unique<sf::Music>()) {
    m_fftResult.resize(64, 0.f); // 64 frequency bins
}

AudioCore::~AudioCore() = default;

bool AudioCore::loadMusic(const std::string& path) {
    if (!m_music->openFromFile(path)) {
        fmt::print(stderr, "Failed to load music from {}\n", path);
        return false;
    }

    m_musicBuffer.clear(); // Not using memory buffer
    
    m_sampleRate = m_music->getSampleRate();
    m_totalSamplesProcessed = 0;

    // Attach effect processor
    m_music->setEffectProcessor([this](const float* input, std::size_t frameCount, 
                                       float* output, std::size_t outputFrameCount, 
                                       unsigned int channelCount) {
        (void)outputFrameCount;
        std::copy(input, input + (frameCount * channelCount), output);
        this->processAudio(input, frameCount, channelCount);
    });

    return true;
}

bool AudioCore::loadMusicFromMemory(std::vector<char> buffer) {
    m_musicBuffer = std::move(buffer);
    if (!m_music->openFromMemory(m_musicBuffer.data(), m_musicBuffer.size())) {
        fmt::print(stderr, "Failed to load music from memory\n");
        m_musicBuffer.clear();
        return false;
    }

    m_sampleRate = m_music->getSampleRate();
    m_totalSamplesProcessed = 0;

    m_music->setEffectProcessor([this](const float* input, std::size_t frameCount, 
                                       float* output, std::size_t outputFrameCount, 
                                       unsigned int channelCount) {
        (void)outputFrameCount;
        std::copy(input, input + (frameCount * channelCount), output);
        this->processAudio(input, frameCount, channelCount);
    });

    return true;
}

void AudioCore::processAudio(const float* input, std::size_t frameCount, unsigned int channelCount) {
    std::lock_guard lock(m_audioMutex);
    
    float sumSquares = 0.f;
    for (std::size_t i = 0; i < frameCount; ++i) {
        float sample = input[i * channelCount];
        sumSquares += sample * sample;
        
        m_sampleBuffer[m_writePos] = sample;
        m_writePos = (m_writePos + 1) % BUFFER_SIZE;
    }

    // RMS Amplitude
    m_amplitude = std::sqrt(sumSquares / static_cast<float>(frameCount));
    m_totalSamplesProcessed += frameCount;
}

void AudioCore::performFFT() {
    std::lock_guard lock(m_audioMutex);
    
    // Very crude pseudo-FFT for now
    size_t numBins = m_fftResult.size();
    size_t samplesPerBin = BUFFER_SIZE / numBins;

    float totalBass = 0.f;
    int bassBinCount = std::max(1, static_cast<int>(numBins / 8));

    for (size_t i = 0; i < numBins; ++i) {
        float binSum = 0.f;
        for (size_t j = 0; j < samplesPerBin; ++j) {
            size_t idx = (i * samplesPerBin + j) % BUFFER_SIZE;
            binSum += std::abs(m_sampleBuffer[idx]);
        }
        float targetVal = (samplesPerBin > 0) ? (binSum / static_cast<float>(samplesPerBin)) : 0.f;
        
        // Low-pass smoothing for visual stability
        m_fftResult[i] = m_fftResult[i] * 0.7f + targetVal * 0.3f;

        if (i < static_cast<size_t>(bassBinCount)) {
            totalBass += m_fftResult[i];
        }
    }

    m_bassEnergy = totalBass / static_cast<float>(bassBinCount);
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
    m_totalSamplesProcessed = 0;
}

void AudioCore::setVolume(float volume) {
    m_music->setVolume(volume);
}

float AudioCore::getVolume() const {
    return m_music->getVolume();
}

void AudioCore::update() {
    if (m_isPlaying && m_music->getStatus() == sf::SoundSource::Status::Playing) {
        m_lastPolledPosition = m_music->getPlayingOffset();
        m_smoothedPosition = getSampleTime(); 
        
        performFFT();
    }
}

sf::Time AudioCore::getPlaybackPosition() const {
    return m_lastPolledPosition;
}

sf::Time AudioCore::getSmoothedPosition() const {
    return m_smoothedPosition;
}

sf::Time AudioCore::getSampleTime() const {
    if (m_sampleRate == 0) return sf::Time::Zero;
    double seconds = static_cast<double>(m_totalSamplesProcessed.load()) / static_cast<double>(m_sampleRate);
    return sf::seconds(static_cast<float>(seconds));
}

sf::Time AudioCore::getTotalDuration() const {
    return m_music->getDuration();
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
