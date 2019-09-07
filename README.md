# DrawerDetect
needed for Kitchen Control<br><br>
ATTENTION: program development is still in progress!<br>
(S/W compiles, but NOT yet tested)<br>
<br>this is a S/W for one of 3 ProTrinkets, which detect (max. 7) open drawers,<br>
by using an Ultra Sonic transmitter per drawer, which is attached at the far end of the drawer.<br>
All open drawers are signalled to the KitchenControl Arduino (BT Feather), which steers RGB- and WW-LEDs<br>
(see the overview png file in erWINter/KitchenControl)<br>
<br>
LCD_I2C_optional.h:<br>
if included, this module provides extra information via I2C to a 16x2 LCD screen<br>
It shows either:<br>
- open percentage via bar-graph level (like VU)<br>
- or it shows the text messages sent to Kitchen Control for open drawers<br>
see following example:<br>
text = "3m50 5o99 7u05"  means:<br>
- in cabinet# 3 is the middle(DE:mitte) drawer half (50%) open<br>
- in cabinet# 5 is the upper(DE:oben) drawer full (99%) open<br>
- in cabinet# 7 is the lower(DE:unten) drawer nearly closed (or 5% open)<br>
