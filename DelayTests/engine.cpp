#include "engine.h"

extern daisy::patch_sm::DaisyPatchSM hw;
static ReverbSc DSY_SDRAM_BSS reverb;
const int   kBufferDuration     = 10;                       // In seconds
const int   kBufferSize         = 48000 * kBufferDuration;
const int kGrainCount = 5;
float DSY_SDRAM_BSS grainBuffer[kBufferSize * kGrainCount];
#define MAX_DELAY static_cast<size_t>(48000)
static graindelay::Grain grains[kGrainCount];

namespace daisy
{
    Engine::Engine() {}

    Engine::~Engine() {}

    void Engine::Init(float sample_rate)
    {
        samp_rate = sample_rate;
        del.Init();
        del_second.Init();
        reverb.Init(samp_rate);

        const float readSpeeds[kGrainCount] = {0.5f, 0.25f, 1.4983f, -1.0f, -0.5f};    // In samples
        const float mixValues[kGrainCount] = {0.25f, 0.2f, 0.1f, 0.3f, 0.15f};    
        for (int i = 0; i < kGrainCount; i++)
        {
            grains[i].Init(samp_rate,&grainBuffer[kBufferSize * i],kBufferSize);
            grains[i].SetAmp(mixValues[i] * 1.66f);
            grains[i].SetSpeed(readSpeeds[i]);
        }
    }

    void Engine::SetParam(float pot1, float pot2, float pot3, float pot4, float pot5, float pot6, float pot7, float pot8)
    {
        GetProcessor();
        switch (effect_suite)
        {
            case 1:
                // Background mode
                delay_time1 = (0.001f + (pot1 * pot1) * 5.0f) * samp_rate;
                delay_time2 = (0.001f + (pot2 * pot2) * 5.0f) * samp_rate;
                feedbackamt = fmap(pot5, 0.0f, 0.99f);
                feedbackamt2 = fmap(pot6, 0.0f, 0.99f);
                reverb_time = fmap(pot3, 0.3f, 0.99f);
                reverb_filter = fmap(pot7, 1000.f, 19000.f, Mapping::LOG);

                fonepole(smooth_time1, delay_time1, 0.0005f);
                del.SetDelay1(smooth_time1);
                fonepole(smooth_time2, delay_time2, 0.0005f);
                del_second.SetDelay1(smooth_time2);
                reverb.SetFeedback(reverb_time);
                reverb.SetLpFreq(reverb_filter);
                break;
            case 2:
                break;

            case 3:
                break;

            case 4:
                // Grain mode
                g_amp_amt = pot1;
                g_dur_amt = pot2;
                g_fb_amt = pot3;
                g_dens_amt = pot5;
                g_speed_amt = pot6;

                reverb_time = fmap(pot4, 0.3f, 0.99f);
                reverb_filter = fmap(pot8, 1000.f, 19000.f, Mapping::LOG);

                for (int i = 0; i < kGrainCount; i++)
                {
                    grains[i].SetAmp(g_amp_amt);
                    grains[i].SetDuration(g_dur_amt);
                    grains[i].SetFeedback(g_fb_amt);
                    grains[i].SetGrainDensity(g_dens_amt);
                    grains[i].SetSpeed(g_speed_amt);
                }
                break;

            default:
                break;
        }
    }

    void Engine::Write(float in)
    {
        float feedback = (del_out * feedbackamt) + in;
        float feedback2 = (second_phase_out * feedbackamt2) + del_out;

        del.Write(feedback);
        del_second.Write(feedback2);
    }

    void Engine::Feed()
    {
        del_out = del.ReadRev();
    }

    float Engine::Process(float in)
    {
        switch (effect_suite)
        {
        case 1:
            // Background Mode
            if (activate)
            {
                Write(in);
            }
            del_out = del.ReadRev();
            second_phase_out = del_second.ReadFwd();
            reverb.Process(second_phase_out, second_phase_out, &output, &output);
            break;

        case 2:
            output = del.ReadFwd();
            break;

        case 3:
            output = in;
            break;

        case 4:
            // Grain mode
            if (activate)
            {
                for (int i = 0; i < kGrainCount; i++)
                {
                    float g_processed = grains[i].Process(in);
                    reverb.Process(g_processed, g_processed, &output, &output);
                }
            }
            break;

        default:
            break;
        }
        return output;
    }

    void Engine::UpdateProcesor(float type)
    {
        effect_suite = type;
    }

    float Engine::GetProcessor()
    {
        return effect_suite;
    }

    void Engine::GetActivate(bool signal)
    {
        activate = signal;
    }
} // namespace daisy
