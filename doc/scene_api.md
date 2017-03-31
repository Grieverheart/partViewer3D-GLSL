## `scene` module API

The scene module contains functionality for manipulating the scene as well as various objects for building a scene and displaying text on the screen. A scene is loaded to the rendered through the [`load`](#load) function.

The following classes are exposed to the Lua API:

* [Particle](#particle)
* [Vertex](#vertex)
* [Sphere](#sphere)
* [Mesh](#mesh)
* [TextProperties](#textproperties)

Along with the following functions:

* [load](#load)
* [zoom](#zoom)
* [rotate](#rotate)
* [select_particle](#select_particle)
* [hide_particle](#hide_particle)
* [unhide_particle](#unhide_particle)
* [set_particle_color](#set_particle_color)
* [toggle_particle_point_drawing](#toggle_particle_point_drawing)
* [set_point_radius](#set_point_radius)
* [set_point_outline_radius](#set_point_outline_radius)
* [set_point_color](#set_point_color)
* [set_point_outline_color](#set_point_outline_color)
* [set_box_line_width](#set_box_line_width)
* [is_selected](#is_selected)
* [clear_selection](#clear_selection)
* [enable_clip_plane](#enable_clip_plane)
* [disable_clip_plane](#disable_clip_plane)
* [toggle_box](#toggle_box)
* [is_ssao_blur_enabled](#is_ssao_blur_enabled)
* [set_ssao_blur](#set_ssao_blur)
* [set_ssao_num_samples](#set_ssao_num_samples)
* [get_ssao_num_samples](#get_ssao_num_samples)
* [set_ssao_radius](#set_ssao_radius)
* [get_ssao_radius](#get_ssao_radius)
* [set_clip_plane](#set_clip_plane)
* [get_view_matrix](#get_view_matrix)
* [get_projection_matrix](#get_projection_matrix)
* [get_model_matrix](#get_model_matrix)
* [get_view_position](#get_view_position)
* [set_view_position](#set_view_position)
* [get_view_direction](#get_view_direction)
* [set_view_direction](#set_view_direction)
* [get_light_intensity](#get_light_intensity)
* [set_light_intensity](#set_light_intensity)
* [get_light_ambient_intensity](#get_light_ambient_intensity)
* [set_light_ambient_intensity](#set_light_ambient_intensity)
* [get_light_diffuse_intensity](#get_light_diffuse_intensity)
* [set_light_diffuse_intensity](#set_light_diffuse_intensity)
* [get_light_specular_intensity](#get_light_specular_intensity)
* [set_light_specular_intensity](#set_light_specular_intensity)
* [get_background_color](#get_background_color)
* [set_background_color](#set_background_color)
* [get_sky_color](#get_sky_color)
* [set_sky_color](#set_sky_color)
* [get_light_direction](#get_light_direction)
* [set_light_direction](#set_light_direction)
* [draw_text](#draw_text)
* [save_snapshot](#save_snapshot)

## Class Reference:

## <a name="particle"><a/> `Particle`
A `Particle` represents a shape instance.

### Class Members:

#### `glm.vec4 rot`
The particle rotation, represented in axis-angle format. The angle is expected to be in radians.

#### `glm.vec3 pos`
The particle position.

#### `float size`
The particle size.

#### `int shape_id`
The index of the particle's shape in the shapes table provided to the `load` function. The index should be zero-based (unlike Lua tables).

### Class Methods:

#### `Particle()`
Constructor.

#### `Particle(Particle)`
Copy constructor.

## <a name="vertex"><a/> `Vertex`
The `Vertex` class is used for building a `Mesh`.

### Class Members:

#### `glm.vec3 coord`
Coordinate of the vertex.

#### `glm.vec3 normal`
Normal vector at vertex.

### Class Methods:

#### `Vertex(glm.vec3 coord, glm.vec3 normal)`
Construct a `Vertex` with coordinate `coord` and normal vector `normal`.

## <a name="sphere"><a/> `Sphere`
A sphere of unit radius. The renderer spheres in screen space instead of triangulating. This makes the rendering good looking and efficient.

### Class Methods:

#### `Sphere()`
Constructor.

## <a name="mesh"><a/> `Mesh`
A triangle mesh. Vertices are added through the `add_vertex` method. During rendering, vertices are interpreted as triangle vertices in groups of three, in the order that they were added.

### Class Methods:

#### `Mesh()`
Constructor.

#### `add_vertex(Vertex v)`
Add a vertex `v` to the mesh.

#### `get_vertex(int i)`
Get the `i`th vertex.

#### `get_num_vertices()`
Get the current number of vertices added to the mesh.

## <a name="textproperties"><a/> `TextProperties`
A helper for defining text font properties which are passed to [`draw_text`](#draw_text) to draw text on the screen.

### Class Members:

#### `glm.vec4 color`
The text color.

#### `string font`
The font location path.

#### `float width`
The font size.

#### `int x`
The text position along the x screen axis.

#### `int y`
The text position along the y screen axis.

### Class Methods:

#### `TextProperties(string font, float width, glm.vec4 color, int x, int y)`
Construct a `TextProperties` object with the font at the path given by `font`, font size `width`, color `color`, and at position given by `x` and `y`.

## Function Reference:

#### <a name="load"><a/>`load(table[float] box, table[Particle] particles, table[Sphere,Mesh] shapes)`
Load a scene for rendering. The `box` parameter should be a parallelepiped containing the scene. The parameter is expected to be a 3x3 matrix in row-major order, with columns given by the parallelepiped vectors along the three edge emanating from a common vertex. The `particles` table is the collection of particles that should be rendered, and `shapes` contains a list of all the shapes which `Particle`s refer to through their `shape_id`.

#### <a name="draw_text"><a/>`draw_text(string text, TextProperties props)`

#### <a name="save_snapshot"><a/>`save_snapshot(string path)`

#### <a name="zoom"><a/>`zoom(float zoom_level)`
#### <a name="rotate"><a/>`rotate(float angle, glm.vec3 axis)`
#### <a name="select_particle"><a/>`select_particle(int i)`
#### <a name="hide_particle"><a/>`hide_particle(int i)`
#### <a name="unhide_particle"><a/>`unhide_particle(int i)`
#### <a name="set_particle_color"><a/>`set_particle_color(int i)`
#### <a name="toggle_particle_point_drawing"><a/>`toggle_particle_point_drawing(int i)`
#### <a name="set_point_radius"><a/>`set_point_radius(float radius)`
#### <a name="set_point_outline_radius"><a/>`set_point_outline_radius(float radius)`
#### <a name="set_point_color"><a/>`set_point_color(glm.vec4 color)`
#### <a name="set_point_outline_color"><a/>`set_point_outline_color(glm.vec4 color)`
#### <a name="set_box_line_width"><a/>`set_box_line_width(float width)`
#### <a name="is_selected"><a/>`is_selected(int i)`
#### <a name="clear_selection"><a/>`clear_selection()`
#### <a name="enable_clip_plane"><a/>`enable_clip_plane()`
#### <a name="disable_clip_plane"><a/>`disable_clip_plane()`
#### <a name="toggle_box"><a/>`toggle_box()`
#### <a name="is_ssao_blur_enabled"><a/>`is_ssao_blur_enabled()`
#### <a name="set_ssao_blur"><a/>`set_ssao_blur(bool)`
#### <a name="set_ssao_num_samples"><a/>`set_ssao_num_samples(int n_samples)`
#### <a name="get_ssao_num_samples"><a/>`get_ssao_num_samples()`
#### <a name="set_ssao_radius"><a/>`set_ssao_radius(float radius)`
#### <a name="get_ssao_radius"><a/>`get_ssao_radius()`
#### <a name="set_clip_plane"><a/>`set_clip_plane(glm.vec4)`
#### <a name="get_view_matrix"><a/>`get_view_matrix()`
#### <a name="get_projection_matrix"><a/>`get_projection_matrix()`
#### <a name="get_model_matrix"><a/>`get_model_matrix()`
#### <a name="get_view_position"><a/>`get_view_position()`
#### <a name="set_view_position"><a/>`set_view_position(glm.vec3 pos)`
#### <a name="get_view_direction"><a/>`get_view_direction()`
#### <a name="set_view_direction"><a/>`set_view_direction(glm.vec3 dir)`
#### <a name="get_light_intensity"><a/>`get_light_intensity()`
#### <a name="set_light_intensity"><a/>`set_light_intensity(float intensity)`
#### <a name="get_light_ambient_intensity"><a/>`get_light_ambient_intensity()`
#### <a name="set_light_ambient_intensity"><a/>`set_light_ambient_intensity(float intensity)`
#### <a name="get_light_diffuse_intensity"><a/>`get_light_diffuse_intensity()`
#### <a name="set_light_diffuse_intensity"><a/>`set_light_diffuse_intensity(float intensity)`
#### <a name="get_light_specular_intensity"><a/>`get_light_specular_intensity()`
#### <a name="set_light_specular_intensity"><a/>`set_light_specular_intensity(float intensity)`
#### <a name="get_background_color"><a/>`get_background_color()`
#### <a name="set_background_color"><a/>`set_background_color(glm.vec3 color)`
#### <a name="get_sky_color"><a/>`get_sky_color()`
#### <a name="set_sky_color"><a/>`set_sky_color(glm.vec3 color)`
#### <a name="get_light_direction"><a/>`get_light_direction()`
#### <a name="set_light_direction"><a/>`set_light_direction(glm.vec3 dir)`
