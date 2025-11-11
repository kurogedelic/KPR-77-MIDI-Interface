# KPR-77-MIDI-Interface
**Beta project**

## Build
Please use Arduino IDE with DxCore

## Drum Map
| GM | Note |	Name (GM)	| KPR-77 Voice |
|--|--|--|--|
| 35 | 	Acoustic Bass Drum	 | Bass Drum | 1 |
| 36 | 	Bass Drum 1 | 	Bass Drum (Alt) | 	2 |
| 38 | 	Acoustic Snare	 | Snare	 | 3 |
| 40 | 	Electric Snare | Snare (Alt)	 | 4 |
| 41 | 	Low Floor Tom	 | Low Tom	 | 5 |
| 45 | 	Low Tom	 | Low Tom (Alt) | 	6 |
| 48 | 	High Tom | 	High Tom	 | 7 |
| 42 | 	Closed Hi-Hat | 	Closed HH | 	8 |
| 44 | 	Pedal Hi-Hat | 	Closed HH (Alt)	 | 9 |
| 46 | 	Open Hi-Hat	 | Open HH | 	10 |
| 49 | 	Crash Cymbal	 | Cymbal	 | 11 |
| 39 | 	Hand Clap | 	Claps | 	12 |
| 37 | 	Side Stick / Click | 	Metronome Click | 	any |
| 56 | 	Cowbell | 	Metronome Bell | 	any |
| 57 | 	Crash 2 | 	Accent | 	req. |


## BOM
+ AVR16EB28-I/SP
+ TLP2361 (or Similar isolate photocoupler)
+ Zener diode (6V->5V)
+ 1N4148
+ 220Ω
+ 1kΩ

## Config 
by Sysex.

## Credit
by Leo Kuroshita
