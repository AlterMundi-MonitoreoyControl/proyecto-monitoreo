Slice tex sensors

Increase speed 
mbpoll /dev/ttyUSB1 -b 4800 -d 8 -P none -r 0x7D2 -1 2

Read sped
mbpoll /dev/ttyUSB1 -b 9600 -d 8 -P none -r 0x7D2 -1


Read hum 
mbpoll /dev/ttyUSB1 -b 9600 -d 8 -P none -r 0x1 -1 
-- Polling slave 1...
[1]: 	399

Read temp 
mbpoll /dev/ttyUSB1 -b 4800 -d 8 -P none -r 0x2 -1 
-- Polling slave 1...
[2]: 	282

Change address
mbpoll /dev/ttyUSB1 -b 9600 -d 8 -P none -r 0x7D1 -1 45
Written 1 references.

mbpoll /dev/ttyUSB1 -b 9600 -d 8 -P none -r 0x7D1 -1 -a 45
-- Polling slave 45...
[2001]: 	45

now ... reading must be done to address 45

mbpoll /dev/ttyUSB1 -b 9600 -d 8 -P none -r 0x1 -1 -a 45
-- Polling slave 45...
[1]: 	416

mbpoll /dev/ttyUSB1 -b 9600 -d 8 -P none -r 0x2 -1 -a 45
-- Polling slave 45...
[2]: 	284


wiring 
ESP32   Adapter   
17 tx | DI
16 rx | RO 
18 de | DE-RE   

Adapter      sensor
GND  
VCC  (5v)
Yellow     |  D+ 
Blue       |  D- 