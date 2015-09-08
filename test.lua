
print("Hello World!");
local a = {}
local b = {}
local c = {}
for i=1,9 do a[i] = 0.0 end
a[1] = 9.0
a[5] = 9.0
a[9] = 9.0
for i=1,9 do
    local particle = Particle()
    particle.size = 1.0
    particle.shape_id = 0
    particle.pos = vec3(i, i, i)
    particle.rot = vec4(0.0, 1.0, 0.0, 0.0)
    b[i] = particle
end
for i=1,9 do
    local sphere = Sphere()
    c[i] = sphere
end
scene_load(a, b, c)
scene_zoom(1.5)

