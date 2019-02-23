# JVS_DRV8835MotorShield_DCC_Test

First test with DCC controlled Kato N-scale Turntable 20-283
* http://www.katousa.com/N/Unitrack/Turntable.html
* http://www.katousa.com/images/unitrack/20-283.jpg

Followed the standard from Littfinski DatenTechnik (LDT) TurnTable Decoder TT-DEC
* https://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:tt-dec
DCC Address are:
* 225 - 0 = END
* 225 - 1 = INPUT (For now both will stop Turntable)
* 226 - 0 = CLEAR
* 226 - 1 = TURN 180
* 227 - 0 = 1 STEP CW
* 227 - 1 = 1 STEP CCW

2019-02-23
----------
Now working with DCC Address:
* 225 - 0 = STOP
* 226 - 0 = STOP
* 227 - 0 = 1 STEP CW
* 227 - 1 = 1 STEP CCW
