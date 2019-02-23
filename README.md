First test with DCC controlled Kato N-scale Turntable 20-283<BR>
http://www.katousa.com/N/Unitrack/Turntable.html<BR>
http://www.katousa.com/images/unitrack/20-283.jpg<BR>

Followed the standard from Littfinski DatenTechnik (LDT) TurnTable Decoder TT-DEC<BR>
https://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:tt-dec<BR>
DCC Address are:
* 225 - 0 = END
* 225 - 1 = INPUT (For now both will stop Turntable)
* 226 - 0 = CLEAR
* 226 - 1 = TURN 180
* 227 - 0 = 1 STEP CW
* 227 - 1 = 1 STEP CCW

Used Components:<BR>
Pololu DRV8835 Dual Motor Driver Shield for Arduino<BR>
https://www.pololu.com/product/2511<BR>
https://github.com/pololu/drv8835-motor-shield<BR>

2019-02-23
----------
Now working with DCC Address:
* 225 - 0 = STOP
* 226 - 0 = STOP
* 227 - 0 = 1 STEP CW
* 227 - 1 = 1 STEP CCW
