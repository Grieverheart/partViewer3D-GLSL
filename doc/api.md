## Lua API Reference ##

The Lua API consists out of several modules. For linear algebra operations, we provide bindings to the C++ library, [GLM](http://glm.g-truc.net/0.9.8/index.html), in the `glm` module. The `window` module is used to get information about and manipulate the application's window. Various values related to the keyboard and mouse, are stored in the [keyboard](keyboard_api.md) and [mouse](mouse_api.md) modules. Finally, and most importantly, the [scene](scene_api.md) module contains functionality for manipulating the scene as well as various objects for building a scene and displaying text on the screen. Note that if you find Lua to be too slow for some operations, [you can write your own modules in C](https://www.lua.org/pil/26.2.html).

### Modules ###

* [glm](glm_api.md)
* [scene](scene_api.md)
* [window](window_api.md)
* [keyboard](keyboard_api.md)
* [mouse](mouse_api.md)
