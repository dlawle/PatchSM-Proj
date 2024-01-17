#pragma once
#ifndef BABY_OSC_H
#define BABY_OSC_H
#include <stdint.h>
#include "Utility/dsp.h"
#ifdef __cplusplus

namespace daisysp
{
/** Synthesis of several waveforms, including polyBLEP bandlimited waveforms.
*/
class BabyOscillator
{
  public:
    BabyOscillator() {}
    ~BabyOscillator() {}

    /** Initializes the BabyOscillator

        \param sample_rate - sample rate of the audio engine being run, and the frequency that the Process function will be called.

        Defaults:
        - freq_ = 100 Hz
        - amp_ = 0.5
        - waveform_ = sine wave.
    */
    void Init(float sample_rate)
    {
        sr_        = sample_rate;
        sr_recip_  = 1.0f / sample_rate;
        freq_      = 100.0f;
        amp_       = 0.5f;
        pw_        = 0.5f;
        pw_rad_    = pw_ * TWOPI_F;
        phase_     = 0.0f;
        phase_inc_ = CalcPhaseInc(freq_);
        eoc_       = true;
        eor_       = true;
        mod_       = 10.f;
    }


    /** Changes the frequency of the BabyOscillator, and recalculates phase increment.
    */
    inline void SetFreq(const float f)
    {
        freq_      = f;
        phase_inc_ = CalcPhaseInc(f);
    }


    /** Sets the amplitude of the waveform.
    */
    inline void SetAmp(const float a) { amp_ = a; }

    /** Modifier to apply to signal (1 - 10)
    */
   inline void SetMod(const float n) { mod_ = n; };

    /** Sets the pulse width for WAVE_SQUARE and WAVE_POLYBLEP_SQUARE (range 0 - 1)
     */
    inline void SetPw(const float pw)
    {
        pw_     = fclamp(pw, 0.0f, 1.0f);
        pw_rad_ = pw_ * TWOPI_F;
    }

    /** Returns true if cycle is at end of rise. Set during call to Process.
    */
    inline bool IsEOR() { return eor_; }

    /** Returns true if cycle is at end of cycle. Set during call to Process.
    */
    inline bool IsEOC() { return eoc_; }

    /** Returns true if cycle rising.
    */
    inline bool IsRising() { return phase_ < PI_F; }

    /** Returns true if cycle falling.
    */
    inline bool IsFalling() { return phase_ >= PI_F; }

    /** Processes the waveform to be generated, returning one sample. This should be called once per sample period.
    */
    float Process();


    /** Adds a value 0.0-1.0 (mapped to 0.0-TWO_PI) to the current phase. Useful for PM and "FM" synthesis.
    */
    void PhaseAdd(float _phase) { phase_ += (_phase * TWOPI_F); }
    /** Resets the phase to the input argument. If no argumeNt is present, it will reset phase to 0.0;
    */
    void Reset(float _phase = 0.0f) { phase_ = _phase; }

  private:
    //int     N_; // number of harmonics
    float   mod_;
    float   CalcPhaseInc(float f);
    float   amp_, freq_, pw_, pw_rad_;
    float   sr_, sr_recip_, phase_, phase_inc_;
    float   last_out_, last_freq_;
    bool    eor_, eoc_;
};
} // namespace daisysp
#endif //BABY_OSC_H
#endif
