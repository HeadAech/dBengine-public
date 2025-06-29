//
// Created by Hubert Klonowski on 19/03/2025.
//

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H
#include <Singletons/Ref/Ref.h>
#include <deque>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Component/Component.h"
#include "Components/Camera/Camera.h"
#include "Signal/Signals.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include "fmod_studio.hpp"

class AudioSource : public Component {
private:
    Ref &ref = Ref::GetInstance();

    FMOD_RESULT F_CALL programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event,
                                               void *parameters);
    std::vector<std::pair<int, int>> computeBandLimits(const FMOD_DSP_PARAMETER_FFT *dspFFT, float samplingFrequency,
                                                       int windowSize, int numDesiredBands);
    bool wasBeatRecently = false;
    float lastBassDetectionTime;
    float bassCooldown = 0.2f;
    float currentTime = 0.0f;

public:
    float currentBPM;
    FMOD::Studio::EventDescription *eventDescription;
    std::vector<FMOD::Studio::EventInstance *> activeEvents;
    FMOD::Studio::Bank *bank;
    float attackStrength = 0.5;
    std::vector<std::string> eventNames;
    bool isAllPaused = false;
    glm::vec3 pos;
    std::string bankPath;
    FMOD::ChannelGroup *ChannelGroup;

    int WindowSize = 1024;
    float SamplingFrequency = 0;

    typedef std::deque<std::vector<float>> FFTHistoryContainer;
    int FFTHistoryMaxSize = 0;
    bool affectedByDSP;
    FFTHistoryContainer FFTHistoryBeatDetector;
    std::vector<int> BeatDetectorBandLimits;


    void fillAverageSpectrum(std::vector<float> &averageSpectrum, const FFTHistoryContainer &fftHistory);

    void fillVarianceSpectrum(std::vector<float> &varianceSpectrum, const FFTHistoryContainer &fftHistory,
                              const std::vector<float> &averageSpectrum);


    float beatThreshold(float variance);

    AudioSource(std::string filePath, FMOD_STUDIO_LOAD_BANK_FLAGS flag, bool affectedByDSP = false);
    ~AudioSource();

    void AddToEvents(const std::string &eventName);
    void Play(const std::string &eventName);
    void PlayWithVariation(const std::string &eventName);
    void Stop(const std::string &eventName);
    void PlayAll();
    void ResumeAll();
    void PauseAll();
    void StopAll();
    void Update(float deltaTime);

    void SetPosition(const glm::vec3 &position);
    void SetVolume(const std::string &eventName, float volume);
    void SetPitch(const std::string &eventName, float pitch);
    void SetVolumeAll(float volume);
    void SetPitchAll(float pitch);

    float GetVolumeFromAll();
    float GetPitchFromAll();

    void SetParameter(const std::string &eventName, const std::string &paramName, float value);
    std::string GetParameterList(const std::string &eventName);

    std::string GetActiveEventList();
    std::string GetBankEventList();

    glm::vec3 GetPos();
    void SetPos(glm::vec3 pos);

    std::string GetBankPath();

    FMOD_STUDIO_LOAD_BANK_FLAGS flag;
};

#endif // AUDIOSOURCE_H
