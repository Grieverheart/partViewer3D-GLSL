## `glm` module API ##

The following classes are exposed to the Lua API:

* [vec3](#vec3)
* [vec4](#vec4)
* [mat3](#mat3)
* [mat4](#mat4)

Along with the following functions:

* [cross(vec3, vec3)](#cross)
* [dot(vec3, vec3)](#dot)
* [normalize(vec3)](#normalize)
* [column(mat3, int)](#column-m3)
* [row(mat3, int)](#row-m3)
* [inverse(mat3)](#inverse-m3)
* [transpose(mat3)](#transpose-m3)
* [column(mat4, int)](#column-m4)
* [row(mat4, int)](#row-m4)
* [inverse(mat4)](#inverse-m4)
* [transpose(mat4)](#transpose-m4)

## Class Reference:

## <a name="vec3"><a/> `vec3`

A three-component vector of type `float`.

### Class Members:

`float x`, `float y`, `float z`

### Class Methods:

#### `vec3(float x, float y, float z)`
Construct a `vec3` with components `x`, `y`, and `z`.

#### `vec3(float s)`
Construct a `vec3` with components all its components initialized to `s`.

#### `vec3(vec3 v)`
Copy construct a `vec3` from the vector `v`.

#### `vec3 + vec3`
Addition between vectors.

#### `vec3 - vec3`
Subtraction between vectors.

#### `vec3 * vec3`
Component-wise multiplication between vectors.

#### `vec3 + float`, `float + vec3`
Addition between a `vec3` and a float. Is equivalent to `vec3 + vec3(float)`, `vec3(float) + vec3` respectively.

#### `vec3 - float`, `float - vec3`
Subtraction between a `vec3` and a float. Is equivalent to `vec3 - vec3(float)`, `vec3(float) - vec3` respectively.

#### `vec3 * float`, `float * vec3`
Multiplication between a `vec3` and a float. Is equivalent to `vec3 * vec3(float)`, `vec3(float) * vec3` respectively.

## <a name="vec4"><a/> `vec4`

A four-component vector of type `float`.

### Class Members:

`float x`, `float y`, `float z`, `float w`

### Class Methods:

#### `vec4(float x, float y, float z, float w)`
Construct a `vec4` with components `x`, `y`, `z`, and `w`.

#### `vec4(float s)`
Construct a `vec4` with components all its components initialized to `s`.

#### `vec4(vec4 v)`
Copy construct a `vec4` from the vector `v`.

#### `vec4 + vec4`
Addition between vectors.

#### `vec4 - vec4`
Subtraction between vectors.

#### `vec4 * vec4`
Multiplication between vectors.

#### `vec4 + float`, `float + vec4`
Addition between a `vec4` and a float. Is equivalent to `vec4 + vec4(float)`, `vec4(float) + vec4` respectively.

#### `vec4 - float`, `float - vec4`
Subtraction between a `vec4` and a float. Is equivalent to `vec4 - vec4(float)`, `vec4(float) - vec4` respectively.

#### `vec4 * float`, `float * vec4`
Multiplication between a `vec4` and a float. Is equivalent to `vec4 * vec4(float)`, `vec4(float) * vec4` respectively.

## <a name="mat3"><a/> `mat3`

A 3x3 matrix of type `float`. The elements are stored in column-major order.

### Class Methods:

#### `mat3(float s)`
Construct a `mat3` with components all its components initialized to `s`.

#### `mat3(mat3 m)`
Copy construct a `mat3` from the vector `m`.

#### `mat3(mat4 m)`
Construct a `mat3` from the upper left 3x3 part of `mat4`.

#### `mat3 + mat3`
Matrix addition.

#### `mat3 - mat3`
Matrix subtraction.

#### `mat3 * mat3`
Matrix multiplication.

#### `mat3 * vec3`
Matrix vector multiplication.

#### `mat3 + float`, `float + mat3`
Addition between a `mat3` and a float. Is equivalent to `mat3 + mat3(float)`, `mat3(float) + mat3` respectively.

#### `mat3 - float`, `float - mat3`
Subtraction between a `mat3` and a float. Is equivalent to `mat3 - mat3(float)`, `mat3(float) - mat3` respectively.

#### `mat3 * float`, `float * mat3`
Multiplication between a `mat3` and a float. Is equivalent to `mat3 * mat3(float)`, `mat3(float) * mat3` respectively.


## <a name="mat4"><a/> `mat4`

A 4x4 matrix of type `float`. The elements are stored in column-major order.

## Function Reference:

#### <a name="cross"><a/> `cross(vec3 va, vec3 vb)`
Vector cross product between vectors `va` and `vb`.

#### <a name="dot"><a/> `dot(vec3 va, vec3 vb)`
Dot product between vectors `va` and `vb`.

#### <a name="normalize"><a/> `normalize(vec3 v)`
Returns the vector `v` normalized by its length.

#### <a name="column-m3"><a/> `column(mat3 m, int i)`
Returns the column `i` of the `mat3` matrix `m`.

#### <a name="row-m3"><a/> `row(mat3 m, int i)`
Returns the row `i` of the `mat3` matrix `m`.

#### <a name="inverse-m3"><a/> `inverse(mat3 m)`
Returns the inverse of the `mat3` matrix `m`.

#### <a name="transpose-m3"><a/> `transpose(mat3 m)`
Returns the transpose of the `mat3` matrix `m`.

#### <a name="column-m4"><a/> `column(mat4 m, int i)`
Returns the column `i` of the `mat4` matrix `m`.

#### <a name="row-m4"><a/> `row(mat4 m, int i)`
Returns the row `i` of the `mat4` matrix `m`.

#### <a name="inverse-m4"><a/> `inverse(mat4 m)`
Returns the inverse of the `mat4` matrix `m`.

#### <a name="transpose-m4"><a/> `transpose(mat4 m)`
Returns the transpose of the `mat4` matrix `m`.
