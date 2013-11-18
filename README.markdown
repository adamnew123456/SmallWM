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
- Window Snapping - Windows can be moved to the top-half, bottom-half, left-half, or bottom-half of the screen

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
- `Super+Up` `Super+Down` `Super+Left` `Super+Right`: Snap window to that half of the screen

Building
========
The Makefile contains everything you need to build and test SmallWM.

Namely, it has the following interesting targets:
- `make smallwm` builds a debug version useful for testing
- `mall smallwm-release` builds an optimized version useful for running
- `make xephyr-test` runs SmallWM via GDB inside a virtual X11 server so that you can test SmallWM without having to logout.

Configuration
=============
The terminal that SmallWM uses can be configured. Open up ~/.config/smallwm, and put something like the following in:

  [smallwm]
  shell=your-preferred-terminal

The path to the terminal can be elided as long as it is on your $PATH. SmallWM will use this terminal when launching or
opening up new terminals with `Super-LClick`.

Bugs/Todo
=========
- Improved ICCCM compliance

Credits
=======
- Nick Welch <mack@incise.org>, the original TinyWM author.
- Myself (Adam Marchetti <adamnew123456@gmail.com>).
- The author(s) of the inih library.
- Possibly, you - assuming you make any useful changes and I accept your pull request. Refactorings are welcome, as are those who are actually knowledgeable about Xorg and could spot any obvious mistakes.

License
=======
SmallWM was migrated to the 2-Clause BSD License on 2013-11-18. See LICENSE.txt for details.

The inih code, included as a part of SmallWM, is available under the New BSD License. See inih/LICENSE.txt for details.
