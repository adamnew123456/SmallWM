#!/bin/bash
# Simple script used to debug SmallWM through Xephyr
Xephyr :20 &
sleep 5
DISPLAY=":20" gdb smallwm
