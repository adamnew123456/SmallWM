# ICCCM Overview #

This is an overview of the "Window Management" section of the 
[ICCCM](http://tronce.com/gui/x/icccm/sec-4.html), with comments on what
SmallWM supports.

## Clients Properties ##

 - `WM_NAME` is the title of a particular client, typically placed in window title bars. SmallWM uses this property through the `XFetchName` convience function.

 - `WM_ICON_NAME` is the text placed on a client's icon when the client is hidden. SmallWM uses this property through the `XGetIconName` convience function.

 - `WM_NORMAL_HINTS` is a group of properties related to client size and position - see [the ICCCM](http://tronche.com/gui/x/icccm/sec-4.html#s-4.1.2.3) for the specific flags. SmallWM does not use these now but may in the future.

 - `WM_HINTS` is a group of properties related to a window's icon and other misc. information; see [the ICCCM](http://tronche.com/gui/x/icccm/sec-4.html#s-4.1.2.4) for the specifics. SmallWM uses the information to set the icon pixmap to set the image for a window's icon via the `XGetWMHints` function.

 - `WM_CLASS` is a property which contains two NUL terminated strings, which specifies the instance and class for looking up application resources.  SmallWM does not use this property.

 - `WM_TRANSIENT_FOR` contains the ID of a window, and means that the window  is a "child" of another client, such as a popup or a splash screen. It differs from "override redirect" in that it does not block out its parent window. SmallWM does not use this property.

 - `WM_PROTOCOLS` is a group of three properties which have to do with direct communication with clients. See [the ICCCM](http://tronche.com/gui/x/icccm/sec-4.html#s-4.1.2.4) for the complete list.
   - `WM_TAKE_FOCUS` is used to convey whether or not a client's focus model is "active" (it actively moves the input focus around) or "passive" (it does not change the focus at all). SmallWM does not use this property.
   - `WM_SAVE_YOURSELF` is an obsolete protocol which is used to ask a client to save its state. SmallWM does not use this property.
   - `WM_DELETE_WINDOW` is used to ask a client to close its windows. SmallWM uses this property manually via the `XSendEvent` function.

 - `WM_COLORMAP_WINDOWS` is a window which needs a custom colormap. SmallWM does not use this property.

 - `WM_CLIENT_MACHINE` is the hostname of the machine which is running the client. SmallWM does not use this property.

## Window Manager Properties ##

 - `WM_STATE` is used to communicate to the client the state of the window - "withdrawn" (meaning closed), "normal" (meaning shown) and "iconic" (meaning hidden). SmallWM does not use this property.

 - `WM_ICON_SIZE` is used to communicate to the client the desired size of the window's icon. SmallWM uses this property - it creates this structure with `XAllocIconSize` and then sets it for each client via `XSetIconSize`.
