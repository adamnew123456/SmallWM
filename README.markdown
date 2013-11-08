What is SmallWM?
================
SmallWM is an extended version of TinyWM, made for actual desktop use.

Improvements over TinyWM
========================
- Window Iconification - Windows can be hidden and placed in little rectangles at the top of the screen.
- Window Layering - Windows can be layered, but little policy is actually enforced.
- Click-To-Focus - SmallWM reimplements an idea from 9wm, keeping the focusing code small.
- Window Placeholders - SmallWM does not do window resizing and moving directly, because that is a graphically intensive operation. It instead uses placeholder windows that it deletes after moving.
- Window Borders - Just a simple border to see the extent of a window.
- Window Refreshing - Windows can be refreshed - this avoids subtle focus issues with applications like Chromium which cause certain issues.
- Multiple Desktops - SmallWM can handle multiple desktops - they are rotated circularly, and their number can be configured in `client.h`.
- Window Sticking - Along with multiple desktops, windows can be stuck to all the desktops

Controls
========
The controls of SmallWM are customizable - see `event.h`, specifically `SHORTCUTS` and `KEYBINDS`. The defaults are as follows:
- `Super+PageUp`: Layers the current window onto the top
- `Super+PageDown`: Layers the current window onto the bottom
- `Super+m`: Maximizes the current window
- `Super+c`: Closes the current window
- `Super+h`: Iconifies the current window
- `Super+r`: Refreshes the current window
- `Super+[`: Move the current window to the previous desktop
- `Super+]`: Move the current window to the next desktop
- `Super+Escape`: Quits SmallWM
- `Super+,`: Move to the previous desktop
- `Super+.`: Move to the next desktop
- `Super+\`: Stick to all desktops/unstick

Building
========
The Makefile contains everything you need to build and test SmallWM.

Namely, it has the following interesting targets:
- `make smallwm` builds a debug version useful for testing
- `mall smallwm-release` builds an optimized version useful for running
- `make xephyr-test` runs SmallWM via GDB inside a virtual X11 server so that you can test SmallWM without having to logout.

Bugs/Todo
=========
- Improved ICCCM compliance

Credits
=======
- Nick Welch <mack@incise.org>, the original TinyWM author.
- Myself (Adam Marchetti <adamnew123456@gmail.com>).
- Possibly, you - assuming you make any useful changes and I accept your pull request. Refactorings are welcome, as are those who are actually knowledgeable about Xorg and could spot any obvious mistakes.

License
=======
In the spirit of the original TinyWM, SmallWM is public domain as well. 

Simply keep this README file (or credit me and the original author) with any distribution you make.
