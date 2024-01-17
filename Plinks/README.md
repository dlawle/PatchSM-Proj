# Plinks
a plucked string arpeggiator with modifiable speed for rich tone creation

## Author
Daniel Lawler

## Description
### Plinks is a combination pluck engine and rich texturized "background noise generator". 

![image](https://user-images.githubusercontent.com/39495124/224424011-034505a8-f5aa-4185-a906-0daffa765b25.png)


[Audio sample of raw output displaying clock speed and reverb/filter levels](https://soundcloud.com/dlawler/plinks?si=e33e9bdff9154520beaf0bd30887ca63&utm_source=clipboard&utm_medium=text&utm_campaign=social_sharing)

Plinks takes a 1v/oct CV in and Quantizes it to a midi note, which then creates an arpeggio scale and applies it to a PolyPluck instance with 32 separate voices. These voices are passed through a filter and reverb to create lush walls of sound.

Most if not all of this code was adapted from DaisyExamples repo with a few other things mixed in. 
The code itself is built to work on a custom board (to be realeased) called the Daisy Bed, 
utilizing the Patch SM. Below is 
an outline of the mapped parameters. It should work on just about any board, with a few tweaks! 

The 4 LEDs on the front panel indicate which arpeggio is selected. There are 4 modes:
Major Pentad
Minor Pentad
Major Triad
Minor Triad
(check arp_notes.h for ideas and try out your own!)

| Pin Name | Function | Comment |
| --- | --- | --- |
|  CV_1 | Moog Ladder Filter (LPF Mode) Frequency | This is set to effect only the PolyPluck voice, and allows to take off some of the "edge" of the harder plucks |
|  CV_2 | Reverb Low Pass Filter (Frequency) | Modify the frequency of the reverb's built in LPF |
|  CV_3 | Reverb Feedback | Set the feedback amount of the reverb (warning - expect clipping in the 0.9-1.0 range) | 
|  CV_4 | PolyPluck Decay | This is a feature I plan on "tightening" as the lower end of the decay doesn't generate a very pleasent sound | 
|  CV_8 | PolyPluck initial Frequency | The voltage input is quantized to a midi note number, and then act's as a "base" note number for the selected arpeggio |
| ADC_11| Envelope Attack | Modify the internal envelope's attack rate |
| ADC_12| Envelope Decay | Modify the internal envelope's decay rate |
| Gate In 1 | Pluck Speed | The speed here is intended to be run at an almost "unnaturally high" rate in order to create rich tones with the reverb, though you can create interesting sounds with slower speeds | 
| Gate In 2 | Trigger Envelope | This triggers the internal envelope to fire which provides a swell on the outputs | 
| Button 1 | Arp Mode | Switch between 4 arpeggio modes (Major Triad, Minor Triad, Major Pentad, Minor Pentad). See arp_notes.h for customization and modification |
| Audio Out L | Reverb + Plucks, no envelope | | 
| Audio Out R |  Reverb + Plucks, with envelope | |

## Note 
Some peripherals are left unused, but will be updated later! 

Update: This project uses a custom board called the Daisy-Bed. [Click here to go the Daisy-Bed schematics and board files](https://github.com/dlawle/DaisyBed)
