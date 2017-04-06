## `window` module API ##

The following functions are exposed in the `window` module:

* [size](#size)
* [position](#position)
* [set_size](#set_size)
* [set_position](#set_position)
* [set_title](#set_title)

## Function Reference:

#### <a name="size"></a> `size() -> int, int`
This function retrieves the size, in screen coordinates, of the client area of the application's window.

#### <a name="position"></a> `position() -> int, int`
This function retrieves the position, in screen coordinates, of the upper-left corner of the client area of the application's window.

#### <a name="set_size"></a> `set_size(int w, int h)`
This function sets the width to `w` and height to `h`, in screen coordinates, of the client area of the application's window.

#### <a name="set_position"></a> `set_position(int x, int y)`
This function sets the position to `x` and height to `y`, in screen coordinates, of the upper-left corner of the client area of the application's window.

#### <a name="set_title"></a> `set_title(string title)`
This function sets the window title to `title`, of the application's window.

