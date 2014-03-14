What is SmallWM?
================
SmallWM is an extended version of TinyWM, made for actual desktop use.
This is the newest version, rewritten in C++ - you can checkout the `legacy` branch if you want to use the old C version.

Improvements over TinyWM
========================
- Window Iconification
- Window Layering
- Click-To-Focus (Focus Is Indicated By Colored Borders)
- Moving/Resizing Placeholders
- Multiple Virtual Desktops (With Window Sticking)
- Window Snapping
- Class Actions

Controls
========

Note that these are the default controls - one of the improvements of the C++ 
port is that it SmallWM now supports configurable keybindings. See the __Configuration__
for details on how to setup keybindings. The only unconfigurable key bindings are the
ones that involve cliking the mouse, and the `Super+1` ... `Super+9` bindings.

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
As a dependency, you'll need to have acess to the headers for Xlib and XRandR.
You should be able to easily obtain these via your package manager. You'll also
need a C++ compiler which has C++11 support (although all it _really_ needs to
support is std::tuple). A recent version of GNU G++ or clang should compile
SmallWM with no problems.

Other than the dependencies, the Makefile contains everything you need to build and test SmallWM.

 - `make` compiles a version with symbols useful for debugging. Note that there is no optimized build - if you want open, open the Makefile and change `-g` to `-O3` in `CXXFLAGS`.

For modifying SmallWM, the other target that you should be aware of is `make check` 
which compiles everything but does no linking. This is useful for incremental building
to track compiler errors in source files.

Configuration
=============

The C++ version follows a similar configuration file format

    [smallwm]
    shell=your-preferred-terminal
    desktops=42
    icon_width=75
    icon_height=20
    border_width=4
    [actions]
    stalonetray=stick,layer:9
    [keyboard]
    toggle-stick=asciitilde
    snap-top=w
    snap-bottom=s
    snap-left=a
    snap-right=d

The options in the `[smallwm]` section are:

 - The shell launched by `Super+LClick` (default: xterm). This can be any syntax supported by /bin/sh.
 - The number of desktops (default: 5)
 - The width in pixels of icons (default: 75)
 - The height in pixels of icons (default: 20)
 - The width of the border of windows(default: 4)

The options in the `[actions]` section are covered next, and then the 
`[keyboard]` section after that.

Actions
=======

X has the notion of an application "class" which is supposed to be a unique 
identifier for a window which belongs to a particular application. For example, 
there is a popular system tray called `stalonetray` which I use personally to 
manage status notifiers (like for NetworkManager, Dropbox, and the like). A 
quick `xprop` of the window shows that its class name is `stalonetray`.

The example given in the _Configuration_ section shows how to stick any window 
belonging to stalonetray and layer it on top of all other applictaion windows. 
Generally speaking, any number of these class actions can be chained together 
by separating them with commas.

The possibilities for a class action are:
 - `stick` makes a particular window stick to all the desktops
 - `maximize` maximizes that window
 - `layer:x` sets the layer of the window to `x` where `x` is a number in the range 1 to 9
 - `snap:left`, `snap:right`, `snap:top`, `snap:bottom` snap the window to the relevant side of the screen

Keyboard Bindings
=================

Starting with the C++ rewrite, keyboard bindings in SmallWM are almost entirely 
(except for `Super+1` ... `Super+9`) configurable. The mechanism isn't that
sophisticated, however, so make sure that you have a copy of /usr/include/X11/keysymdef.h
or an equivalent file open.

In order to bind a key, you first have to know the name of the "keysym" that the
key uses. To do this, search keysymdef.h for your key - the keysym name is the first 
word after the `#define`. The text that you put in the configuration file is the
keysym name but with the leading `XK_` removed.

The following options can be set under the `[keyboard]` section to configure SmallWM's
keyboard bindings. Their meanings _should_ be fairly obvious.

 - `client-next-desktop`, `client-prev-desktop`
 - `next-desktop`, `prev-desktop`
 - `toggle-stick`
 - `iconify`
 - `maximize`
 - `request-close`
 - `force-close`
 - `snap-top`, `snap-bottom`, `snap-left`, `snap-right`
 - `layer-above`, `layer-below`, `layer-top`, `layer-bottom`
 - `exit-wm` 

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
