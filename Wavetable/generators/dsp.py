import numpy as np

# Initialize state variables
x1 = 0.0
x2 = 0.0
y1 = 0.0
y2 = 0.0

# soft saturate test (from DaisySP)
def soft_saturate(inp, thresh):
    flip = inp < 0.0
    val = -inp if flip else inp
    out = 0.0
    
    if val < thresh:
        out = inp
    elif val > 1.0:
        out = (thresh + 1.0) / 2.0
        if flip:
            out *= -1.0
    elif val > thresh:
        temp = (val - thresh) / (1 - thresh)
        out = thresh + (val - thresh) / (1.0 + (temp * temp))
        if flip:
            out *= -1.0
    
    return out

# soft limit (From DaisySP)
def SoftLimit(x):
    return x * (27 + x * x) / (27 + 9 * x * x)

# soft clip (from DaisySP)
def SoftClip(x):
    if x < -3.0:
        return -1.0
    elif x > 3.0:
        return 1.0
    else:
        return SoftLimit(x)
    
# from stmlib
def SLOPE(output, inpint, positive, negative):
  error = (inpint) - output
  output += (positive if error > 0 else negative) * error

# from stmlib
def Limiter(input, pre_gain):
    peak_ = 0.5
    s = input * pre_gain
    SLOPE(peak_, np.abs(s), 0.05, 0.00002)
    gain = 1.0 if peak_ <= 1.0 else 1.0 / peak_
    input += s * gain * 0.8
    return input

# from stmlib
def hysteresis_filter(value, threshold):
    value_ = 0.0
    threshold_ = threshold
    if (threshold == 0.0):
      value_ = value
    else:
      error = value - value_
      if (error > threshold):
        value_ = value - threshold
      elif (error < -threshold):
        value_ = value + threshold
    return value_

def fold_distort(input, mod):
    return mod * input - (mod * 0.25) * input * input * input

def lowpass(input, samplerate, cutoff, resonancedB):
    global x1, x2, y1, y2
    
    c = 1.0 / (np.tan(np.pi * (cutoff / samplerate)))
    csq = c * c
    resonance = np.power(10, -(resonancedB * 0.1))
    q = np.sqrt(2.0) * resonance
    a0 = 1.0 / (1.0 + (q * c) + (csq))
    a1 = 2.0 * a0
    a2 = a0
    b1 = (2.0 * a0) * (1.0 - csq)
    b2 = a0 * (1.0 - (q * c) + csq)

    output = a0 * input + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2

    # Update state variables
    x2 = x1
    x1 = input
    y2 = y1
    y1 = output

    return output
