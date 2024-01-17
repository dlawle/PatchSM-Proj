import numpy as np
from scipy.io import wavfile as wave
import dsp
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

sampling_rate = 44100
frequency = 440
duration = 5
s_start = 5
mod_to_apply = "saturate"
# Create the time array
t = np.linspace(0, duration, int(sampling_rate * duration), endpoint=False)


"""----------------------------------------------------------------------------
signal setup
----------------------------------------------------------------------------"""


def equation(x, mod):
    p = mod


    # saw wave
    #result = np.mod((p*x)/(0.5*np.pi),0.5)
    
    # weird harmonic wave 
    #result = np.sin(p*x) - (1 / 2) * np.sin(2 * p*x) + (1 / 3) * np.sin(3 * p*x) - (1 / 4) * np.sin(4 * p*x) + (1 / 5) * np.sin(5 * p*x) - (1 / 6) * np.sin(6 * p*x)

    # harmonic combinations
    #result = np.sin(p * t) * (4 * np.abs(t - np.floor(t + 3/4) + 1/4))

    #square + sine weirdness. 
    #result = np.sign(np.sin(2*np.pi*x)+p) + np.sin(x + p)

    # dancing sine?
    #result = 2*(np.sin(np.sin(p*np.pi*x)+p))/np.pi

    # sawtooth_wave
    #result = 2 * (t * frequency - np.floor(t * frequency + 0.5))
    
    # square_wave
    #result = np.sign(np.sin(2 * np.pi * frequency * t))
    
    # sine_wave 
    result = np.sin(2 * np.pi * frequency * t)
    
    # triangle_wave
    #result = 2 * np.abs(2 * (t * frequency - np.floor(t * frequency + 0.5))) - 1

    return result


"""----------------------------------------------------------------------------
Application of filters/signal processing
----------------------------------------------------------------------------"""

# Create the function to apply the soft saturation filter
def apply_mod(data, mod, value):
    match mod:
        case "saturate":
            vectorized = np.vectorize(dsp.soft_saturate)
            return vectorized(data, value)
        
        case "clip":
            vectorized = np.vectorize(dsp.SoftClip)
            return vectorized(data)

        case "hysteresis":
            vectorized = np.vectorize(dsp.hysteresis_filter)
            return vectorized(data, value)
        
        case "limiter":
            vectorized = np.vectorize(dsp.Limiter)
            return vectorized(data, value)
        
        case "fold":
            vectorized = np.vectorize(dsp.fold_distort)
            return vectorized(data, value)
        
        case "lowpass":
            vectorized = np.vectorize(dsp.lowpass)
            return vectorized(data, sampling_rate, 2000, 1)
        
        case " ":
            print("none")


"""----------------------------------------------------------------------------
Utilities: wav writer and wable writer
----------------------------------------------------------------------------"""

def waveWrite(audio_data):
    amplitude = np.iinfo(np.int16).max
    data = amplitude * audio_data
    wave.write("output/example.wav", sampling_rate, data.astype(np.int16))



"""----------------------------------------------------------------------------
Plots 
----------------------------------------------------------------------------"""


# Create a function to update the plot when the main waveform slider value changes
def update_waveform(val):
    mod_waveform = slider_mod_waveform.val
    audio_data = equation(t, mod_waveform)
    slider_mod_waveform.val = slider_mod_waveform.val
    audio_data_normalized = np.int16(audio_data * 32767)
    
    # Update the plot data and title for the main waveform
    line.set_ydata(audio_data)
    ax.set_title('Custom Function with mod_waveform={}'.format(mod_waveform))
    
    # Redraw the canvas
    fig.canvas.draw_idle()

# Create a function to update the plot when the soft saturation slider value changes
def update_mod(val):
    mod_update = slider_mod_update.val
    audio_data_waveform = equation(t, slider_mod_waveform.val)
    
    # Apply soft saturation with the new mod value
    processed_values_mod = apply_mod(audio_data_waveform, mod_to_apply, mod_update)
    
    # Update the plot data for the soft saturation waveform
    line_mod.set_ydata(processed_values_mod)
    
    # Update the plot title for the soft saturation waveform
    ax.set_title('Custom Function with mod_waveform={} and modifier={}'.format(slider_mod_waveform.val, mod_update))
    
    # Redraw the canvas
    fig.canvas.draw_idle()



# Create the figure and axis
fig, ax = plt.subplots(figsize=(10, 5))
plt.subplots_adjust(bottom=0.35)
plt.xlim(0, 0.01)  # Adjust the limits as needed

# Plot the initial data for the main waveform
line, = ax.plot(t, equation(t, 5))
ax.set_xlabel('t')
ax.set_ylabel('Output')
ax.set_ylim(-1,1)
if mod_to_apply == "lowpass":
    ax.set_ylim(-3,3)
ax.set_title('Custom Function with mod_waveform=5')
ax.grid(True)

if mod_to_apply != "none":
    # Create the soft saturation line and plot it with the initial data
    line_mod, = ax.plot(t, apply_mod(equation(t, 5), mod_to_apply, 0.5), label='Waveform Mod')
    ax_mod_update = plt.axes([0.2, 0.07, 0.65, 0.03])
    slider_mod_update = Slider(ax_mod_update, 'modifier', 0.0, 1.0, valinit=0.5)
    slider_mod_update.on_changed(update_mod)

# Create the Slider widgets for the main waveform and soft saturation filter
ax_mod_waveform = plt.axes([0.2, 0.02, 0.65, 0.03])

slider_mod_waveform = Slider(ax_mod_waveform, 'waveform', 0.0, 10.0, valinit=5.0)

# Connect the slider update functions to the respective slider widgets
slider_mod_waveform.on_changed(update_waveform)

waveWrite(apply_mod(equation(t,5),mod_to_apply, 0))  # Write the modulated waveform to the wave file
plt.show()