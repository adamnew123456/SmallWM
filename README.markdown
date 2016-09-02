What is SmallWM?
================
SmallWM is an extended version of TinyWM, made for actual desktop use.

Improvements over TinyWM
========================
- Window Iconification
- Window Layering
- Click-To-Focus and Focus Cycling
- Moving/Resizing Placeholders
- Multiple Virtual Desktops 
- Window Snapping
- Class Actions

Controls
========

Note that these are the default controls. See the __Configuration__ for 
details on how to setup keybindings. The only non-configurable key bindings are the
ones that involve clicking the mouse, and the `Super+1` ... `Super+9` bindings.

## Desktops ##

- `Super+[`: Move a window to the previous desktop (`client-prev-desktop`).
- `Super+]`: Move a window to the next desktop (`client-next-desktop`).
- `Super+,`: Switch to the previous desktop (`prev-desktop`).
- `Super+.`: Switch to the next desktop (`next-desktop`).
- `Super+\`: Sticks/unsticks a window; a _stuck_ window is shown on all desktops (`toggle-stick`).

## Clients ##

- `Super+h`: Iconifies the current window (`iconify`).
- `Super+m`: Maximizes the current window (`maximize`).
- `Super+c`, Requests the current window to close (`request-close`).
- `Super+x`: Force-closes the current window (`force-close`).
- `Super+Up`, `Super+Down`, `Super+Left`, `Super+Right`: Snaps a window to either the top-half, bottom-half, left-half or right-half of the screen.
    - Actions: `snap-top`, `snap-bottom`, `snap-left`, `snap-right`
- `Super+Ctrl+Up`, `Super+Ctrl+Down`, ...: Moves a window to the screen in the
  relative direction of the arrow key.
    - Actions: `screen-top`, `screen-bottom`, `screen-left`, `screen-right`
- `Super+PageUp`, `Super+PageDown`: Increments or decrements the layer of this window.
    - Actions: `layer-above`, `layer-below`
- `Super+Home`, `Super+End`: Puts a window on the topmost or bottommost layer.
    - Actions: `layer-top`, `layer-bottom`
- `Super+Tab`: Focuses the next visible window in the focus list (`cycle-focus`).
- `Super+LClick`: Dragging the left mouse button starts moving a window - to place it, let go.
- `Super+RClick`: Dragging the right mouse button starts resizing a window - to scale it, let go.
- `Super+1` ... `Super+5` ... `Super+9`: These change the layer to the specified value (1, 5, or 9 respectively, in this example)

## Misc. ##

- `Super+LClick`: Left-clicking the root window launches a new terminal.
- `Super+Escape`: Quits SmallWM.

Building
========
As a dependency, you'll need to have access to the headers for Xlib and XRandR.
You should be able to easily obtain these via your package manager. You'll also
need a C++ compiler - GNU G++ and clang++ work well.

Other than the dependencies, the Makefile contains everything you need to build and test SmallWM.

- `make` compiles a version with symbols useful for debugging. Note that there
  is no optimized build - if you want an optimized version, open the Makefile and
  change `-g` to `-O3` in `CXXFLAGS`.

For modifying SmallWM, the other target that you should be aware of is `make check` 
which compiles everything but does no linking. This is useful for incremental building
to track compiler errors in source files.

Running
=======

Typically, the easiest place to put the `smallwm` binary is in `/usr/local/bin`.

If you want to run SmallWM from your login manager, you should put a file like the following in `/usr/share/xsessions/smallwm.desktop`:

    [Desktop Entry]
    Name=SmallWM
    Exec=/usr/local/bin/smallwm.sh
    Type=Application

Inside the script `/usr/local/bin/smallwm.sh`, you should enter something like 
the following:

    #!/bin/bash
    if [ -x $HOME/.smallwmrc ]; then
        $HOME/.smallwmrc &
    fi

    exec /usr/local/bin/smallwm

At this point, you may choose to write a `.smallwmrc` file to start any programs
you wish to run for the duration of your session. Note that SmallWM does not include
a process manager to handle session programs (unlike say, XFCE, which will restart
components like the panel or the desktop if they crash). I use a tool I wrote myself,
called [jobmon](http://github.com/adamnew123456/jobmon), to manage my system tray and
other programs, but you are free to choose whatever process manager you like, since
SmallWM doesn't care about it.

Configuration
=============

The C++ version follows a similar configuration file format to the original C 
version, but with some extended options. It should be placed at `$HOME/.config/smallwm`.

For example:

    [smallwm]
    shell=your-preferred-terminal
    desktops=42
    icon-width=75
    icon-height=20
    border-width=4
    icon-icons=0
    log-level=NOTICE
    hotkey-mode=focus
    [actions]
    stalonetray=stick,layer:9,xpos:90,ypos:0
    [keyboard]
    toggle-stick=asciitilde
    snap-top=w
    snap-bottom=s
    snap-left=a
    snap-right=!d

The options in the `[smallwm]` section are (in order):

- `shell` The shell launched by `Super+LClick` (default: xterm). This can be any syntax supported by /bin/sh.
- `desktops` The number of desktops (default: 5).
- `icon-width` The width in pixels of icons (default: 75).
- `icon-height` The height in pixels of icons (default: 20).
- `border-width` The width of the border of windows (default: 4).
- `icon-icons` Whether to (1) or not to (0) show application icons inside icon windows (default: 1).
- `log-level` The severity of logging messages to send to syslog. By
  default, this is `WARNING`. See `syslog(3)` for the other log levels.
- `hotkey-mode` What window to apply hotkeys like MINIMIZE to - this can be
  either `focus` (which means that the currently focused window is acted upon) or
  `mouse` (which means that the window under the cursor is acted upon).  The
  default is `mouse`.

Actions
=======

X has the notion of an application "class" which is supposed to be a unique 
identifier for a window which belongs to a particular application. For example, 
there is a popular system tray called `stalonetray` which I use personally to 
manage status notifiers (like for NetworkManager, Dropbox, and the like). A 
quick `xprop` of the window shows that its class name is `stalonetray`.

The example given in the _Configuration_ section shows how to stick any window 
belonging to stalonetray and layer it on top of all other application windows. 
Generally speaking, any number of these class actions can be chained together 
by separating them with commas.

The possibilities for a class action are:

- `stick` makes a particular window stick to all the desktops.
- `maximize` maximizes that window.
- `layer:x` sets the layer of the window to `x` where `x` is a number in the range 1 to 9; 9 is the highest layer, 1 is the lowest.
- `snap:left`, `snap:right`, `snap:top`, `snap:bottom` snap the window to the relevant side of the screen.
- `xpos:X` and `ypos:Y` set the relative position of the window on the screen. `X` and `Y` are decimals in the range 0 to 100,
  inclusive. For example, setting `xpos:50` puts the window's left edge in the middle of the screen (because `xpos:50` is
  equivalent to saying that the X position should be 50 percent of the screen's width).
- `nofocus` prevents SmallWM from automatically focusing windows of the given class. This is useful for windows like system trays,
  clocks, or other utility windows that you don't want to manipulate by accident.

Keyboard Bindings
=================

Keyboard bindings in SmallWM are almost entirely (except for `Super+1` ... `Super+9`) 
configurable. The mechanism isn't that sophisticated, so make sure that you
have a copy of `/usr/include/X11/keysymdef.h` or an equivalent file open.

In order to bind a key, you first have to know the name of the "keysym" that the
key uses. To do this, search `keysymdef.h` for your key - the keysym name is the first 
word after the `#define`. The text that you put in the configuration file is the
keysym name but with the leading `XK_` removed. For example, take 
`toggle-stick=asciitilde` in the example configuration file. This binds the `toggle-stick`
action to the `XK_asciitilde` keysym.

The following options can be set under the `[keyboard]` section to configure SmallWM's
keyboard bindings.

- `client-next-desktop`, `client-prev-desktop`
    - These bindings move the current window to either the next or previous desktop
- `next-desktop`, `prev-desktop`
    - These bindings move the view the next or previous desktop
- `toggle-stick`
    - This toggles the desktop stickiness of the current window
- `iconify`
    - This iconifies the current window
- `maximize`
    - This maximizes the current window
- `request-close`
    - This requests that the current window close, allowing the application to
      show save prompts and the like.
- `force-close`
    - This forces the current window to close. Only use this is an emergency -
      most applications will crash after you do this.
- `snap-top`, `snap-bottom`, `snap-left`, `snap-right`
    - Snaps the current window to the top, bottom, left or right half of the screen.
- `screen-left`, `screen-right`, `screen-top`, `screen-bottom`
    - Moves the current window to the screen to the left of, to the right of,
      above, or below the current screen it occupies.
- `layer-above`, `layer-below`
    - Moves the current window to the layer above or below its *current* layer.
- `layer-top`, `layer-bottom`
    - Moves the current window to the topmost or bottommost layer
- `cycle-focus`
    - Changes the focused window to the window next in the focus list.
- `exit-wm` 
    - Terminates SmallWM

Note the key binding given for `snap-right` in the example - the `!` that prefixes
the 'a' is used to indicate that this key bindings uses a secondary modifier key
(Control, by default). In order to activate `snap-left`, you need to press 
`Super+Ctrl+a` rather than just `Super+a`. Only the key bindings used to move windows
between screens use this by default.

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
