// placeholder for stmlib shit that needs covered


#include "../../../stmlib/stmlib.h"
#include <cmath>
#include <algorithm>

namespace vitiate {

using namespace stmlib;

class Random {
 public:
  static inline uint32_t state() { return rng_state_; }

  static inline void Seed(uint32_t seed) {
    rng_state_ = seed;
  }

  static inline uint32_t GetWord() {
    rng_state_ = rng_state_ * 1664525L + 1013904223L;
    return state();
  }
  
  static inline int16_t GetSample() {
    return static_cast<int16_t>(GetWord() >> 16);
  }

  static inline float GetFloat() {
    return static_cast<float>(GetWord()) / 4294967296.0f;
  }

 private:
  static uint32_t rng_state_;

  DISALLOW_COPY_AND_ASSIGN(Random);
};

enum FilterMode {
  FILTER_MODE_LOW_PASS,
  FILTER_MODE_BAND_PASS,
  FILTER_MODE_BAND_PASS_NORMALIZED,
  FILTER_MODE_HIGH_PASS
};

enum FrequencyApproximation {
  FREQUENCY_EXACT,
  FREQUENCY_ACCURATE,
  FREQUENCY_FAST,
  FREQUENCY_DIRTY
};

#define M_PI_F float(M_PI)
#define M_PI_POW_2 M_PI * M_PI
#define M_PI_POW_3 M_PI_POW_2 * M_PI
#define M_PI_POW_5 M_PI_POW_3 * M_PI_POW_2
#define M_PI_POW_7 M_PI_POW_5 * M_PI_POW_2
#define M_PI_POW_9 M_PI_POW_7 * M_PI_POW_2
#define M_PI_POW_11 M_PI_POW_9 * M_PI_POW_2

class DCBlocker {
 public:
  DCBlocker() { }
  ~DCBlocker() { }
  
  void Init(float pole) {
    x_ = 0.0f;
    y_ = 0.0f;
    pole_ = pole;
  }
  
  inline void Process(float* in_out, size_t size) {
    float x = x_;
    float y = y_;
    const float pole = pole_;
    while (size--) {
      float old_x = x;
      x = *in_out;
      *in_out++ = y = y * pole + x - old_x;
    }
    x_ = x;
    y_ = y;
  }
  
 private:
  float pole_;
  float x_;
  float y_;
};

class OnePole {
 public:
  OnePole() { }
  ~OnePole() { }
  
  void Init() {
    set_f<FREQUENCY_DIRTY>(0.01f);
    Reset();
  }
  
  void Reset() {
    state_ = 0.0f;
  }
  
  template<FrequencyApproximation approximation>
  static inline float tan(float f) {
    if (approximation == FREQUENCY_EXACT) {
      // Clip coefficient to about 100.
      f = f < 0.497f ? f : 0.497f;
      return tanf(M_PI_F * f);
    } else if (approximation == FREQUENCY_DIRTY) {
      // Optimized for frequencies below 8kHz.
      const float a = 3.736e-01f * M_PI_POW_3;
      return f * (M_PI_F + a * f * f);
    } else if (approximation == FREQUENCY_FAST) {
      // The usual tangent approximation uses 3.1755e-01 and 2.033e-01, but
      // the coefficients used here are optimized to minimize error for the
      // 16Hz to 16kHz range, with a sample rate of 48kHz.
      const float a = 3.260e-01f * M_PI_POW_3;
      const float b = 1.823e-01f * M_PI_POW_5;
      float f2 = f * f;
      return f * (M_PI_F + f2 * (a + b * f2));
    } else if (approximation == FREQUENCY_ACCURATE) {
      // These coefficients don't need to be tweaked for the audio range.
      const float a = 3.333314036e-01f * M_PI_POW_3;
      const float b = 1.333923995e-01f * M_PI_POW_5;
      const float c = 5.33740603e-02f * M_PI_POW_7;
      const float d = 2.900525e-03f * M_PI_POW_9;
      const float e = 9.5168091e-03f * M_PI_POW_11;
      float f2 = f * f;
      return f * (M_PI_F + f2 * (a + f2 * (b + f2 * (c + f2 * (d + f2 * e)))));
    }
  }
  
  // Set frequency and resonance from true units. Various approximations
  // are available to avoid the cost of tanf.
  template<FrequencyApproximation approximation>
  inline void set_f(float f) {
    g_ = tan<approximation>(f);
    gi_ = 1.0f / (1.0f + g_);
  }
  
  template<FilterMode mode>
  inline float Process(float in) {
    float lp;
    lp = (g_ * in + state_) * gi_;
    state_ = g_ * (in - lp) + lp;

    if (mode == FILTER_MODE_LOW_PASS) {
      return lp;
    } else if (mode == FILTER_MODE_HIGH_PASS) {
      return in - lp;
    } else {
      return 0.0f;
    }
  }
  
  template<FilterMode mode>
  inline void Process(float* in_out, size_t size) {
    while (size--) {
      *in_out = Process<mode>(*in_out);
      ++in_out;
    }
  }
  
 private:
  float g_;
  float gi_;
  float state_;
  
  DISALLOW_COPY_AND_ASSIGN(OnePole);
};



class Svf {
 public:
  Svf() { }
  ~Svf() { }
  
  void Init() {
    set_f_q<FREQUENCY_DIRTY>(0.01f, 100.0f);
    Reset();
  }
  
  void Reset() {
    state_1_ = state_2_ = 0.0f;
  }
  
  // Copy settings from another filter.
  inline void set(const Svf& f) {
    g_ = f.g();
    r_ = f.r();
    h_ = f.h();
  }

  // Set all parameters from LUT.
  inline void set_g_r_h(float g, float r, float h) {
    g_ = g;
    r_ = r;
    h_ = h;
  }
  
  // Set frequency and resonance coefficients from LUT, adjust remaining
  // parameter.
  inline void set_g_r(float g, float r) {
    g_ = g;
    r_ = r;
    h_ = 1.0f / (1.0f + r_ * g_ + g_ * g_);
  }

  // Set frequency from LUT, resonance in true units, adjust the rest.
  inline void set_g_q(float g, float resonance) {
    g_ = g;
    r_ = 1.0f / resonance;
    h_ = 1.0f / (1.0f + r_ * g_ + g_ * g_);
  }

  // Set frequency and resonance from true units. Various approximations
  // are available to avoid the cost of tanf.
  template<FrequencyApproximation approximation>
  inline void set_f_q(float f, float resonance) {
    g_ = OnePole::tan<approximation>(f);
    r_ = 1.0f / resonance;
    h_ = 1.0f / (1.0f + r_ * g_ + g_ * g_);
  }
  
  template<FilterMode mode>
  inline float Process(float in) {
    float hp, bp, lp;
    hp = (in - r_ * state_1_ - g_ * state_1_ - state_2_) * h_;
    bp = g_ * hp + state_1_;
    state_1_ = g_ * hp + bp;
    lp = g_ * bp + state_2_;
    state_2_ = g_ * bp + lp;
    
    if (mode == FILTER_MODE_LOW_PASS) {
      return lp;
    } else if (mode == FILTER_MODE_BAND_PASS) {
      return bp;
    } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
      return bp * r_;
    } else if (mode == FILTER_MODE_HIGH_PASS) {
      return hp;
    }
  }
  
  template<FilterMode mode_1, FilterMode mode_2>
  inline void Process(float in, float* out_1, float* out_2) {
    float hp, bp, lp;
    hp = (in - r_ * state_1_ - g_ * state_1_ - state_2_) * h_;
    bp = g_ * hp + state_1_;
    state_1_ = g_ * hp + bp;
    lp = g_ * bp + state_2_;
    state_2_ = g_ * bp + lp;
    
    if (mode_1 == FILTER_MODE_LOW_PASS) {
      *out_1 = lp;
    } else if (mode_1 == FILTER_MODE_BAND_PASS) {
      *out_1 = bp;
    } else if (mode_1 == FILTER_MODE_BAND_PASS_NORMALIZED) {
      *out_1 = bp * r_;
    } else if (mode_1 == FILTER_MODE_HIGH_PASS) {
      *out_1 = hp;
    }

    if (mode_2 == FILTER_MODE_LOW_PASS) {
      *out_2 = lp;
    } else if (mode_2 == FILTER_MODE_BAND_PASS) {
      *out_2 = bp;
    } else if (mode_2 == FILTER_MODE_BAND_PASS_NORMALIZED) {
      *out_2 = bp * r_;
    } else if (mode_2 == FILTER_MODE_HIGH_PASS) {
      *out_2 = hp;
    }
  }
  
  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
    
      float value;
      if (mode == FILTER_MODE_LOW_PASS) {
        value = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        value = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        value = bp * r_;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        value = hp;
      }
      
      *out = value;
      ++out;
      ++in;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  template<FilterMode mode>
  inline void ProcessAdd(const float* in, float* out, size_t size, float gain) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
    
      float value;
      if (mode == FILTER_MODE_LOW_PASS) {
        value = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        value = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        value = bp * r_;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        value = hp;
      }
      
      *out += gain * value;
      ++out;
      ++in;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size, size_t stride) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
    
      float value;
      if (mode == FILTER_MODE_LOW_PASS) {
        value = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        value = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        value = bp * r_;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        value = hp;
      }
      
      *out = value;
      out += stride;
      in += stride;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  inline void ProcessMultimode(
      const float* in,
      float* out,
      size_t size,
      float mode) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    float hp_gain = mode < 0.5f ? -mode * 2.0f : -2.0f + mode * 2.0f;
    float lp_gain = mode < 0.5f ? 1.0f - mode * 2.0f : 0.0f;
    float bp_gain = mode < 0.5f ? 0.0f : mode * 2.0f - 1.0f;
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
      *out = hp_gain * hp + bp_gain * bp + lp_gain * lp;
      ++in;
      ++out;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  inline void ProcessMultimodeLPtoHP(
      const float* in,
      float* out,
      size_t size,
      float mode) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    float hp_gain = std::min(-mode * 2.0f + 1.0f, 0.0f);
    float bp_gain = 1.0f - 2.0f * fabsf(mode - 0.5f);
    float lp_gain = std::max(1.0f - mode * 2.0f, 0.0f);
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
      *out = hp_gain * hp + bp_gain * bp + lp_gain * lp;
      ++in;
      ++out;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  template<FilterMode mode>
  inline void Process(
      const float* in, float* out_1, float* out_2, size_t size,
      float gain_1, float gain_2) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
    
      float value;
      if (mode == FILTER_MODE_LOW_PASS) {
        value = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        value = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        value = bp * r_;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        value = hp;
      }
      
      *out_1 += value * gain_1;
      *out_2 += value * gain_2;
      ++out_1;
      ++out_2;
      ++in;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  inline float g() const { return g_; }
  inline float r() const { return r_; }
  inline float h() const { return h_; }
  
 private:
  float g_;
  float r_;
  float h_;

  float state_1_;
  float state_2_;
  
  DISALLOW_COPY_AND_ASSIGN(Svf);
};



// Naive Chamberlin SVF.
class NaiveSvf {
 public:
  NaiveSvf() { }
  ~NaiveSvf() { }
  
  void Init() {
    set_f_q<FREQUENCY_DIRTY>(0.01f, 100.0f);
    Reset();
  }
  
  void Reset() {
    lp_ = bp_ = 0.0f;
  }
  
  // Set frequency and resonance from true units. Various approximations
  // are available to avoid the cost of sinf.
  template<FrequencyApproximation approximation>
  inline void set_f_q(float f, float resonance) {
    if (approximation == FREQUENCY_EXACT) {
      f = f < 0.497f ? f : 0.497f;
      f_ = 2.0f * sinf(M_PI_F * f);
    } else {
      f = f < 0.158f ? f : 0.158f;
      f_ = 2.0f * M_PI_F * f;
    }
    damp_ = 1.0f / resonance;
  }
  
  template<FilterMode mode>
  inline float Process(float in) {
    float hp, notch, bp_normalized;
    bp_normalized = bp_ * damp_;
    notch = in - bp_normalized;
    lp_ += f_ * bp_;
    hp = notch - lp_;
    bp_ += f_ * hp;
    
    if (mode == FILTER_MODE_LOW_PASS) {
      return lp_;
    } else if (mode == FILTER_MODE_BAND_PASS) {
      return bp_;
    } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
      return bp_normalized;
    } else if (mode == FILTER_MODE_HIGH_PASS) {
      return hp;
    }
  }
  
  inline float lp() const { return lp_; }
  inline float bp() const { return bp_; }
  
  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size) {
    float hp, notch, bp_normalized;
    float lp = lp_;
    float bp = bp_;
    while (size--) {
      bp_normalized = bp * damp_;
      notch = *in++ - bp_normalized;
      lp += f_ * bp;
      hp = notch - lp;
      bp += f_ * hp;
      
      if (mode == FILTER_MODE_LOW_PASS) {
        *out++ = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        *out++ = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        *out++ = bp_normalized;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        *out++ = hp;
      }
    }
    lp_ = lp;
    bp_ = bp;
  }
  
  inline void Split(const float* in, float* low, float* high, size_t size) {
    float hp, notch, bp_normalized;
    float lp = lp_;
    float bp = bp_;
    while (size--) {
      bp_normalized = bp * damp_;
      notch = *in++ - bp_normalized;
      lp += f_ * bp;
      hp = notch - lp;
      bp += f_ * hp;
      *low++ = lp;
      *high++ = hp;
    }
    lp_ = lp;
    bp_ = bp;
  }

  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size, size_t decimate) {
    float hp, notch, bp_normalized;
    float lp = lp_;
    float bp = bp_;
    size_t n = decimate - 1;
    while (size--) {
      bp_normalized = bp * damp_;
      notch = *in++ - bp_normalized;
      lp += f_ * bp;
      hp = notch - lp;
      bp += f_ * hp;
      
      ++n;
      if (n == decimate) {
        if (mode == FILTER_MODE_LOW_PASS) {
          *out++ = lp;
        } else if (mode == FILTER_MODE_BAND_PASS) {
          *out++ = bp;
        } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
          *out++ = bp_normalized;
        } else if (mode == FILTER_MODE_HIGH_PASS) {
          *out++ = hp;
        }
        n = 0;
      }
    }
    lp_ = lp;
    bp_ = bp;
  }
  
 private:
  float f_;
  float damp_;
  float lp_;
  float bp_;
  
  DISALLOW_COPY_AND_ASSIGN(NaiveSvf);
};



// Modified Chamberlin SVF (Duane K. Wise) 
// http://www.dafx.ca/proceedings/papers/p_053.pdf
class ModifiedSvf {
 public:
  ModifiedSvf() { }
  ~ModifiedSvf() { }
  
  void Init() {
    Reset();
  }
  
  void Reset() {
    lp_ = bp_ = 0.0f;
  }
  
  inline void set_f_fq(float f, float fq) {
    f_ = f;
    fq_ = fq;
    x_ = 0.0f;
  }
  
  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size) {
    float lp = lp_;
    float bp = bp_;
    float x = x_;
    const float fq = fq_;
    const float f = f_;
    while (size--) {
      lp += f * bp;
      bp += -fq * bp -f * lp + *in;
      if (mode == FILTER_MODE_BAND_PASS ||
          mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        bp += x;
      }
      x = *in++;
      
      if (mode == FILTER_MODE_LOW_PASS) {
        *out++ = lp * f;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        *out++ = bp * f;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        *out++ = bp * fq;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        *out++ = x - lp * f - bp * fq;
      }
    }
    lp_ = lp;
    bp_ = bp;
    x_ = x;
  }
  
 private:
  float f_;
  float fq_;
  float x_;
  float lp_;
  float bp_;
  
  DISALLOW_COPY_AND_ASSIGN(ModifiedSvf);
};



// Two passes of modified Chamberlin SVF with the same coefficients -
// to implement Linkwitz–Riley (Butterworth squared) crossover filters.
class CrossoverSvf {
 public:
  CrossoverSvf() { }
  ~CrossoverSvf() { }
  
  void Init() {
    Reset();
  }
  
  void Reset() {
    lp_[0] = bp_[0] = lp_[1] = bp_[1] = 0.0f;
    x_[0] = 0.0f;
    x_[1] = 0.0f;
  }
  
  inline void set_f_fq(float f, float fq) {
    f_ = f;
    fq_ = fq;
  }
  
  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size) {
    float lp_1 = lp_[0];
    float bp_1 = bp_[0];
    float lp_2 = lp_[1];
    float bp_2 = bp_[1];
    float x_1 = x_[0];
    float x_2 = x_[1];
    const float fq = fq_;
    const float f = f_;
    while (size--) {
      lp_1 += f * bp_1;
      bp_1 += -fq * bp_1 -f * lp_1 + *in;
      if (mode == FILTER_MODE_BAND_PASS ||
          mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        bp_1 += x_1;
      }
      x_1 = *in++;
      
      float y;
      if (mode == FILTER_MODE_LOW_PASS) {
        y = lp_1 * f;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        y = bp_1 * f;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        y = bp_1 * fq;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        y = x_1 - lp_1 * f - bp_1 * fq;
      }
      
      lp_2 += f * bp_2;
      bp_2 += -fq * bp_2 -f * lp_2 + y;
      if (mode == FILTER_MODE_BAND_PASS ||
          mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        bp_2 += x_2;
      }
      x_2 = y;
      
      if (mode == FILTER_MODE_LOW_PASS) {
        *out++ = lp_2 * f;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        *out++ = bp_2 * f;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        *out++ = bp_2 * fq;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        *out++ = x_2 - lp_2 * f - bp_2 * fq;
      }
    }
    lp_[0] = lp_1;
    bp_[0] = bp_1;
    lp_[1] = lp_2;
    bp_[1] = bp_2;
    x_[0] = x_1;
    x_[1] = x_2;
  }
  
 private:
  float f_;
  float fq_;
  float x_[2];
  float lp_[2];
  float bp_[2];
  
  DISALLOW_COPY_AND_ASSIGN(CrossoverSvf);
};



inline int32_t Clip16(int32_t x) {
if (x < -32768) {
    return -32768;
} else if (x > 32767) {
    return 32767;
} else {
    return x;
}
}

#define MAKE_INTEGRAL_FRACTIONAL(x) \
  int32_t x ## _integral = static_cast<int32_t>(x); \
  float x ## _fractional = x - static_cast<float>(x ## _integral);

inline float Interpolate(const float* table, float index, float size) {
  index *= size;
  MAKE_INTEGRAL_FRACTIONAL(index)
  float a = table[index_integral];
  float b = table[index_integral + 1];
  return a + (b - a) * index_fractional;
}

#define ONE_POLE(out, in, coefficient) out += (coefficient) * ((in) - out);
#define SLOPE(out, in, positive, negative) { \
  float error = (in) - out; \
  out += (error > 0 ? positive : negative) * error; \
}
#define SLEW(out, in, delta) { \
  float error = (in) - out; \
  float d = (delta); \
  if (error > d) { \
    error = d; \
  } else if (error < -d) { \
    error = -d; \
  } \
  out += error; \
}


//Convert from semitones to other units. e.g. 2 ^ (kOneTwelfth * x)
static constexpr float kOneTwelfth = 1.f / 12.f;

float SemitonesToRatio(float in) { return powf(2.f, in * kOneTwelfth); }

/** Soft Limiting function ported extracted from pichenettes/stmlib */
inline float SoftLimit(float x)
{
    return x * (27.f + x * x) / (27.f + 9.f * x * x);
}

inline float SoftClip(float x)
{
    if(x < -3.0f)
        return -1.0f;
    else if(x > 3.0f)
        return 1.0f;
    else
        return SoftLimit(x);
}

class ParameterInterpolator {
 public:
  ParameterInterpolator() { }
  ParameterInterpolator(float* state, float new_value, size_t size) {
    Init(state, new_value, size);
  }

  ParameterInterpolator(float* state, float new_value, float step) {
    state_ = state;
    value_ = *state;
    increment_ = (new_value - *state) * step;
  }

  ~ParameterInterpolator() {
    *state_ = value_;
  }
  
  inline void Init(float* state, float new_value, size_t size) {
    state_ = state;
    value_ = *state;
    increment_ = (new_value - *state) / static_cast<float>(size);
  }

  inline float Next() {
    value_ += increment_;
    return value_;
  }

  inline float subsample(float t) {
    return value_ + increment_ * t;
  }
  
 private:
  float* state_;
  float value_;
  float increment_;
};

}  // namespace stmlib