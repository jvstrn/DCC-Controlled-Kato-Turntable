DCC Controlled Kato N-scale Turntable 20-283<BR>
--------------------------------------------
http://www.katousa.com/N/Unitrack/Turntable.html<BR>
http://www.katousa.com/images/unitrack/20-283.jpg<BR>

Followed the standard from Littfinski DatenTechnik (LDT) TurnTable Decoder TT-DEC<BR>
https://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:tt-dec<BR>

Used Components:<BR>
Pololu DRV8835 Dual Motor Driver Shield for Arduino<BR>
https://www.pololu.com/product/2511<BR>
https://github.com/pololu/drv8835-motor-shield<BR>

DCC Shield for Arduno:<BR>
https://www.arcomora.com/mardec/<BR>

Controller will be used with:<BR>
ESU ECoS and iTrain<BR>
http://www.esu.eu/en/products/digital-control/<BR>

Uhlenbrock Intellibox and iTrain<BR>
https://www.uhlenbrock.de/de_DE/produkte/digizen/index.htm<BR>

Model railroad control program called: iTrain:<BR>
https://www.berros.eu/en/itrain/<BR>

2020-06-14 - Added EEPROM capability
----------
Store turntable bridge position when positioned<BR>
Read turntable bridge position at power up.

2020-06-11 - Added Additional Addresses for Direct Goto Track xx
----------
Now working with DCC Addresses:
* 225 - 0 = Red Button --> STOP
* 226 - 0 = Red Button --> POS (Set Track = 0 when Bridge in Home Position)
* 226 - 1 = Green Button --> T180 (Turn 180 degrees ClockWise)
* 227 - 0 = Red Button --> 1 STEP CW
* 227 - 1 = Green Button --> 1 STEP CCW
* 228 - 0 = Red Button --> Turn CW
* 228 - 1 = Green Button --> Turn CCW
* 229 - 0 = Red Button --> Goto Track 1
* 229 - 1 = Green Button --> Goto Track 2
* 230 - 0 = Red Button --> Goto Track 3
* 230 - 1 = Green Button --> Goto Track 4
* 231 - 0 = Red Button --> Goto Track 5
* 231 - 1 = Green Button --> Goto Track 6
* 232 - 0 = Red Button --> Goto Track 7
* 232 - 1 = Green Button --> Goto Track 8
* 233 - 0 = Red Button --> Goto Track 9
* 233 - 1 = Green Button --> Goto Track 10
* 234 - 0 = Red Button --> Goto Track 11
* 234 - 1 = Green Button --> Goto Track 12
* 235 - 0 = Red Button --> Goto Track 31
* 235 - 1 = Green Button --> Goto Track 32
* 236 - 0 = Red Button --> Goto Track 33
* 236 - 1 = Green Button --> Goto Track 34
* 237 - 0 = Red Button --> Goto Track 35
* 237 - 1 = Green Button --> Goto Track 36

2019-03-13 - Small Demo Video
----------
https://youtu.be/28wLVEbTYVs

2019-02-24 - Added Turn 180 and Goto Track
----------
Now working with DCC Addresses:
* 225 - 0 = Red Button --> STOP
* 226 - 0 = Red Button --> POS (Set Track = 0 when Bridge in Home Position)
* 226 - 1 = Green Button --> T180 (Turn 180 degrees ClockWise)
* 227 - 0 = Red Button --> 1 STEP CW
* 227 - 1 = Green Button --> 1 STEP CCW
* 228 - 0 = Red Button --> Turn CW
* 228 - 1 = Green Button --> Turn CCW
* 229 - 0 = Red Button --> Goto Track 1
* 229 - 1 = Green Button --> Goto Track 2

2019-02-23 - Improved Postion Detection; no false stop commands
----------
Now working with DCC Addresses:
* 225 - 0 = STOP
* 226 - 0 = POS (Set Track = 0 when Bridge in Home Position)
* 227 - 0 = 1 STEP CW
* 227 - 1 = 1 STEP CCW

2019-02-22 - First Test
----------
Now working with DCC Address:
* 225 - 0 = STOP
* 226 - 0 = STOP
* 227 - 0 = 1 STEP CW
* 227 - 1 = 1 STEP CCW




