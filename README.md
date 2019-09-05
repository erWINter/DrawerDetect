# DrawerDetect
needed for Kitchen Control<br><br>
ATTENTION: program development is still in progress!<br>
(S/W compiles, but NOT yet tested)<br>

if included, this module provides extra information via I2C to a 16x2 LCD screen<br>
It shows either:<br>
- open percentage via bar-graph level (like VU)<br>
- or it shows the text messages sent to Kitchen Control for open drawers<br>
see following example:<br>
text = "3m50 5o99 7u05"  means:<br>
- in cabinet# 3 is the middle(DE:mitte) drawer half (50%) open<br>
- in cabinet# 5 is the upper(DE:oben) drawer full (99%) open<br>
- in cabinet# 7 is the lower(DE:unten) drawer nearly closed (or 5% open)<br>
