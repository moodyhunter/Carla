﻿/*
 * Carla FluidSynth Plugin
 * Copyright (C) 2011-2013 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the GPL.txt file
 */

#include "CarlaPluginInternal.hpp"

#ifdef WANT_FLUIDSYNTH

#include <fluidsynth.h>

#define FLUIDSYNTH_VERSION_NEW_API (FLUIDSYNTH_VERSION_MAJOR >= 1 && FLUIDSYNTH_VERSION_MINOR >= 1 && FLUIDSYNTH_VERSION_MICRO >= 4)

CARLA_BACKEND_START_NAMESPACE

class FluidSynthPlugin : public CarlaPlugin
{
public:
    FluidSynthPlugin(CarlaEngine* const engine, const unsigned int id)
        : CarlaPlugin(engine, id)
    {
        carla_debug("FluidSynthPlugin::FluidSynthPlugin()");

        // create settings
        fSettings = new_fluid_settings();

        // define settings
        fluid_settings_setnum(fSettings, "synth.sample-rate", kData->engine->getSampleRate());
        fluid_settings_setint(fSettings, "synth.threadsafe-api ", 0);

        // create synth
        fSynth = new_fluid_synth(fSettings);

#ifdef FLUIDSYNTH_VERSION_NEW_API
        fluid_synth_set_sample_rate(fSynth, kData->engine->getSampleRate());
#endif

        // set default values
        fluid_synth_set_reverb_on(fSynth, 0);
        fluid_synth_set_reverb(fSynth, FLUID_REVERB_DEFAULT_ROOMSIZE, FLUID_REVERB_DEFAULT_DAMP, FLUID_REVERB_DEFAULT_WIDTH, FLUID_REVERB_DEFAULT_LEVEL);

        fluid_synth_set_chorus_on(fSynth, 0);
        fluid_synth_set_chorus(fSynth, FLUID_CHORUS_DEFAULT_N, FLUID_CHORUS_DEFAULT_LEVEL, FLUID_CHORUS_DEFAULT_SPEED, FLUID_CHORUS_DEFAULT_DEPTH, FLUID_CHORUS_DEFAULT_TYPE);

        fluid_synth_set_polyphony(fSynth, 64);

        for (int i=0; i < 16; i++)
            fluid_synth_set_interp_method(fSynth, i, FLUID_INTERP_DEFAULT);
    }

    ~FluidSynthPlugin()
    {
        carla_debug("FluidSynthPlugin::~FluidSynthPlugin()");

        delete_fluid_synth(fSynth);
        delete_fluid_settings(fSettings);
    }

    // -------------------------------------------------------------------
    // Information (base)

    PluginType type() const
    {
        return PLUGIN_SF2;
    }

    PluginCategory category() const
    {
        return PLUGIN_CATEGORY_SYNTH;
    }

    // -------------------------------------------------------------------
    // Information (count)

    uint32_t parameterScalePointCount(const uint32_t parameterId) const
    {
        CARLA_ASSERT(parameterId < kData->param.count);

        switch (parameterId)
        {
        case FluidSynthChorusType:
            return 2;
        case FluidSynthInterpolation:
            return 4;
        default:
            return 0;
        }
    }

    // -------------------------------------------------------------------
    // Information (per-plugin data)

    float getParameterValue(const uint32_t parameterId)
    {
        CARLA_ASSERT(parameterId < kData->param.count);

        return fParamBuffers[parameterId];
    }

    float getParameterScalePointValue(const uint32_t parameterId, const uint32_t scalePointId)
    {
        CARLA_ASSERT(parameterId < kData->param.count);
        CARLA_ASSERT(scalePointId < parameterScalePointCount(parameterId));

        switch (parameterId)
        {
        case FluidSynthChorusType:
            switch (scalePointId)
            {
            case 0:
                return FLUID_CHORUS_MOD_SINE;
            case 1:
                return FLUID_CHORUS_MOD_TRIANGLE;
            default:
                return FLUID_CHORUS_DEFAULT_TYPE;
            }
        case FluidSynthInterpolation:
            switch (scalePointId)
            {
            case 0:
                return FLUID_INTERP_NONE;
            case 1:
                return FLUID_INTERP_LINEAR;
            case 2:
                return FLUID_INTERP_4THORDER;
            case 3:
                return FLUID_INTERP_7THORDER;
            default:
                return FLUID_INTERP_DEFAULT;
            }
        default:
            return 0.0f;
        }
    }

    void getLabel(char* const strBuf)
    {
        if (fLabel.isNotEmpty())
            std::strncpy(strBuf, (const char*)fLabel, STR_MAX);
        else
            CarlaPlugin::getLabel(strBuf);
    }

    void getMaker(char* const strBuf)
    {
        std::strncpy(strBuf, "FluidSynth SF2 engine", STR_MAX);
    }

    void getCopyright(char* const strBuf)
    {
        std::strncpy(strBuf, "GNU GPL v2+", STR_MAX);
    }

    void getRealName(char* const strBuf)
    {
        getLabel(strBuf);
    }

    void getParameterName(const uint32_t parameterId, char* const strBuf)
    {
        CARLA_ASSERT(parameterId < kData->param.count);

        switch (parameterId)
        {
        case FluidSynthReverbOnOff:
            std::strncpy(strBuf, "Reverb On/Off", STR_MAX);
            break;
        case FluidSynthReverbRoomSize:
            std::strncpy(strBuf, "Reverb Room Size", STR_MAX);
            break;
        case FluidSynthReverbDamp:
            std::strncpy(strBuf, "Reverb Damp", STR_MAX);
            break;
        case FluidSynthReverbLevel:
            std::strncpy(strBuf, "Reverb Level", STR_MAX);
            break;
        case FluidSynthReverbWidth:
            std::strncpy(strBuf, "Reverb Width", STR_MAX);
            break;
        case FluidSynthChorusOnOff:
            std::strncpy(strBuf, "Chorus On/Off", STR_MAX);
            break;
        case FluidSynthChorusNr:
            std::strncpy(strBuf, "Chorus Voice Count", STR_MAX);
            break;
        case FluidSynthChorusLevel:
            std::strncpy(strBuf, "Chorus Level", STR_MAX);
            break;
        case FluidSynthChorusSpeedHz:
            std::strncpy(strBuf, "Chorus Speed", STR_MAX);
            break;
        case FluidSynthChorusDepthMs:
            std::strncpy(strBuf, "Chorus Depth", STR_MAX);
            break;
        case FluidSynthChorusType:
            std::strncpy(strBuf, "Chorus Type", STR_MAX);
            break;
        case FluidSynthPolyphony:
            std::strncpy(strBuf, "Polyphony", STR_MAX);
            break;
        case FluidSynthInterpolation:
            std::strncpy(strBuf, "Interpolation", STR_MAX);
            break;
        case FluidSynthVoiceCount:
            std::strncpy(strBuf, "Voice Count", STR_MAX);
            break;
        default:
            CarlaPlugin::getParameterName(parameterId, strBuf);
            break;
        }
    }

    void getParameterUnit(const uint32_t parameterId, char* const strBuf)
    {
        CARLA_ASSERT(parameterId < kData->param.count);

        switch (parameterId)
        {
        case FluidSynthChorusSpeedHz:
            std::strncpy(strBuf, "Hz", STR_MAX);
            break;
        case FluidSynthChorusDepthMs:
            std::strncpy(strBuf, "ms", STR_MAX);
            break;
        default:
            CarlaPlugin::getParameterUnit(parameterId, strBuf);
            break;
        }
    }

    void getParameterScalePointLabel(const uint32_t parameterId, const uint32_t scalePointId, char* const strBuf)
    {
        CARLA_ASSERT(parameterId < kData->param.count);
        CARLA_ASSERT(scalePointId < parameterScalePointCount(parameterId));

        switch (parameterId)
        {
        case FluidSynthChorusType:
            switch (scalePointId)
            {
            case 0:
                std::strncpy(strBuf, "Sine wave", STR_MAX);
                return;
            case 1:
                std::strncpy(strBuf, "Triangle wave", STR_MAX);
                return;
            }
        case FluidSynthInterpolation:
            switch (scalePointId)
            {
            case 0:
                std::strncpy(strBuf, "None", STR_MAX);
                return;
            case 1:
                std::strncpy(strBuf, "Straight-line", STR_MAX);
                return;
            case 2:
                std::strncpy(strBuf, "Fourth-order", STR_MAX);
                return;
            case 3:
                std::strncpy(strBuf, "Seventh-order", STR_MAX);
                return;
            }
        }

        CarlaPlugin::getParameterScalePointLabel(parameterId, scalePointId, strBuf);
    }

    // -------------------------------------------------------------------
    // Set data (plugin-specific stuff)

    void setParameterValue(const uint32_t parameterId, const float value, const bool sendGui, const bool sendOsc, const bool sendCallback)
    {
        CARLA_ASSERT(parameterId < kData->param.count);

        const float fixedValue = kData->param.fixValue(parameterId, value);
        fParamBuffers[parameterId] = fixedValue;

        switch (parameterId)
        {
        case FluidSynthReverbOnOff:
            fluid_synth_set_reverb_on(fSynth, (fixedValue > 0.5f) ? 1 : 0);
            break;

        case FluidSynthReverbRoomSize:
        case FluidSynthReverbDamp:
        case FluidSynthReverbLevel:
        case FluidSynthReverbWidth:
            fluid_synth_set_reverb(fSynth, fParamBuffers[FluidSynthReverbRoomSize], fParamBuffers[FluidSynthReverbDamp], fParamBuffers[FluidSynthReverbWidth], fParamBuffers[FluidSynthReverbLevel]);
            break;

        case FluidSynthChorusOnOff:
        {
            // FIXME
            //const ScopedDisabler m(this, ! kData->engine->isOffline());
            fluid_synth_set_chorus_on(fSynth, (value > 0.5f) ? 1 : 0);
            break;
        }

        case FluidSynthChorusNr:
        case FluidSynthChorusLevel:
        case FluidSynthChorusSpeedHz:
        case FluidSynthChorusDepthMs:
        case FluidSynthChorusType:
        {
            //FIXME
            //const ScopedDisabler m(this, ! kData->engine->isOffline());
            fluid_synth_set_chorus(fSynth, fParamBuffers[FluidSynthChorusNr], fParamBuffers[FluidSynthChorusLevel], fParamBuffers[FluidSynthChorusSpeedHz], fParamBuffers[FluidSynthChorusDepthMs], fParamBuffers[FluidSynthChorusType]);
            break;
        }

        case FluidSynthPolyphony:
        {
            //FIXME
            //const ScopedDisabler m(this, ! kData->engine->isOffline());
            fluid_synth_set_polyphony(fSynth, value);
            break;
        }

        case FluidSynthInterpolation:
        {
            //FIXME
            //const ScopedDisabler m(this, ! kData->engine->isOffline());

            for (int i=0; i < 16; i++)
                fluid_synth_set_interp_method(fSynth, i, value);

            break;
        }

        default:
            break;
        }

        CarlaPlugin::setParameterValue(parameterId, value, sendGui, sendOsc, sendCallback);
    }

    void setMidiProgram(int32_t index, const bool sendGui, const bool sendOsc, const bool sendCallback, const bool block)
    {
        CARLA_ASSERT(index >= -1 && index < static_cast<int32_t>(kData->midiprog.count));

        if (index < -1)
            index = -1;
        else if (index > static_cast<int32_t>(kData->midiprog.count))
            return;

        if (kData->ctrlInChannel < 0 || kData->ctrlInChannel >= 16)
            return;

        // FIXME
        if (index >= 0)
        {
            const uint32_t bank    = kData->midiprog.data[index].bank;
            const uint32_t program = kData->midiprog.data[index].program;

            if (kData->engine->isOffline())
            {
                //const CarlaEngine::ScopedLocker m(x_engine, block);
                fluid_synth_program_select(fSynth, kData->ctrlInChannel, fSynthId, bank, program);
            }
            else
            {
                //const ScopedDisabler m(this, block);
                fluid_synth_program_select(fSynth, kData->ctrlInChannel, fSynthId, bank, program);
            }
        }

        CarlaPlugin::setMidiProgram(index, sendGui, sendOsc, sendCallback, block);
    }

    // -------------------------------------------------------------------
    // Plugin state

    void reload()
    {
        carla_debug("FluidSynthPlugin::reload() - start");
        CARLA_ASSERT(kData->engine != nullptr);
        CARLA_ASSERT(fSynth != nullptr);

        const ProcessMode processMode(kData->engine->getProccessMode());

        // Safely disable plugin for reload
        const ScopedDisabler sd(this);

        if (kData->client->isActive())
            kData->client->deactivate();

        deleteBuffers();

        uint32_t aOuts, params, j;
        aOuts  = 2;
        params = FluidSynthParametersMax;

        kData->audioOut.createNew(aOuts);
        kData->param.createNew(params);

        const int   portNameSize = kData->engine->maxPortNameSize();
        CarlaString portName;

        // ---------------------------------------
        // Audio Outputs

        {
            portName.clear();

            if (processMode == PROCESS_MODE_SINGLE_CLIENT)
            {
                portName  = fName;
                portName += ":";
            }

            portName += "out-left";
            portName.truncate(portNameSize);

            kData->audioOut.ports[0].port   = (CarlaEngineAudioPort*)kData->client->addPort(kEnginePortTypeAudio, portName, false);
            kData->audioOut.ports[0].rindex = 0;
        }

        {
            portName.clear();

            if (processMode == PROCESS_MODE_SINGLE_CLIENT)
            {
                portName  = fName;
                portName += ":";
            }

            portName += "out-right";
            portName.truncate(portNameSize);

            kData->audioOut.ports[1].port   = (CarlaEngineAudioPort*)kData->client->addPort(kEnginePortTypeAudio, portName, false);
            kData->audioOut.ports[1].rindex = 1;
        }

        // ---------------------------------------
        // Event Input

        {
            portName.clear();

            if (processMode == PROCESS_MODE_SINGLE_CLIENT)
            {
                portName  = fName;
                portName += ":";
            }

            portName += "event-in";
            portName.truncate(portNameSize);

            kData->event.portIn = (CarlaEngineEventPort*)kData->client->addPort(kEnginePortTypeEvent, portName, true);
        }

        // ---------------------------------------
        // Event Output

        {
            portName.clear();

            if (processMode == PROCESS_MODE_SINGLE_CLIENT)
            {
                portName  = fName;
                portName += ":";
            }

            portName += "event-out";
            portName.truncate(portNameSize);

            kData->event.portOut = (CarlaEngineEventPort*)kData->client->addPort(kEnginePortTypeEvent, portName, false);
        }

        // ----------------------
        j = FluidSynthReverbOnOff;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_AUTOMABLE | PARAMETER_IS_BOOLEAN;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 1.0f;
        kData->param.ranges[j].def = 0.0f; // off
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 1.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthReverbRoomSize;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_AUTOMABLE;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 1.2f;
        kData->param.ranges[j].def = FLUID_REVERB_DEFAULT_ROOMSIZE;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthReverbDamp;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_AUTOMABLE;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 1.0f;
        kData->param.ranges[j].def = FLUID_REVERB_DEFAULT_DAMP;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthReverbLevel;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_AUTOMABLE;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = MIDI_CONTROL_REVERB_SEND_LEVEL;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 1.0f;
        kData->param.ranges[j].def = FLUID_REVERB_DEFAULT_LEVEL;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthReverbWidth;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_AUTOMABLE;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 10.0f; // should be 100, but that sounds too much
        kData->param.ranges[j].def = FLUID_REVERB_DEFAULT_WIDTH;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthChorusOnOff;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_BOOLEAN;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 1.0f;
        kData->param.ranges[j].def = 0.0f; // off
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 1.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthChorusNr;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_INTEGER;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 99.0f;
        kData->param.ranges[j].def = FLUID_CHORUS_DEFAULT_N;
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 10.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthChorusLevel;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = 0; //MIDI_CONTROL_CHORUS_SEND_LEVEL;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 10.0f;
        kData->param.ranges[j].def = FLUID_CHORUS_DEFAULT_LEVEL;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthChorusSpeedHz;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.29f;
        kData->param.ranges[j].max = 5.0f;
        kData->param.ranges[j].def = FLUID_CHORUS_DEFAULT_SPEED;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthChorusDepthMs;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 2048000.0 / kData->engine->getSampleRate();
        kData->param.ranges[j].def = FLUID_CHORUS_DEFAULT_DEPTH;
        kData->param.ranges[j].step = 0.01f;
        kData->param.ranges[j].stepSmall = 0.0001f;
        kData->param.ranges[j].stepLarge = 0.1f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthChorusType;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_INTEGER | PARAMETER_USES_SCALEPOINTS;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = FLUID_CHORUS_MOD_SINE;
        kData->param.ranges[j].max = FLUID_CHORUS_MOD_TRIANGLE;
        kData->param.ranges[j].def = FLUID_CHORUS_DEFAULT_TYPE;
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 1.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthPolyphony;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_INTEGER;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 1.0f;
        kData->param.ranges[j].max = 512.0f; // max theoric is 65535
        kData->param.ranges[j].def = fluid_synth_get_polyphony(fSynth);
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 10.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthInterpolation;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_INPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_INTEGER | PARAMETER_USES_SCALEPOINTS;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = FLUID_INTERP_NONE;
        kData->param.ranges[j].max = FLUID_INTERP_HIGHEST;
        kData->param.ranges[j].def = FLUID_INTERP_DEFAULT;
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 1.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ----------------------
        j = FluidSynthVoiceCount;
        kData->param.data[j].index  = j;
        kData->param.data[j].rindex = j;
        kData->param.data[j].type   = PARAMETER_OUTPUT;
        kData->param.data[j].hints  = PARAMETER_IS_ENABLED | PARAMETER_IS_AUTOMABLE | PARAMETER_IS_INTEGER;
        kData->param.data[j].midiChannel = 0;
        kData->param.data[j].midiCC = -1;
        kData->param.ranges[j].min = 0.0f;
        kData->param.ranges[j].max = 65535.0f;
        kData->param.ranges[j].def = 0.0f;
        kData->param.ranges[j].step = 1.0f;
        kData->param.ranges[j].stepSmall = 1.0f;
        kData->param.ranges[j].stepLarge = 1.0f;
        fParamBuffers[j] = kData->param.ranges[j].def;

        // ---------------------------------------

        // plugin checks
        fHints &= ~(PLUGIN_IS_SYNTH | PLUGIN_USES_CHUNKS | PLUGIN_CAN_DRYWET | PLUGIN_CAN_VOLUME | PLUGIN_CAN_BALANCE | PLUGIN_CAN_FORCE_STEREO);

        fHints |= PLUGIN_IS_SYNTH;
        fHints |= PLUGIN_CAN_VOLUME;
        fHints |= PLUGIN_CAN_BALANCE;
        fHints |= PLUGIN_CAN_FORCE_STEREO;

        reloadPrograms(true);

        kData->client->activate();

        carla_debug("FluidSynthPlugin::reload() - end");
    }

    void reloadPrograms(const bool init)
    {
        carla_debug("FluidSynthPlugin::reloadPrograms(%s)", bool2str(init));

        // Delete old programs
        kData->midiprog.clear();

        // Query new programs
        uint32_t count = 0;
        fluid_sfont_t* f_sfont;
        fluid_preset_t f_preset;
        bool hasDrums = false;

        f_sfont = fluid_synth_get_sfont_by_id(fSynth, fSynthId);

        // initial check to know how much midi-programs we have
        f_sfont->iteration_start(f_sfont);
        while (f_sfont->iteration_next(f_sfont, &f_preset))
            count += 1;

        // soundfonts must always have at least 1 midi-program
        CARLA_ASSERT(count > 0);

        if (count == 0)
            return;

        kData->midiprog.createNew(count);

        // Update data
        uint32_t i = 0;
        f_sfont->iteration_start(f_sfont);

        while (f_sfont->iteration_next(f_sfont, &f_preset))
        {
            CARLA_ASSERT(i < kData->midiprog.count);
            kData->midiprog.data[i].bank    = f_preset.get_banknum(&f_preset);
            kData->midiprog.data[i].program = f_preset.get_num(&f_preset);
            kData->midiprog.data[i].name    = carla_strdup(f_preset.get_name(&f_preset));

            if (kData->midiprog.data[i].bank == 128)
                hasDrums = true;

            i++;
        }

        //f_sfont->free(f_sfont);

        // Update OSC Names
        if (kData->engine->isOscControlRegistered())
        {
            kData->engine->osc_send_control_set_midi_program_count(fId, kData->midiprog.count);

            for (i=0; i < kData->midiprog.count; i++)
                kData->engine->osc_send_control_set_midi_program_data(fId, i, kData->midiprog.data[i].bank, kData->midiprog.data[i].program, kData->midiprog.data[i].name);
        }

        if (init)
        {
            fluid_synth_program_reset(fSynth);

            // select first program, or 128 for ch10
            for (i=0; i < 16 && i != 9; i++)
            {
                fluid_synth_program_select(fSynth, i, fSynthId, kData->midiprog.data[0].bank, kData->midiprog.data[0].program);
#ifdef FLUIDSYNTH_VERSION_NEW_API
                fluid_synth_set_channel_type(fSynth, i, CHANNEL_TYPE_MELODIC);
#endif
            }

            if (hasDrums)
            {
                fluid_synth_program_select(fSynth, 9, fSynthId, 128, 0);
#ifdef FLUIDSYNTH_VERSION_NEW_API
                fluid_synth_set_channel_type(fSynth, 9, CHANNEL_TYPE_DRUM);
#endif
            }
            else
            {
                fluid_synth_program_select(fSynth, 9, fSynthId, kData->midiprog.data[0].bank, kData->midiprog.data[0].program);
#ifdef FLUIDSYNTH_VERSION_NEW_API
                fluid_synth_set_channel_type(fSynth, 9, CHANNEL_TYPE_MELODIC);
#endif
            }

            setMidiProgram(0, false, false, false, true);
        }
        else
        {
            kData->engine->callback(CALLBACK_RELOAD_PROGRAMS, fId, 0, 0, 0.0f, nullptr);
        }
    }

    // -------------------------------------------------------------------
    // Plugin processing

    void process(float** const, float** const outBuffer, const uint32_t frames, const uint32_t framesOffset)
    {
        uint32_t i, k;
        uint32_t midiEventCount = 0;

        // --------------------------------------------------------------------------------------------------------
        // Check if active

        if (! kData->active)
        {
            // disable any output sound
            for (i=0; i < kData->audioOut.count; i++)
                carla_zeroFloat(outBuffer[i], frames);

            kData->activeBefore = kData->active;
            return;
        }

        // --------------------------------------------------------------------------------------------------------
        // Check if active before

        if (! kData->activeBefore)
        {
            for (int c=0; c < MAX_MIDI_CHANNELS; c++)
            {
#ifdef FLUIDSYNTH_VERSION_NEW_API
                fluid_synth_all_notes_off(fSynth, c);
                fluid_synth_all_sounds_off(fSynth, c);
#else
                fluid_synth_cc(f_synth, c, MIDI_CONTROL_ALL_SOUND_OFF, 0);
                fluid_synth_cc(f_synth, c, MIDI_CONTROL_ALL_NOTES_OFF, 0);
#endif
            }
        }

        // --------------------------------------------------------------------------------------------------------
        // Event Input and Processing

        else
        {
            // ----------------------------------------------------------------------------------------------------
            // MIDI Input (External)

            if (kData->extNotes.mutex.tryLock())
            {
                while (! kData->extNotes.data.isEmpty())
                {
                    const ExternalMidiNote& note = kData->extNotes.data.getFirst(true);

                    CARLA_ASSERT(note.channel >= 0);

                    if (note.velo > 0)
                        fluid_synth_noteon(fSynth, note.channel, note.note, note.velo);
                    else
                        fluid_synth_noteoff(fSynth,note.channel, note.note);

                    midiEventCount += 1;
                }

                kData->extNotes.mutex.unlock();

            } // End of MIDI Input (External)

            // ----------------------------------------------------------------------------------------------------
            // Event Input (System)

            bool allNotesOffSent = false;

            uint32_t time, nEvents = kData->event.portIn->getEventCount();
            uint32_t timeOffset = 0;

            uint32_t nextBankIds[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 0, 0 };

            if (kData->midiprog.current >= 0 && kData->midiprog.count > 0 && kData->ctrlInChannel >= 0 && kData->ctrlInChannel < 16)
                nextBankIds[kData->ctrlInChannel] = kData->midiprog.data[kData->midiprog.current].bank;

            for (i=0; i < nEvents; i++)
            {
                const EngineEvent& event = kData->event.portIn->getEvent(i);

                time = event.time - framesOffset;

                if (time >= frames)
                    continue;

                CARLA_ASSERT_INT2(time >= timeOffset, time, timeOffset);

                if (time > timeOffset)
                {
                    fluid_synth_write_float(fSynth, time - timeOffset, outBuffer[0] + timeOffset, 0, 1, outBuffer[1] + timeOffset, 0, 1);
                    timeOffset = time;
                }

                // Control change
                switch (event.type)
                {
                case kEngineEventTypeNull:
                    break;

                case kEngineEventTypeControl:
                {
                    const EngineControlEvent& ctrlEvent = event.ctrl;

                    switch (ctrlEvent.type)
                    {
                    case kEngineControlEventTypeNull:
                        break;

                    case kEngineControlEventTypeParameter:
                    {
                        // Control backend stuff
                        if (event.channel == kData->ctrlInChannel)
                        {
                            double value;

                            if (MIDI_IS_CONTROL_BREATH_CONTROLLER(ctrlEvent.param) && (fHints & PLUGIN_CAN_DRYWET) > 0)
                            {
                                value = ctrlEvent.value;
                                setDryWet(value, false, false);
                                postponeRtEvent(kPluginPostRtEventParameterChange, PARAMETER_DRYWET, 0, value);
                                continue;
                            }

                            if (MIDI_IS_CONTROL_CHANNEL_VOLUME(ctrlEvent.param) && (fHints & PLUGIN_CAN_VOLUME) > 0)
                            {
                                value = ctrlEvent.value*127/100;
                                setVolume(value, false, false);
                                postponeRtEvent(kPluginPostRtEventParameterChange, PARAMETER_VOLUME, 0, value);
                                continue;
                            }

                            if (MIDI_IS_CONTROL_BALANCE(ctrlEvent.param) && (fHints & PLUGIN_CAN_BALANCE) > 0)
                            {
                                double left, right;
                                value = ctrlEvent.value/0.5 - 1.0;

                                if (value < 0.0)
                                {
                                    left  = -1.0;
                                    right = (value*2)+1.0;
                                }
                                else if (value > 0.0)
                                {
                                    left  = (value*2)-1.0;
                                    right = 1.0;
                                }
                                else
                                {
                                    left  = -1.0;
                                    right = 1.0;
                                }

                                setBalanceLeft(left, false, false);
                                setBalanceRight(right, false, false);
                                postponeRtEvent(kPluginPostRtEventParameterChange, PARAMETER_BALANCE_LEFT, 0, left);
                                postponeRtEvent(kPluginPostRtEventParameterChange, PARAMETER_BALANCE_RIGHT, 0, right);
                                continue;
                            }
                        }

                        // Control plugin parameters
                        for (k=0; k < kData->param.count; k++)
                        {
                            if (kData->param.data[k].midiChannel != event.channel)
                                continue;
                            if (kData->param.data[k].midiCC != ctrlEvent.param)
                                continue;
                            if (kData->param.data[k].type != PARAMETER_INPUT)
                                continue;
                            if ((kData->param.data[k].hints & PARAMETER_IS_AUTOMABLE) == 0)
                                continue;

                            double value;

                            if (kData->param.data[k].hints & PARAMETER_IS_BOOLEAN)
                            {
                                value = (ctrlEvent.value < 0.5) ? kData->param.ranges[k].min : kData->param.ranges[k].max;
                            }
                            else
                            {
                                // FIXME - ranges call for this
                                value = ctrlEvent.value * (kData->param.ranges[k].max - kData->param.ranges[k].min) + kData->param.ranges[k].min;

                                if (kData->param.data[k].hints & PARAMETER_IS_INTEGER)
                                    value = std::rint(value);
                            }

                            setParameterValue(k, value, false, false, false);
                            postponeRtEvent(kPluginPostRtEventParameterChange, k, 0, value);
                        }

                        break;
                    }

                    case kEngineControlEventTypeMidiBank:
                        if (event.channel < 16 && event.channel != 9) // FIXME
                            nextBankIds[event.channel] = ctrlEvent.param;
                        break;

                    case kEngineControlEventTypeMidiProgram:
                        if (event.channel < 16)
                        {
                            const uint32_t bankId = nextBankIds[event.channel];
                            const uint32_t progId = ctrlEvent.param;

                            for (k=0; k < kData->midiprog.count; k++)
                            {
                                if (kData->midiprog.data[k].bank == bankId && kData->midiprog.data[k].program == progId)
                                {
                                    if (event.channel == kData->ctrlInChannel)
                                    {
                                        setMidiProgram(k, false, false, false, false);
                                        postponeRtEvent(kPluginPostRtEventMidiProgramChange, k, 0, 0.0);
                                    }
                                    else
                                        fluid_synth_program_select(fSynth, event.channel, fSynthId, bankId, progId);

                                    break;
                                }
                            }
                        }
                        break;

                    case kEngineControlEventTypeAllSoundOff:
                        if (event.channel == kData->ctrlInChannel)
                        {
                            if (! allNotesOffSent)
                                sendMidiAllNotesOff();

                            postponeRtEvent(kPluginPostRtEventParameterChange, PARAMETER_ACTIVE, 0, 0.0);
                            postponeRtEvent(kPluginPostRtEventParameterChange, PARAMETER_ACTIVE, 0, 1.0);

                            allNotesOffSent = true;
                        }
                        break;

                    case kEngineControlEventTypeAllNotesOff:
                        if (event.channel == kData->ctrlInChannel)
                        {
                            if (! allNotesOffSent)
                                sendMidiAllNotesOff();

                            allNotesOffSent = true;
                        }
                        break;
                    }

                    break;
                }

                case kEngineEventTypeMidi:
                {
                    if (midiEventCount >= MAX_MIDI_EVENTS)
                        continue;

                    const EngineMidiEvent& midiEvent = event.midi;

                    uint8_t status  = MIDI_GET_STATUS_FROM_DATA(midiEvent.data);
                    uint8_t channel = MIDI_GET_CHANNEL_FROM_DATA(midiEvent.data);

                    // Fix bad note-off
                    if (MIDI_IS_STATUS_NOTE_ON(status) && midiEvent.data[2] == 0)
                        status -= 0x10;

                    if (MIDI_IS_STATUS_NOTE_OFF(status))
                    {
                        const uint8_t note = midiEvent.data[1];

                        fluid_synth_noteoff(fSynth, channel, note);

                        postponeRtEvent(kPluginPostRtEventNoteOff, channel, note, 0.0);
                    }
                    else if (MIDI_IS_STATUS_NOTE_ON(status))
                    {
                        const uint8_t note = midiEvent.data[1];
                        const uint8_t velo = midiEvent.data[2];

                        fluid_synth_noteon(fSynth, channel, note, velo);

                        postponeRtEvent(kPluginPostRtEventNoteOn, channel, note, velo);
                    }
                    else if (MIDI_IS_STATUS_POLYPHONIC_AFTERTOUCH(status))
                    {
                        const uint8_t note     = midiEvent.data[1];
                        const uint8_t pressure = midiEvent.data[2];

                        // TODO, not in fluidsynth API?
                        Q_UNUSED(note);
                        Q_UNUSED(pressure);
                    }
                    else if (MIDI_IS_STATUS_AFTERTOUCH(status))
                    {
                        const uint8_t pressure = midiEvent.data[1];

                        fluid_synth_channel_pressure(fSynth, channel, pressure);;
                    }
                    else if (MIDI_IS_STATUS_PITCH_WHEEL_CONTROL(status))
                    {
                        const uint8_t lsb = midiEvent.data[1];
                        const uint8_t msb = midiEvent.data[2];

                        fluid_synth_pitch_bend(fSynth, channel, (msb << 7) | lsb);
                    }
                    else
                        continue;

                    midiEventCount += 1;

                    break;
                }
                }
            }

            kData->postRtEvents.trySplice();

            if (frames > timeOffset)
                fluid_synth_write_float(fSynth, frames - timeOffset, outBuffer[0] + timeOffset, 0, 1, outBuffer[1] + timeOffset, 0, 1);

        } // End of Event Input and Processing

        CARLA_PROCESS_CONTINUE_CHECK;

        // --------------------------------------------------------------------------------------------------------
        // Post-processing (volume and balance)

        {
            const bool doVolume  = (fHints & PLUGIN_CAN_VOLUME) > 0 && kData->postProc.volume != 1.0f;
            const bool doBalance = (fHints & PLUGIN_CAN_BALANCE) > 0 && (kData->postProc.balanceLeft != -1.0f || kData->postProc.balanceRight != 1.0f);

            float oldBufLeft[doBalance ? frames : 1];

            for (i=0; i < kData->audioOut.count; i++)
            {
                // Balance
                if (doBalance)
                {
                    if (i % 2 == 0)
                        std::memcpy(oldBufLeft, outBuffer[i], sizeof(float)*frames);

                    float balRangeL = (kData->postProc.balanceLeft  + 1.0f)/2.0f;
                    float balRangeR = (kData->postProc.balanceRight + 1.0f)/2.0f;

                    for (k=0; k < frames; k++)
                    {
                        if (i % 2 == 0)
                        {
                            // left
                            outBuffer[i][k]  = oldBufLeft[k]     * (1.0f - balRangeL);
                            outBuffer[i][k] += outBuffer[i+1][k] * (1.0f - balRangeR);
                        }
                        else
                        {
                            // right
                            outBuffer[i][k]  = outBuffer[i][k] * balRangeR;
                            outBuffer[i][k] += oldBufLeft[k]   * balRangeL;
                        }
                    }
                }

                // Volume
                if (doVolume)
                {
                    for (k=0; k < frames; k++)
                        outBuffer[i][k] *= kData->postProc.volume;
                }
            }

        } // End of Post-processing

        CARLA_PROCESS_CONTINUE_CHECK;

        // --------------------------------------------------------------------------------------------------------
        // Control Output

        {
            k = FluidSynthVoiceCount;
            fParamBuffers[k] = fluid_synth_get_active_voice_count(fSynth);
            kData->param.ranges[k].fixValue(fParamBuffers[k]);

            if (kData->param.data[k].midiCC > 0)
            {
                double value = kData->param.ranges[k].normalizeValue(fParamBuffers[k]);
                kData->event.portOut->writeControlEvent(framesOffset, kData->param.data[k].midiChannel, kEngineControlEventTypeParameter, kData->param.data[k].midiCC, value);
            }

        } // End of Control Output

        // --------------------------------------------------------------------------------------------------------

        kData->activeBefore = kData->active;
    }

    // -------------------------------------------------------------------

    bool init(const char* const filename, const char* const name, const char* const label)
    {
        CARLA_ASSERT(fSynth != nullptr);
        CARLA_ASSERT(filename != nullptr);
        CARLA_ASSERT(label != nullptr);

        // ---------------------------------------------------------------
        // open soundfont

        fSynthId = fluid_synth_sfload(fSynth, filename, 0);

        if (fSynthId < 0)
        {
            kData->engine->setLastError("Failed to load SoundFont file");
            return false;
        }

        // ---------------------------------------------------------------
        // get info

        fFilename = filename;
        fLabel    = label;

        if (name != nullptr)
            fName = kData->engine->getNewUniquePluginName(name);
        else
            fName = kData->engine->getNewUniquePluginName(label);

        // ---------------------------------------------------------------
        // register client

        kData->client = kData->engine->addClient(this);

        if (kData->client == nullptr || ! kData->client->isOk())
        {
            kData->engine->setLastError("Failed to register plugin client");
            return false;
        }

        return true;
    }

private:
    enum FluidSynthInputParameters {
        FluidSynthReverbOnOff    = 0,
        FluidSynthReverbRoomSize = 1,
        FluidSynthReverbDamp     = 2,
        FluidSynthReverbLevel    = 3,
        FluidSynthReverbWidth    = 4,
        FluidSynthChorusOnOff    = 5,
        FluidSynthChorusNr       = 6,
        FluidSynthChorusLevel    = 7,
        FluidSynthChorusSpeedHz  = 8,
        FluidSynthChorusDepthMs  = 9,
        FluidSynthChorusType     = 10,
        FluidSynthPolyphony      = 11,
        FluidSynthInterpolation  = 12,
        FluidSynthVoiceCount     = 13,
        FluidSynthParametersMax  = 14
    };

    CarlaString fLabel;

    fluid_settings_t* fSettings;
    fluid_synth_t* fSynth;
    int fSynthId;

    double fParamBuffers[FluidSynthParametersMax];
};

/**@}*/

CARLA_BACKEND_END_NAMESPACE

#else // WANT_FLUIDSYNTH
# warning fluidsynth not available (no SF2 support)
#endif

CARLA_BACKEND_START_NAMESPACE

CarlaPlugin* CarlaPlugin::newSF2(const Initializer& init)
{
    carla_debug("CarlaPlugin::newSF2({%p, \"%s\", \"%s\", \"%s\"})", init.engine, init.filename, init.name, init.label);

#ifdef WANT_FLUIDSYNTH

    if (! fluid_is_soundfont(init.filename))
    {
        init.engine->setLastError("Requested file is not a valid SoundFont");
        return nullptr;
    }

    FluidSynthPlugin* const plugin = new FluidSynthPlugin(init.engine, init.id);

    if (! plugin->init(init.filename, init.name, init.label))
    {
        delete plugin;
        return nullptr;
    }

    plugin->reload();
    plugin->registerToOscClient();

    return plugin;
#else
    init.engine->setLastError("fluidsynth support not available");
    return nullptr;
#endif
}

CARLA_BACKEND_END_NAMESPACE
