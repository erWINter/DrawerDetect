# DrawerDetect
needed for Kitchen Control<br><br>
if included, this module provides extra information via I2C to a 16x2 LCD screen<br>
It shows either:<br>
open percentage via bar-graph level (like VU)<br>
or it shows the text messages sent to Kitchen Control for open drawers; see following example:<br>
text = "3m50 5o99 7u05"  means:<br>
in cabinet# 3 is the middle drawer half (50%) open<br>
in cabinet# 5 is the oben(eng.upper) drawer full (99%) open<br>
in cabinet# 7 is the unten(eng.lower) drawer nearly closed (or 5% open)<br>
