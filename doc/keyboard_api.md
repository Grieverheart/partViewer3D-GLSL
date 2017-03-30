## `keyboard` module API

The `keyboard` module contains three tables containing values for all the keys you can press. Note that some keys may not be available on your keyboard or system. The three tables are the following:

* [action]()
* [mod]()
* [key]()

Values are accessed by their string key, i.e. `keyboard.key['m']` returns the value of key M.

## <a name="action"><a/> `action`

| Key string| Description |
| :--- | :--- |
|release | A key is released |
|press | A key is pressed |

## <a name="mod"><a/> `mod`

| Key string| Description |
| :--- | :--- |
| shift | Shift |
| ctrl | Control |
| alt  | Alt |
| super | Super |

## <a name="key"><a/> `key`

| Key string| Description |
| :--- | :--- |
| space | Space |
| ' |Single quote|
| , |Comma|
| - |Minus|
| . |Full stop|
| / |Slash|
| 0 |0|
| 1 |1|
| 2 |2|
| 3 |3|
| 4 |4|
| 5 |5|
| 6 |6|
| 7 |7|
| 8 |8|
| 9 |9|
| ; |Semicolumn|
| = |Equal|
| a |a|
| b |b|
| c |c|
| d |d|
| e |e|
| f |f|
| g |g|
| h |h|
| i |i|
| j |j|
| k |k|
| l |l|
| m |m|
| n |n|
| o |o|
| p |p|
| q |q|
| r |r|
| s |s|
| t |t|
| u |u|
| v |v|
| w |w|
| x |x|
| y |y|
| z |z|
| [ |Left square bracket|
| \ | Backslash |
| ] |Right square bracket|
| \` |Backtick|
| world_1 | Key that lack a clear US mapping. |
| world_2 | Key that lack a clear US mapping.|
| esc | Escape |
| enter | Enter|
| tab | Tab |
| backspace | Backspace|
| insert |Insert|
| delete |Delete|
| right | Right arrow |
| left | Left arrow |
| down | Down arrow|
| up | Up arrow |
| page_up |Page up|
| page_down |Page down|
| home | Home |
| end | End|
| caps_lock | Caps-lock |
| scroll_lock | Scroll-lock|
| num_lock | Num-lock |
| print_screen |Printscreen|
| pause |Pause|
| f1  |F1 |
| f2  |F2 |
| f3  |F3 |
| f4  |F4 |
| f5  |F5 |
| f6  |F6 |
| f7  |F7 |
| f8  |F8 |
| f9  |F9 |
| f10 |F10|
| f11 |F11|
| f12 |F12|
| f13 |F13|
| f14 |F14|
| f15 |F15|
| f16 |F16|
| f17 |F17|
| f18 |F18|
| f19 |F19|
| f20 |F20|
| f21 |F21|
| f22 |F22|
| f23 |F23|
| f24 |F24|
| f25 |F25|
| kp0 |Numpad 0|
| kp1 |Numpad 1|
| kp2 |Numpad 2|
| kp3 |Numpad 3|
| kp4 |Numpad 4|
| kp5 |Numpad 5|
| kp6 |Numpad 6|
| kp7 |Numpad 7|
| kp8 |Numpad 8|
| kp9 |Numpad 9|
| kp. |Numpad decimal|
| kp/ |Numpad division|
| kp* |Numpad multiplication|
| kp- |Numpad subtraction|
| kp+ |Numpad addition|
| kpenter |Numpad enter|
| kp= |Numpad equal|
| lshift | Left shift|
| lctrl |Left control|
| left_alt |Left alt|
| lsuper |Left super|
| rshift |Right shift|
| rcontrol |Right control|
| ralt |Right alt|
| rsuper |Right super|
| menu |Menu|
