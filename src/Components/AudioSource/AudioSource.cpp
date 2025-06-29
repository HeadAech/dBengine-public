#include "AudioSource.h"
#include <Helpers/Util.h>
#include <Singletons/Ref/Ref.h>
#include <thread>


float pitch = 1.0;
AudioSource::AudioSource(std::string filePath, FMOD_STUDIO_LOAD_BANK_FLAGS flag, bool affectedByDSP) :
    bank(bank), eventDescription(nullptr), affectedByDSP(affectedByDSP) {

    bankPath = Util::getRelativePath(filePath).string();
    name = "Audio Source";
    icon = ICON_FA_VOLUME_UP;

    Signals::Music_AdjustVolume.connect(this->uuid, [this](float volume) {});
    Signals::Music_AdjustPitch.connect(this->uuid, [this](float pitch) {});
}

AudioSource::~AudioSource() {
    StopAll();

    Signals::Music_AdjustVolume.disconnect(this->uuid);
    Signals::Timer_OnTimeout.disconnect(this->uuid);
}

void AudioSource::AddToEvents(const std::string &eventName) {
    if (std::find(eventNames.begin(), eventNames.end(), eventName) != eventNames.end()) {
        EngineDebug::GetInstance().PrintWarning("Event already added: " + eventName);
        return;
    }

    FMOD::Studio::EventDescription *eventDescription = nullptr;
    ref.ERRCHECK(ref.fmodSystem->getEvent(eventName.c_str(), &eventDescription));


    FMOD::Studio::EventInstance *eventInstance = nullptr;
    ref.ERRCHECK(eventDescription->createInstance(&eventInstance));

    FMOD_3D_ATTRIBUTES attributes = {};
    attributes.position = {pos.x, pos.y, pos.z};
    attributes.forward = {0.0f, 0.0f, 1.0f};
    attributes.up = {0.0f, 1.0f, 0.0f};
    attributes.velocity = {0.0f, 0.0f, 0.0f};
    eventInstance->set3DAttributes(&attributes);

    activeEvents.push_back(eventInstance);
    eventNames.push_back(eventName);
}

/// <summary>
/// it is trash, get parameters from bank and we gucci.
/// </summary>
/// <param name="eventName"></param>
void AudioSource::PlayWithVariation(const std::string &eventName) {
    // Not good it seems, just variate it instead of playing i think.

    FMOD::Studio::EventDescription *eventDescription = nullptr;
    ref.ERRCHECK(ref.fmodSystem->getEvent(eventName.c_str(), &eventDescription));

    FMOD::Studio::EventInstance *eventInstance = nullptr;
    ref.ERRCHECK(eventDescription->createInstance(&eventInstance));

    FMOD_3D_ATTRIBUTES attributes = {};
    attributes.position = {pos.x, pos.y, pos.z};
    attributes.forward = {0.0f, 0.0f, 1.0f};
    attributes.up = {0.0f, 1.0f, 0.0f};
    attributes.velocity = {0.0f, 0.0f, 0.0f};
    eventInstance->set3DAttributes(&attributes);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pitchDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> noiseDist(0.0f, 1.0f);

    float pitchVariation = pitchDist(gen) + (attackStrength * 0.5f);
    float noiseLevel = noiseDist(gen) * attackStrength;
    eventInstance->setParameterByName("PitchVariation", pitchVariation);
    eventInstance->setParameterByName("NoiseLevel", noiseLevel);

    eventInstance->start();
    activeEvents.push_back(eventInstance);
    if (std::find(eventNames.begin(), eventNames.end(), eventName) == eventNames.end()) {
        eventNames.push_back(eventName);
    }
}

void AudioSource::Play(const std::string &eventName) {
    if (isAllPaused) {
        return;
    }

    // for (auto *instance: activeEvents) {
    //     FMOD::Studio::EventDescription *desc = nullptr;
    //     ref.ERRCHECK(instance->getDescription(&desc));
    //     char existingEventName[256];
    //     int retrieved = 0;
    //     ref.ERRCHECK(desc->getPath(existingEventName, sizeof(existingEventName), &retrieved));
    //     if (existingEventName && eventName == existingEventName) {
    //         FMOD_STUDIO_PLAYBACK_STATE state;
    //         ref.ERRCHECK(instance->getPlaybackState(&state));
    //         if (state != FMOD_STUDIO_PLAYBACK_PLAYING) {
    //             ref.ERRCHECK(instance->start());
    //         }
    //         return;
    //     }
    // }

    FMOD::Studio::EventDescription *eventDescription = nullptr;
    ref.ERRCHECK(ref.fmodSystem->getEvent(eventName.c_str(), &eventDescription));


    FMOD::Studio::EventInstance *eventInstance = nullptr;
    ref.ERRCHECK(eventDescription->createInstance(&eventInstance));

    FMOD_3D_ATTRIBUTES attributes = {};
    attributes.position = {pos.x, pos.y, pos.z};
    attributes.forward = {0.0f, 0.0f, 1.0f};
    attributes.up = {0.0f, 1.0f, 0.0f};
    attributes.velocity = {0.0f, 0.0f, 0.0f};
    eventInstance->set3DAttributes(&attributes);


    if (std::find(activeEvents.begin(), activeEvents.end(), eventInstance) == activeEvents.end()) {
        activeEvents.push_back(eventInstance);
    }
    if (std::find(eventNames.begin(), eventNames.end(), eventName) == eventNames.end()) {
        eventNames.push_back(eventName);
    }
    eventInstance->start();

    if (affectedByDSP) {
        for (int i = 0; i < 10; ++i) { // i do not want to print errors here, it will eventually load, right?
            if (eventInstance->getChannelGroup(&ChannelGroup) == FMOD_OK) {
                break;
            };
            ref.fmodSystem->update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (eventInstance->getChannelGroup(&ChannelGroup) != FMOD_OK) {
            EngineDebug::GetInstance().PrintError(
                    "Failed to get ChannelGroup after retries: " +
                    std::string(FMOD_ErrorString(eventInstance->getChannelGroup(&ChannelGroup))));
            return;
        }

        // Attach DSP to ChannelGroup
        ref.ERRCHECK(ChannelGroup->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, ref.fmodDSP));

        // Activate DSP
        ref.ERRCHECK(ref.fmodDSP->setActive(true));

        // Get sampling frequency from Core System
        int sampleRate, channels;
        FMOD_SPEAKERMODE speakerMode;
        ref.ERRCHECK(ref.fmodCoreSystem->getSoftwareFormat(&sampleRate, &speakerMode, &channels));
        SamplingFrequency = static_cast<float>(sampleRate);
        FFTHistoryMaxSize = static_cast<int>(SamplingFrequency / WindowSize);
        EngineDebug::GetInstance().PrintInfo("Sampling frequency: " + std::to_string(SamplingFrequency) +
                                             ", FFThistoryMaxSize: " + std::to_string(FFTHistoryMaxSize));

        int bandSize = SamplingFrequency / WindowSize;

        BeatDetectorBandLimits.clear();
        FFTHistoryBeatDetector.clear();

        // *CHECK* -> semi good.

        /* BeatDetectorBandLimits.push_back(60 / bandSize);
         BeatDetectorBandLimits.push_back(400 / bandSize);*/


        // BeatDetectorBandLimits.push_back(300 / bandSize);
        // BeatDetectorBandLimits.push_back(750 / bandSize);
    }
}

void AudioSource::Stop(const std::string &eventName) {
    for (auto it = activeEvents.begin(); it != activeEvents.end();) {
        FMOD::Studio::EventInstance *instance = *it;
        if (instance) {
            FMOD::Studio::EventDescription *desc = nullptr;
            instance->getDescription(&desc);

            if (desc) {
                char path[512];
                int retrieved = 0;
                desc->getPath(path, sizeof(path), &retrieved);

                if (eventName == path) {
                    instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
                    instance->release();
                    it = activeEvents.erase(it);
                    continue;
                }
            }
        }
        ++it;
    }
}

void AudioSource::PlayAll() {
    if (activeEvents.size() == 0 || isAllPaused) {
        isAllPaused = false;
        StopAll();
        for (const auto &eventPath: eventNames) {
            Play(eventPath);
        }
    } else {
        EngineDebug::GetInstance().PrintInfo("Cannot play all sounds, there is something that is currently playing");
    }
}

void AudioSource::ResumeAll() {
    if (activeEvents.size() > 0) {
        if (affectedByDSP) {
            ref.ERRCHECK(ChannelGroup->setPaused(false));
        }
        isAllPaused = false;
        for (auto *eventInstance: activeEvents) {
            if (eventInstance) {
                ref.ERRCHECK(eventInstance->setPaused(false));
            }
        }

        ref.fmodSystem->update();
    }
}

void AudioSource::PauseAll() {
    if (activeEvents.size() > 0) {
        if (affectedByDSP) {
            ref.ERRCHECK(ChannelGroup->setPaused(true));
        }
        isAllPaused = true;
        for (auto *eventInstance: activeEvents) {
            if (eventInstance) {
                ref.ERRCHECK(eventInstance->setPaused(true));
            }
        }

        ref.fmodSystem->update();
    }
}

void AudioSource::StopAll() {
    if (activeEvents.empty()) {
        return;
    }

    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD_STUDIO_PLAYBACK_STATE state;
            ref.ERRCHECK(instance->getPlaybackState(&state));
            if (state == FMOD_STUDIO_PLAYBACK_PLAYING || state == FMOD_STUDIO_PLAYBACK_STARTING) {
                ref.ERRCHECK(instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
            }
            ref.ERRCHECK(instance->release());
        }
    }
    activeEvents.clear();

    if (affectedByDSP && ChannelGroup) {
        // ref.ERRCHECK(ChannelGroup->removeDSP(ref.fmodDSP));
        // ref.ERRCHECK(ChannelGroup->release());
        // ChannelGroup = nullptr;
    }

    ref.fmodSystem->update();
}

void AudioSource::Update(float deltaTime) {
    pos = this->gameObject->transform.GetLocalPosition();
    SetPosition(pos); // aktualizacja pozycji w 3D

    // ref.playerHealth -= 0.001f;
    std::clamp(ref.playerHealth, 0.0f, 1.0f);


    ref.ERRCHECK(ref.fmodSystem->setParameterByName("PlayerHealth", ref.playerHealth));
    ref.ERRCHECK(ref.fmodSystem->setParameterByName("BegginingTransition1", 1.0));

    ref.fmodSystem->update();

    if (affectedByDSP) {
        FMOD_DSP_PARAMETER_FFT *dspFFT = nullptr;
        Ref::ERRCHECK(ref.fmodDSP->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void **) &dspFFT, 0, 0, 0));

        if (dspFFT) {
            static int frameCounter = 0;
            const int bandUpdateInterval = 100;

            if (frameCounter % bandUpdateInterval == 0) {
                auto dynamicBands = computeBandLimits(dspFFT, SamplingFrequency, WindowSize, 4);
                BeatDetectorBandLimits.clear();
                for (const auto &band: dynamicBands) {
                    BeatDetectorBandLimits.push_back(band.first);
                    BeatDetectorBandLimits.push_back(band.second);
                }
            }
            frameCounter++;

            const size_t numBands = BeatDetectorBandLimits.size() / 2;
            std::vector<float> spectrum(numBands, 0.0f);
            std::vector<float> averageSpectrum(numBands, 0.0f);

            bool isBass = false;
            bool isLowM = false;
            bool isMidM = false;
            bool isHighM = false;

            if (dspFFT && dspFFT->length > 0 && !BeatDetectorBandLimits.empty()) {
                int length = dspFFT->length / 2;
                int numChannels = dspFFT->numchannels;

                for (size_t numBand = 0; numBand < numBands; ++numBand) {
                    size_t bandBoundIndex = numBand * 2;
                    int startBin = BeatDetectorBandLimits[bandBoundIndex];
                    int endBin = BeatDetectorBandLimits[bandBoundIndex + 1];
                    if (startBin >= endBin || endBin > length)
                        continue;

                    for (int indexFFT = startBin; indexFFT < endBin; ++indexFFT) {
                        for (int channel = 0; channel < numChannels; ++channel) {
                            spectrum[numBand] += dspFFT->spectrum[channel][indexFFT];
                        }
                    }
                    spectrum[numBand] /= (endBin - startBin) * numChannels;
                }

                if (!FFTHistoryBeatDetector.empty()) {
                    fillAverageSpectrum(averageSpectrum, FFTHistoryBeatDetector);

                    std::vector<float> varianceSpectrum(numBands, 0.0f);
                    fillVarianceSpectrum(varianceSpectrum, FFTHistoryBeatDetector, averageSpectrum);

                    isBass = numBands > 0 &&
                             (spectrum[0] - 0.05f) > beatThreshold(varianceSpectrum[0]) * averageSpectrum[0];
                    isLowM = numBands > 1 &&
                             (spectrum[1] - 0.0001f) > beatThreshold(varianceSpectrum[1]) * averageSpectrum[1];
                    isMidM = numBands > 2 &&
                             (spectrum[2] - 0.0001f) > beatThreshold(varianceSpectrum[2]) * averageSpectrum[2];
                    isHighM = numBands > 3 &&
                              (spectrum[3] - 0.0001f) > beatThreshold(varianceSpectrum[3]) * averageSpectrum[3];
                }

                if (FFTHistoryBeatDetector.size() >= FFTHistoryMaxSize) {
                    FFTHistoryBeatDetector.pop_front();
                }
                FFTHistoryBeatDetector.push_back(spectrum);
            }

            if ((isBass || isLowM || isMidM || isHighM) && !wasBeatRecently) {
                wasBeatRecently = true;
                Signals::Timer_OnTimeout.emit("onBeat");
            } else {
                wasBeatRecently = false;
            }
        }
    }
}

void AudioSource::SetPosition(const glm::vec3 &position) {
    pos = position;

    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD_3D_ATTRIBUTES attributes = {};
            attributes.position = {pos.x, pos.y, pos.z};
            attributes.forward = {0.0f, 0.0f, 1.0f};
            attributes.up = {0.0f, 1.0f, 0.0f};
            attributes.velocity = {0.0f, 0.0f, 0.0f};

            instance->set3DAttributes(&attributes);
        }
    }
}


/// <summary>
/// sets volume to given event
/// </summary>
/// <param name="eventName"></param>
/// <param name="volume"></param>
void AudioSource::SetVolume(const std::string &eventName, float volume) {
    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD::Studio::EventDescription *desc = nullptr;
            instance->getDescription(&desc);
            if (desc) {
                char path[512];
                int retrieved = 0;
                desc->getPath(path, sizeof(path), &retrieved);

                if (eventName == path) {
                    instance->setVolume(volume);
                }
            }
        }
    }
    if (affectedByDSP) {
        ref.ERRCHECK(ChannelGroup->setPitch(volume));
    }
}

/// <summary>
/// sets pitch to given event
/// </summary>
/// <param name="eventName"></param>
/// <param name="pitch"></param>
void AudioSource::SetPitch(const std::string &eventName, float pitch) {
    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD::Studio::EventDescription *desc = nullptr;
            instance->getDescription(&desc);
            if (desc) {
                char path[512];
                int retrieved = 0;
                desc->getPath(path, sizeof(path), &retrieved);

                if (eventName == path) {
                    instance->setPitch(pitch);
                }
            }
        }
    }
    if (affectedByDSP) {
        ref.ERRCHECK(ChannelGroup->setPitch(pitch));
    }
}

/// <summary>
/// sets volume to all played events
/// </summary>
/// <param name="volume"></param>
void AudioSource::SetVolumeAll(float volume) {
    for (auto *instance: activeEvents) {
        if (instance) {
            ref.ERRCHECK(instance->setVolume(volume));
        }
    }
    if (affectedByDSP) {
        ref.ERRCHECK(ChannelGroup->setVolume(volume));
    }
    ref.fmodSystem->update();
}

/// <summary>
/// Sets pitch to all played events
/// </summary>
/// <param name="pitch"></param>
void AudioSource::SetPitchAll(float pitch) {
    for (auto *instance: activeEvents) {
        if (instance) {
            ref.ERRCHECK(instance->setPitch(pitch));
        }
    }
    if (affectedByDSP) {
        ref.ERRCHECK(ChannelGroup->setPitch(pitch));
    }
    ref.fmodSystem->update();
}

/// <summary>
/// returns group volume (when we are sure, that volume is the same for all events)
/// </summary>
/// <returns></returns>
float AudioSource::GetVolumeFromAll() {
    for (auto *instance: activeEvents) {
        if (instance) {

            float volume = 0.0f;
            float finalVolume = 0.0f; // not needed but necessary - final value considering all modulations etc.
            ref.ERRCHECK(instance->getVolume(&volume, &finalVolume));

            return volume;
        }
    };
}

/// <summary>
/// returns group pitch (when we are sure, that pitch is the same for all events)
/// </summary>
/// <returns></returns>
float AudioSource::GetPitchFromAll() {

    for (auto *instance: activeEvents) {
        if (instance) {

            float pitch = 0.0f;
            float finalPitch = 0.0f; // not needed but necessary - final value considering all modulations etc.
            ref.ERRCHECK(instance->getPitch(&pitch, &finalPitch));

            return pitch;
        }
    }
}

void AudioSource::SetParameter(const std::string &eventName, const std::string &paramName, float value) {
    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD::Studio::EventDescription *desc = nullptr;
            instance->getDescription(&desc);
            if (desc) {
                char path[512];
                int retrieved = 0;
                desc->getPath(path, sizeof(path), &retrieved);

                if (eventName == path) {
                    ref.ERRCHECK(instance->setParameterByName(paramName.c_str(), value));
                }
            }
        }
    }
    if (affectedByDSP) {
    }
    ref.fmodSystem->update();
}

std::string AudioSource::GetParameterList(const std::string &eventName) {
    std::string output;
    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD::Studio::EventDescription *desc = nullptr;
            instance->getDescription(&desc);
            if (desc) {
                char path[512];
                int retrieved = 0;
                desc->getPath(path, sizeof(path), &retrieved);
                if (eventName == path) {
                    int paramCount = 0;
                    if (desc->getParameterDescriptionCount(&paramCount) == FMOD_OK) {
                        for (int i = 0; i < paramCount; ++i) {
                            FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                            if (desc->getParameterDescriptionByIndex(i, &paramDesc) == FMOD_OK) {
                                // Get the current parameter value
                                float value = 0.0f;
                                instance->getParameterByID(paramDesc.id, &value);

                                // Add parameter name and value to output
                                output += paramDesc.name;
                                output += ": ";
                                output += std::to_string(value);
                                output += "\n";
                            }
                        }
                    }
                }
            }
        }
    }
    return output;
}

std::string AudioSource::GetActiveEventList() {
    std::string eventList;

    for (auto *instance: activeEvents) {
        if (instance) {
            FMOD::Studio::EventDescription *desc = nullptr;
            instance->getDescription(&desc);
            if (desc) {
                char path[512];
                int retrieved = 0;
                if (desc->getPath(path, sizeof(path), &retrieved) == FMOD_OK) {
                    eventList += std::string(path) + "\n";
                }
            }
        }
    }

    return eventList;
}

std::string AudioSource::GetBankEventList() {
    std::string output;

    int eventCount = 0;
    bank->getEventCount(&eventCount);
    std::vector<FMOD::Studio::EventDescription *> eventDescriptions(eventCount);
    bank->getEventList(eventDescriptions.data(), eventCount, &eventCount);

    for (int i = 0; i < eventCount; ++i) {
        char path[512];
        int retrieved = 0;
        eventDescriptions[i]->getPath(path, sizeof(path), &retrieved);
        output += path;
        output += "\n";
    }

    return output;
}

glm::vec3 AudioSource::GetPos() { return this->pos; }

void AudioSource::SetPos(glm::vec3 pos) { this->pos = pos; }

std::string AudioSource::GetBankPath() { return bankPath; }


void AudioSource::fillAverageSpectrum(std::vector<float> &averageSpectrum, const FFTHistoryContainer &fftHistory) {
    for (const auto &fftResult: fftHistory) {
        for (size_t index = 0; index < fftResult.size() && index < averageSpectrum.size(); ++index) {
            averageSpectrum[index] += fftResult[index];
        }
    }

    for (float &value: averageSpectrum) {
        value /= fftHistory.size();
    }
}

void AudioSource::fillVarianceSpectrum(std::vector<float> &varianceSpectrum, const FFTHistoryContainer &fftHistory,
                                       const std::vector<float> &averageSpectrum) {
    for (const auto &fftResult: fftHistory) {
        for (size_t index = 0;
             index < fftResult.size() && index < varianceSpectrum.size() && index < averageSpectrum.size(); ++index) {
            varianceSpectrum[index] +=
                    (fftResult[index] - averageSpectrum[index]) * (fftResult[index] - averageSpectrum[index]);
        }
    }

    for (float &value: varianceSpectrum) {
        value /= fftHistory.size();
    }
}

float AudioSource::beatThreshold(float variance) { return -15 * variance + 1.55; }

FMOD_RESULT F_CALL AudioSource::programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
                                                        FMOD_STUDIO_EVENTINSTANCE *event, void *parameters) {
    if (type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_STOPPED) {
        for (auto it = activeEvents.begin(); it != activeEvents.end();) {
            FMOD::Studio::EventInstance *instance = *it;
            if (instance) {
                FMOD_STUDIO_PLAYBACK_STATE state;
                instance->getPlaybackState(&state);

                if (state == FMOD_STUDIO_PLAYBACK_STOPPED) {
                    instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
                    instance->release();
                    it = activeEvents.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
    return FMOD_OK;
}

std::vector<std::pair<int, int>> AudioSource::computeBandLimits(const FMOD_DSP_PARAMETER_FFT *dspFFT,
                                                                float samplingFrequency, int windowSize,
                                                                int numDesiredBands) {
    std::vector<std::pair<int, int>> bandLimits;
    if (!dspFFT || dspFFT->length == 0)
        return bandLimits;

    if (bandLimits.empty()) {
        bandLimits.emplace_back(60 / (samplingFrequency / windowSize),
                                130 / (samplingFrequency / windowSize)); // Default kick/bass
        bandLimits.emplace_back(9000 / (samplingFrequency / windowSize),
                                11000 / (samplingFrequency / windowSize)); // Default piano
        bandLimits.emplace_back(140 / (samplingFrequency / windowSize), 600 / (samplingFrequency / windowSize));
        bandLimits.emplace_back(2500 / (samplingFrequency / windowSize), 8500 / (samplingFrequency / windowSize));
    }

    return bandLimits;
}
