import numpy as np
from scipy.io import wavfile as wave

# Parameters
sampling_rate = 44100

# Read the original audio file
original_sampling_rate, original_audio = wave.read("output/example.wav")

# Read the impulse response (reverb) audio file
impulse_sampling_rate, impulse_response = wave.read("output/example.wav")

# Ensure both audio signals have the same sampling rate
if original_sampling_rate != impulse_sampling_rate:
    raise ValueError("Sampling rates of original audio and impulse response must match.")

# Perform convolution to apply reverb effect
reverb_audio = np.convolve(original_audio, impulse_response, mode="full")

# Normalize the reverb audio to ensure it's within the valid range [-1, 1]
reverb_audio_normalized = reverb_audio / np.max(np.abs(reverb_audio))

# Write the reverb audio to a new wave file
wave.write("output/reverb_audio.wav", sampling_rate, reverb_audio_normalized.astype(np.int16))
