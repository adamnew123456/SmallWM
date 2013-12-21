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
- Window Snapping - Windows can be moved to the top-half, bottom-half, left-half, or right-half of the screen

Controls
========
The controls listed below are defined inside event.h, and can be configured only via modification and recompilation:

- `Super+PageUp`: Layers the current window onto the top
- `Super+PageDown`: Layers the current window onto the bottom
- `Super+m`: Maximizes the current window
- `Super+c`: Closes the current window
- `Super+x`: Forcefully closes the current window
- `Super+h`: Iconifies the current window
- `Super+[`: Move the current window to the previous desktop
- `Super+]`: Move the current window to the next desktop
- `Super+Escape`: Quits SmallWM
- `Super+,`: Move to the previous desktop
- `Super+.`: Move to the next desktop
- `Super+\`: Stick to all desktops/unstick
- `Super+Up` `Super+Down` `Super+Left` `Super+Right`: Snap window to that half of the screen
- `Super+LClick`
    - If this is used on the background, a new terminal will be launched
    - If this is used on a window, the window will begin moving. To set the window's position, let go of the left mouse button.
- `Super+RClick`: When used on a window, this combination starts resising a window. To set the window's size, let go of the right mouse button.

Building
========
The Makefile contains everything you need to build and test SmallWM.

Namely, it has the following interesting targets:
 - `make smallwm-debug` compiles a version with symbols useful for debugging
 - `make smallwm-release` compiles an optimized version useful for daily use

Configuration
=============
With the rewrite in place, a few things about SmallWM can now be configured:

    [smallwm]
    shell=your-preferred-terminal
    desktops=42
    icon_width=75
    icon_height=20

These are, respectively:

 - The shell launched by `Super+LClick` (default: xterm)
 - The number of desktops (default: 5)
 - The width in pixels of icons (default: 75)
 - The height in pixels of icons (default: 20)

Bugs/Todo
=========
- Support for the EWMH and the `_NET*` atoms

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
