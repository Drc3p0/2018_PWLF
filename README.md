# 2018_PWLF

TX version: 7 sensors per board, 4 boards controlling 28 keys, sending serial to WAV trigger.  All unused piezo inputs grounded out. <br> 

RS485 between every 7 light bars. Teensy reads RS485 from light bars and ouputs midi to Arduino, which triggers a WAV Trigger by robertsonics. <br>

Box Clusters 0 & 1 are sending midi notes 0-15 <br>
Box Clusters 2 & 3 are sending midi notes 16-31 <br>
Box Clusters 4 & 5 are sending midi notes 32-47 <br>
Box Clusters 6 & 7 are sending midi notes 48-63 <br>
Box Clusters 8 & 9 are sending midi notes 64-79 <br>
Box Clusters 10 & 11 are sending midi notes 80-95 <br>
<br>

5vDC  Supply <br>
__ amps in attractor mode <br>
__ amps under heavy activity <br>

Hardware Description <br>
Light bars <br>
7 bars per PCB <br>
4 PCBs total for 28 keys <br>
- 62 WS2812 leds per strip <br>
- several piezos wired in paralell per strip <br>
