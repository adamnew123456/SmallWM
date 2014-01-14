What is SmallWM?
================
SmallWM is an extended version of TinyWM, made for actual desktop use.

Improvements over TinyWM
========================
- Window Iconification - Windows can be hidden and placed in little rectangles at the top of the screen. Clicking the rectangles re-opens the client.
- Window Layering - Windows can be layered, and layerings are enforced and persistent. Windows are layered in "levels" from 1 to 9; 1 is the lowest level and 9 is the highest level. The default level for all windows is 5.
- Click-To-Focus - SmallWM reimplements an idea from 9wm, keeping the focusing code small.
- Window Placeholders - SmallWM does not do window resizing and moving directly, because that is a graphically intensive operation. It instead uses placeholder windows that it deletes after moving.
- Window Borders - Just a simple border to see the extent of a window.
- Multiple Desktops - SmallWM can handle multiple desktops - they are rotated circularly, and their number can be configured.
- Window Sticking - Along with multiple desktops, windows can be stuck to all the desktops.
- Window Snapping - Windows can be moved to the top-half, bottom-half, left-half, or right-half of the screen.

Controls
========

These controls are defined in event.h, `keysym_callbacks[]`; to change the controls below, modify this file and recompile (make sure to use the `-B` flag to force the recompilation).

Note that there are some shortcuts which are defined by the "current window"; this means the window that the pointer is hovering above, _not_ the currently focused window.

## Desktops ##

- `Super+[`, `Super+]`: Move a client to the previous desktop (`[`) or the next desktop (`]`).
- `Super+,`, `Super+.`: Switch to the previous desktop (`,`) or the next desktop (`.`).
- `Super+\`: Sticks/unsticks a window (a _stuck_ window is shown on all desktops).

## Clients ##

- `Super+h`: Iconifies the current client.
- `Super+m`: Maximizes the current client.
- `Super+c`, `Super+x`: Closes (`c` requests a close, while `x` forces it) the current client.
- `Super+Up`, `Super+Down`, `Super+Left`, `Super+Right`: Snaps a window to either the top-half, bottom-half, left-half or right-half of the screen.
- `Super+PageUp`, `Super+PageDown`: Increments or decrements the layer of this client.
- `Super+LClick`: When used on a client, this initiates moving the window. To set the window's position, let go of the left mouse button.
- `Super+RClick`: When used on a client, this initiates resising a window. To set the window's size, let go of the right mouse button.
- `Super+1` ... `Super+5` ... `Super+9`: These change the layer to the specified value (1, 5, or 9 respectively, in this example)

## Misc. ##

- `Super+LClick`: When used on the desktop background, this launches a new terminal.
- `Super+Escape`: Quits SmallWM.

Building
========
The Makefile contains everything you need to build and test SmallWM.

Namely, it has the following interesting targets:
 - `make smallwm-debug` compiles a version with symbols useful for debugging
 - `make smallwm-release` compiles an optimized version useful for daily use

Configuration
=============
With the rewrite in place, a few things about SmallWM can now be configured. A sample configuration file, located at `$HOME/.config/smallwm`, is shown below.

    [smallwm]
    shell=your-preferred-terminal
    desktops=42
    icon_width=75
    icon_height=20
    border_width=4
    [actions]
    stalonetray=stick,layer=9

The options in the `[smallwm]` section are:

 - The shell launched by `Super+LClick` (default: xterm)
 - The number of desktops (default: 5)
 - The width in pixels of icons (default: 75)
 - The height in pixels of icons (default: 20)
 - The width of the border of windows(default: 4)

The options in the `[actions]` section are covered next.

Actions
=======

X has the notion of an application "class" which is supposed to be a unique identifier for a window which belongs to a particular application. For example, there is a popular system tray called `stalonetray` which I use personally to manage status notifiers (like for NetworkManager, Dropbox, and the like).

The example given in the _Configuration_ section shows how to stick any window belonging to stalonetray and layer it on top of all other applictaion windows. Generally speaking, any number of these class actions can be chained together by separating them with commas.

The possibilities for a class action are:
 - `stick` makes a particular window stick to all the desktops
 - `maximize` maximizes that window
 - `Lx` sets the layer of the window to `x` where `x` is a number in the range 1 to 9
 - `snapleft`, `snapright`, `snaptop`, `snapbottom` snap the window to the relevant side of the screen

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
