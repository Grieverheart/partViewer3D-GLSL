## `scene` module API

The scene module contains functionality for manipulating the scene as well as various objects for building a scene and displaying text on the screen. A scene is loaded to the rendered through the [`load`](#load) function. The scene module is also responsible for managing the rendering and its various modes. The renderer uses a deferred rendering technique in combination with the [Phong reflection model](https://en.wikipedia.org/wiki/Phong_reflection_model). [SSAO](https://en.wikipedia.org/wiki/Screen_space_ambient_occlusion) is used to add ambient occlusion to the scenes, which increases the sense of depth. As a consequence of the deferred rendering technique, we do not provide methods to render transparent particles. The renderer also offers a point rendering mode, and a selection rendering mode for each particle. In the point rendering mode, particles are rendered as points of a specific style specified by the appropriate scene functions. In the selection rendering mode, the outline of the selected particle is rendered in yellow.

The following classes are exposed to the Lua API:

* [Particle](#particle)
* [Vertex](#vertex)
* [Sphere](#sphere)
* [Mesh](#mesh)
* [TextProperties](#textproperties)

Along with the following functions:

* [load](#load)
* [draw_text](#draw_text)
* [save_snapshot](#save_snapshot)
* [get_fov_degrees](#get_fov_degrees)
* [set_fov_degrees](#set_fov_degrees)
* [rotate](#rotate)
* [select_particle](#select_particle)
* [is_particle_selected](#is_particle_selected)
* [clear_particle_selections](#clear_particle_selections)
* [hide_particle](#hide_particle)
* [unhide_particle](#unhide_particle)
* [set_particle_color](#set_particle_color)
* [toggle_particle_point_drawing](#toggle_particle_point_drawing)
* [set_point_radius](#set_point_radius)
* [set_point_outline_radius](#set_point_outline_radius)
* [set_point_color](#set_point_color)
* [set_point_outline_color](#set_point_outline_color)
* [toggle_box](#toggle_box)
* [set_box_line_width](#set_box_line_width)
* [enable_clip_plane](#enable_clip_plane)
* [disable_clip_plane](#disable_clip_plane)
* [set_clip_plane](#set_clip_plane)
* [set_ssao_blur](#set_ssao_blur)
* [is_ssao_blur_enabled](#is_ssao_blur_enabled)
* [set_ssao_num_samples](#set_ssao_num_samples)
* [get_ssao_num_samples](#get_ssao_num_samples)
* [set_ssao_radius](#set_ssao_radius)
* [get_ssao_radius](#get_ssao_radius)
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
* [get_light_direction](#get_light_direction)
* [set_light_direction](#set_light_direction)
* [get_background_color](#get_background_color)
* [set_background_color](#set_background_color)
* [get_sky_color](#get_sky_color)
* [set_sky_color](#set_sky_color)

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

#### `Particle() -> Particle`
Constructor.

#### `Particle(Particle) -> Particle`
Copy constructor.

## <a name="vertex"><a/> `Vertex`
The `Vertex` class is used for building a `Mesh`.

### Class Members:

#### `glm.vec3 coord`
Coordinate of the vertex.

#### `glm.vec3 normal`
Normal vector at vertex.

### Class Methods:

#### `Vertex(glm.vec3 coord, glm.vec3 normal) -> Vertex`
Construct a `Vertex` with coordinate `coord` and normal vector `normal`.

## <a name="sphere"><a/> `Sphere`
A sphere of unit radius. The renderer spheres in screen space instead of triangulating. This makes the rendering good looking and efficient.

### Class Methods:

#### `Sphere() -> Sphere`
Constructor.

## <a name="mesh"><a/> `Mesh`
A triangle mesh. Vertices are added through the `add_vertex` method. During rendering, vertices are interpreted as triangle vertices in groups of three, in the order that they were added.

### Class Methods:

#### `Mesh() -> Mesh`
Constructor.

#### `add_vertex(Vertex v)`
Add a vertex `v` to the mesh.

#### `get_vertex(int i) -> Vertex`
Get the `i`th vertex.

#### `get_num_vertices() -> int`
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

#### `TextProperties(string font, float width, glm.vec4 color, int x, int y) -> TextProperties`
Construct a `TextProperties` object with the font at the path given by `font`, font size `width`, color `color`, and at position given by `x` and `y`.

## Function Reference:

#### <a name="load"><a/>`load(table[float] box, table[Particle] particles, table[Sphere,Mesh] shapes)`
Load a scene for rendering. The `box` parameter should be a parallelepiped containing the scene. The parameter is expected to be a 3x3 matrix in row-major order, with columns given by the parallelepiped vectors along the three edge emanating from a common vertex. The `particles` table is the collection of particles that should be rendered, and `shapes` contains a list of all the shapes which `Particle`s refer to through their `shape_id`.

#### <a name="draw_text"><a/>`draw_text(string text, TextProperties props)`
Draw the text given by the string `text`, with text properties `props` on the screen.

#### <a name="save_snapshot"><a/>`save_snapshot(string path)`
Save a snapshot of the current window view in PNG format at path `path`.

#### <a name="get_fov_degrees"><a/> `get_fov_degrees() -> float`
Get the field of view angle in degrees.

#### <a name="set_fov_degrees"><a/> `set_fov_degrees(float fov)`
Set the field of view angle to `fov`, given in degrees. The field of view angle is clipped to [2, 90].

#### <a name="rotate"><a/>`rotate(float angle, glm.vec3 axis)`
Rotate the scene by `angle` radians around `axis`.

#### <a name="select_particle"><a/>`select_particle(int i)`
Set particle `i` as selected. This enables the selection drawing for that particle.

#### <a name="is_particle_selected"><a/>`is_particle_selected(int i)`
Check if particle `i` is selected.

#### <a name="clear_particle_selections"><a/>`clear_particle_selections()`
Clear all particle selections.

#### <a name="hide_particle"><a/>`hide_particle(int i)`
Hide particle `i`. Particle `i` is ignored by the renderer and thus not drawn.

#### <a name="unhide_particle"><a/>`unhide_particle(int i)`
Unhide particle `i` allowing the rendered to draw it again.

#### <a name="set_particle_color"><a/>`set_particle_color(int i, glm.vec3 color)`
Set the color of particle `i` to `color`.

#### <a name="toggle_particle_point_drawing"><a/>`toggle_particle_point_drawing(int i)`
Toggle the point drawing mode for particle `i`.

#### <a name="set_point_radius"><a/>`set_point_radius(float radius)`
Set the point radius for the point drawing mode to `radius`.

#### <a name="set_point_outline_radius"><a/>`set_point_outline_radius(float radius)`
Set the point outline radius for the point drawing mode to `radius`.

#### <a name="set_point_color"><a/>`set_point_color(glm.vec4 color)`
Set the point color for the point drawing mode to `color`.

#### <a name="set_point_outline_color"><a/>`set_point_outline_color(glm.vec4 color)`
Set the point outline color for the point drawing mode to `color`.

#### <a name="toggle_box"><a/>`toggle_box()`
Toggle drawing of the scene box.

#### <a name="set_box_line_width"><a/>`set_box_line_width(float width)`
Set the width of the line used to draw the scene box, to `width`.

#### <a name="enable_clip_plane"><a/>`enable_clip_plane()`
Enable the special clip plane which shows a cut through the scene.

#### <a name="disable_clip_plane"><a/>`disable_clip_plane()`
Disable the special clip plane which.

#### <a name="set_clip_plane"><a/>`set_clip_plane(glm.vec4 plane)`
Set the clip plane to `plane`. The x, y, and z components of `plane` represent the plane normal, while the w component represents the distance of the plane along its normal.

#### <a name="set_ssao_blur"><a/>`set_ssao_blur(bool val)`
Enable or disable the ssao blur, based on the value of `val`.

#### <a name="is_ssao_blur_enabled"><a/>`is_ssao_blur_enabled() -> bool`
Check if SSAO blur is enabled.

#### <a name="set_ssao_num_samples"><a/>`set_ssao_num_samples(int n_samples)`
Set the number of samples used for the SSAO effect. The default is 10. Increasing the number of samples, increases the quality in favor of performance.

#### <a name="get_ssao_num_samples"><a/>`get_ssao_num_samples() -> int`
Get the number of samples used in the SSAO effect.

#### <a name="set_ssao_radius"><a/>`set_ssao_radius(float radius)`
Set the radius used for the SSAO effect. The default is set to 2.

#### <a name="get_ssao_radius"><a/>`get_ssao_radius() -> float`
Get the radius used for the SSAO effect.

#### <a name="get_view_matrix"><a/>`get_view_matrix() -> glm.mat4`
Get the scene view matrix.

#### <a name="get_projection_matrix"><a/>`get_projection_matrix() -> glm.mat4`
Get the scene projection matrix.

#### <a name="get_model_matrix"><a/>`get_model_matrix() -> glm.mat4`
Get the scene model matrix.

#### <a name="get_view_position"><a/>`get_view_position() -> glm.vec3`
Get the position of the view.

#### <a name="set_view_position"><a/>`set_view_position(glm.vec3 pos)`
Set the position of the view to `pos`.

#### <a name="get_view_direction"><a/>`get_view_direction() -> glm.vec3`
Get the direction of the view, also known as 'look-at'.

#### <a name="set_view_direction"><a/>`set_view_direction(glm.vec3 dir)`
Get the direction of the view.

#### <a name="get_light_intensity"><a/>`get_light_intensity() -> float`
Get the light intensity.

#### <a name="set_light_intensity"><a/>`set_light_intensity(float intensity)`
Set the light intensity. In principle, the light intensity takes values in the range [0, 1], but higher values can also be used to oversaturate the scene.

#### <a name="get_light_ambient_intensity"><a/>`get_light_ambient_intensity() -> float`
Get the light 'ambient' intensity.

#### <a name="set_light_ambient_intensity"><a/>`set_light_ambient_intensity(float intensity)`
Set the light 'ambient' intensity to `intensity`.

#### <a name="get_light_diffuse_intensity"><a/>`get_light_diffuse_intensity() -> float`
Get the light 'diffuse' intensity.

#### <a name="set_light_diffuse_intensity"><a/>`set_light_diffuse_intensity(float intensity)`
Set the light 'diffuse' intensity to `intensity`.

#### <a name="get_light_specular_intensity"><a/>`get_light_specular_intensity() -> float`
Get the light 'specular' intensity.

#### <a name="set_light_specular_intensity"><a/>`set_light_specular_intensity(float intensity)`
Set the light 'specular' intensity, to `intensity`.

#### <a name="get_light_direction"><a/>`get_light_direction() -> glm.vec3`
Get the light direction.

#### <a name="set_light_direction"><a/>`set_light_direction(glm.vec3 dir)`
Set the light direction to `dir`.

#### <a name="get_background_color"><a/>`get_background_color() -> glm.vec3`
Get the color of the scene background.

#### <a name="set_background_color"><a/>`set_background_color(glm.vec3 color)`
Set the color of the scene background to `color`.

#### <a name="get_sky_color"><a/>`get_sky_color() -> glm.vec3`
Get the color of the sky.

#### <a name="set_sky_color"><a/>`set_sky_color(glm.vec3 color)`
Set the color of the sky to `color`. The sky color is used as the ambient color.

