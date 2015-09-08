--TODO: Actually implement parsing of file.
local function load_scene(filepath)
    local box = {}
    local particles = {}
    local shapes = {}
    for i=1,9 do box[i] = 0.0 end
    box[1] = 9.0
    box[5] = 9.0
    box[9] = 9.0
    for i=1,9 do
        local particle = Particle()
        particle.size = 1.0
        particle.shape_id = 0
        particle.pos = vec3(i, i, i)
        particle.rot = vec4(0.0, 1.0, 0.0, 0.0)
        particles[i] = particle
    end

    local triangle = Mesh()
    triangle:add_vertex(Vertex(vec3(-1.0, -0.5, -0.5), vec3(0.0, 0.0, 1.0)))
    triangle:add_vertex(Vertex(vec3( 1.0, -0.5, -0.5), vec3(0.0, 0.0, 1.0)))
    triangle:add_vertex(Vertex(vec3( 0.0,  0.8, -0.5), vec3(0.0, 0.0, 1.0)))
    shapes[1] = triangle

    scene_load(box, particles, shapes)

    return true
end

function OnInit(argv)
    for key, value in pairs(argv) do
        print(key, value)
    end
    print("Loading "..argv[2])
    scene_zoom(12.5)
    return load_scene(argv[2])
end

