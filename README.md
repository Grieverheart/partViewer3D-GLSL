## Introduction ##

PartViewer3D is a simple 3D scene viewer written using modern OpenGL deferred shading techniques. The main usage case of PartViewer3D is for viewing e.g. hard particle configurations. As different people have different needs, the viewer can be programmed using Lua scripts through the exposed API.

## Building ##

To build PartViewer3D, CMake (2.8.12+) is needed.

### Linux ###

1. Run `mkdir build && cd build` to create the build directory and move to it.

2. Run `cmake -DCMAKE_INSTALL_PREFIX:PATH="installation path" ..` if you want to install the application to a specific location, or `cmake ..` to install it to the main directory.

3. Finally run `make install` to build and install the application.

### Windows ###

TODO

### Mac OS X ###

You will need the OS X Command Line Tools to build the application. The command line tools, can be installed by issuing `xcode-select --install` on the terminal.

1. Run `mkdir build && cd build` to create the build directory and move to it.

2. Run `cmake -DCMAKE_INSTALL_PREFIX:PATH="installation path" ..` if you want to install the application to a specific location, or `cmake ..` to install it to the main directory.

3. Finally run `make install` to build and install the application.

## Usage ##

Running the application without any arguments will cause it to run the 'init.lua' script by default. A different script can be loaded using the `-f` flag, e.g. `./main -f my_init.lua`.

The application will look for any of the following specially named functions inside the script:

#### `OnInit(argv)`
This function will be called once when the application starts, and a table of the application's arguments, `argv` will be provided.

#### `OnFrame()`
This function will be called whenever a frame is drawn.

#### `OnKey(key, action, mods)`
This function will be called every time a key is pushed or released. The `key` argument holds the key value, the `action` argument lets us know if the key was pressed or released, and the `mods` takes the value of the modification key (e.g. ctrl, alt, etc.), if any was being pressed simultaneously. The API for the keyboard key values, actions, and mods, is exposed via the [keyboard module](doc/keyboard_api.md).

#### `OnMouseClick(x, y, button, action, mods)`
This function will be called every time a mouse button is pushed or released. The `x` and `y` arguments are the mouse pointer coordinates at the moment of the action. The `button` argument holds the button value, the `action` argument lets us know if the button was pressed or released, and the `mods` takes the value of the modification key (e.g. ctrl, alt, etc.), if any was being pressed simultaneously. The API for the mouse button values, and actions, is exposed via the [mouse module](doc/mouse_api.md).

#### `OnMouseMotion(x, y)`
This function will be called every time the mouse pointer moves. The `x` and `y` arguments are the current mouse pointer coordinates.

#### `OnMouseScroll(dz)`
This function will be called every time the mouse scroll is used. The `dz` argument is the offset of the scroll from its previous value.

An example initialization script is included in the repository, named 'init.lua'. The script expects to be passed a configuration file with the following format:

    n_part
    x y z phi a_x a_y a_z sid
    ...
    sid shape_info

where `n_part` is the total number of particles, `x`, `y`, and `z` are the particle coordinates, `phi` is the rotation angle, in degrees, around the axis with coordinates, `a_x`, `a_y`, and `a_z`. `sid` is the shape id for the particle. The shapes with respective ids, `sid`, are defined at the end of the file in `shape_info`, and can have either one of the following formats:

    sphere
    
or

    mesh path

where `path` is the path of a wavefront .obj type file.

## API Reference ##

For documentation on the exposed Lua API and the various modules take a look [here](doc/api.md).
