# Poetry-Reader
I made this project for my dad's birthday -- he loves reading poetry, but doesn't want to lug around heavy poetry books. I built this pocket-sized poetry reader to solve that problem.

## Demo (videos + images)

https://github.com/user-attachments/assets/ed4d790f-b5af-4959-8c8f-0a64da99f5de

https://github.com/user-attachments/assets/21cbcda3-095a-4b04-85aa-d2876527afe0

<img width="480" height="640" alt="IMG_2390" src="https://github.com/user-attachments/assets/d8d349e3-b7f4-4dd6-984e-9875f0d15d8d" />

## Files
On this repo the code is a Platformio project, including the data sent via LittleFS to the Poetry Reader. The Schematic and PCB are in a Kicad project, which you can preview [here](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fimhwinters%2FPoetry-Reader%2Ftree%2Fmain%2FPCB%2520And%2520Schematic). 
The PCB isn't totally accurate -- due to manufacturing time/expenses I ended up just soldering this project on perfboard. The different layout meant that I ended up using different MCU pins for buttons based on convenience.

## BOM
|Part|Count|
|----|-----:|
|Xiao seeed esp32c3|1|
|Tactile buttons|6|
|22 guage wire (solid-core or threaded works, but I used solid-core)|idk|
|1.5" OLED IIC Screen|1|
|[3d Printed Case](https://cad.onshape.com/documents/d6cbfe09ed0ba8b33bb9ea9c/w/6fbf33e595bea7372ae94027/e/cdf8b9d88ac8b494c49c5fa8)|1|
