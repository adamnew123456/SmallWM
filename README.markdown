What is SmallWM?
================
SmallWM is an extended version of TinyWM, made for actual desktop use.
This is the newest version, rewritten in C++ - you can checkout the `legacy` 
branch if you want to use the old C version.

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
for details on how to setup keybindings. The only non-configurable key bindings are the
ones that involve clicking the mouse, and the `Super+1` ... `Super+9` bindings.

## Desktops ##

- `Super+[`: Move a client to the previous desktop (`client-prev-desktop`).
- `Super+]`: Move a client to the next desktop (`client-next-desktop`).
- `Super+,`: Swicth to the previous desktop (`prev-desktop`).
- `Super+.`: Switch to the next desktop (`next-desktop`).
- `Super+\`: Sticks/unsticks a window; a _stuck_ window is shown on all desktops (`toggle-stick`).

## Clients ##

- `Super+h`: Iconifies the current client (`iconify`).
- `Super+m`: Maximizes the current client (`maximize`).
- `Super+c`, Requests the current client to close (`request-close`).
- `Super+x`: Force-closes the current client (`force-close`).
- `Super+Up`, `Super+Down`, `Super+Left`, `Super+Right`: Snaps a window to either the top-half, bottom-half, left-half or right-half of the screen.
 - `snap-top`, `snap-bottom`, `snap-left`, `snap-right`
- `Super+PageUp`, `Super+PageDown`: Increments or decrements the layer of this client.
 - `layer-above`, `layer-below`
- `Super+Home`, `Super+End`: Puts a client on the topmost or bottommost layer.
 - `layer-top`, `layer-bottom`
- `Super+LClick`: Dragging the left mouse button starts moving a window - to place it, let go.
- `Super+RClick`: Dragging the right mouse button starts resizing a window - to scale it, let go.
- `Super+1` ... `Super+5` ... `Super+9`: These change the layer to the specified value (1, 5, or 9 respectively, in this example)

## Misc. ##

- `Super+LClick`: Left-clicking the root window launches a new terminal.
- `Super+Escape`: Quits SmallWM.

Building
========
As a dependency, you'll need to have acxess to the headers for Xlib and XRandR.
You should be able to easily obtain these via your package manager. You'll also
need a C++ compiler - GNU G++ and clang++ work well.

Other than the dependencies, the Makefile contains everything you need to build and test SmallWM.

 - `make` compiles a version with symbols useful for debugging. Note that there is no optimized build - if you want an optimized version, open the Makefile and change `-g` to `-O3` in `CXXFLAGS`.

For modifying SmallWM, the other target that you should be aware of is `make check` 
which compiles everything but does no linking. This is useful for incremental building
to track compiler errors in source files.

Configuration
=============

The C++ version follows a similar configuration file format to the original C 
version, but with some extended options. It should be placed at `$HOME/.config/smallwm.conf`.

For example:

    [smallwm]
    shell=your-preferred-terminal
    desktops=42
    icon-width=75
    icon-height=20
    border-width=4
    icon-icons=0
    log-level=NOTICE
    [actions]
    stalonetray=stick,layer:99
    [keyboard]
    toggle-stick=asciitilde
    snap-top=w
    snap-bottom=s
    snap-left=a
    snap-right=d

The options in the `[smallwm]` section are (in order):

 - The shell launched by `Super+LClick` (default: xterm). This can be any syntax supported by /bin/sh.
 - The number of desktops (default: 5)
 - The width in pixels of icons (default: 75)
 - The height in pixels of icons (default: 20)
 - The width of the border of windows (default: 4)
 - Whether to (1) or not to (0) show application icons inside icon windows. (default: 1)
 - The severity of logging messages to send to syslog. By default, this is `WARNING`. See `syslog(3)` for the other log levels.

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
belonging to stalonetray and layer it on top of all other applictation windows. 
Generally speaking, any number of these class actions can be chained together 
by separating them with commas.

The possibilities for a class action are:
 - `stick` makes a particular window stick to all the desktops
 - `maximize` maximizes that window
 - `layer:x` sets the layer of the window to `x` where `x` is a number in the range 10 to 99 (the default layer is 50).
 - `snap:left`, `snap:right`, `snap:top`, `snap:bottom` snap the window to the relevant side of the screen

Keyboard Bindings
=================

Starting with the C++ rewrite, keyboard bindings in SmallWM are almost entirely 
(except for `Super+1` ... `Super+9`) configurable. The mechanism isn't that
sophisticated, however, so make sure that you have a copy of `/usr/include/X11/keysymdef.h`
or an equivalent file open.

In order to bind a key, you first have to know the name of the "keysym" that the
key uses. To do this, search `keysymdef.h` for your key - the keysym name is the first 
word after the `#define`. The text that you put in the configuration file is the
keysym name but with the leading `XK_` removed. For example, take 
`toggle-stick=asciitilde` in the example configuration file. This binds the `toggle-stick`
action to the `XK_asciitilde` keysym.

The following options can be set under the `[keyboard]` section to configure SmallWM's
keyboard bindings. Their meanings _should_ be fairly obvious - if not, go to the
list of default bindings and see what each of these bindings mean.

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

The inih code, included as a part of SmallWM, is available under the New BSD License. See `inih/LICENSE.txt` for details.
