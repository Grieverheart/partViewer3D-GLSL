
local function load_obj(filepath)
    local vertices = {}
    local vertex_normals = {}
    local face_indices = {}

    local fp = io.open(filepath, "r")
    for line in fp:lines() do
        if string.sub(line, 1, 2) == 'v ' then
            local match = string.gmatch(line, '([%d.-]+)')
            table.insert(vertices, glm.vec3(tonumber(match()), tonumber(match()), tonumber(match())))
        elseif string.sub(line, 1, 2) == 'vn' then
            local match = string.gmatch(line, '([%d.-]+)')
            table.insert(vertex_normals, glm.vec3(tonumber(match()), tonumber(match()), tonumber(match())))
        elseif string.sub(line, 1, 2) == 'f ' then
            local face = {}
            for match in string.gmatch(line, '([%d/]+)') do
                local a, b, c = string.match(match, '(%d*)/(%d*)/(%d*)')
                table.insert(face, {tonumber(a), tonumber(b), tonumber(c)})
            end
            table.insert(face_indices, face)
        end
    end
    fp:close()

    local mesh = Mesh()

    for _, face in ipairs(face_indices) do
        for _, fi in ipairs(face) do
            mesh:add_vertex(Vertex(vertices[fi[1]], vertex_normals[fi[3]]))
        end
    end

    return mesh
end

local function load_scene(filepath)
    local fp = io.open(filepath, "r")

    local box = {}
    local particles = {}
    local shapes = {}

    local n_part = fp:read("*n")
    fp:read() --skip newline

    -- Read box data
    local idx = 1
    for match in string.gmatch(fp:read(), '([%d.-]+)') do
        box[idx] = tonumber(match)
        idx = idx + 1
    end

    -- Read particle data
    for pid = 1, n_part do
        local match = string.gmatch(fp:read(), '([%d.-]+)')
        local particle = Particle()
        particle.pos = glm.vec3(tonumber(match()), tonumber(match()), tonumber(match()))
        particle.rot = glm.vec4(tonumber(match()), tonumber(match()), tonumber(match()), tonumber(match()))
        particle.shape_id = tonumber(match())
        particle.size = 1.0
        particles[pid] = particle
    end
    -- Read shape data
    local shape_id, shape_info = string.match(fp:read(), '(%d+)%s+(.*)')
    shape_id = tonumber(shape_id) + 1
    local shape_type, shape_info = string.match(shape_info, '(.+)%s+(.*)')

    if string.lower(shape_type) == 'sphere' then
        shapes[shape_id] = Sphere()
    elseif string.lower(shape_type) == 'polyhedron' then
        shapes[shape_id] = load_obj("obj/"..shape_info..".obj")
    else
        fp:close()
        error("Unknown shape type, "..shape_type)
    end

    fp:close()

    scene.load(box, particles, shapes)

    return true
end

function OnInit(argv)
    scene.zoom(12.5)
    if argv[2] ~= nil then
        print("Loading "..argv[2])
        return load_scene(argv[2])
    else
        print("Configuration not provided!")
        return false
    end
end

projection_toggle = 0
clip_toggle = 0
function OnKey(key, action, mods)
    if key == 66 then -- B
        if action == 0 then
            scene.toggle_box()
        end
    elseif key == 79 then -- O
        if action == 0 then
            if projection_toggle == 0 then
                scene.set_projection_type('orthographic')
            else
                scene.set_projection_type('perspective')
            end
            projection_toggle = not projection_toggle;
        end
    elseif key == 67 then -- C
        if action == 0 then
            if clip_toggle == 0 then
                scene.enable_clip_plane();
            else
                scene.disable_clip_plane();
            end
            clip_toggle = not clip_toggle;
        end
    end
end

function OnMouseClick(x, y, button, action, mods)
    print(x, y)
    print(button, action, mods)
end

function OnMouseScroll(y)
    print(y)
end

--function OnMouseMotion(x, y)
--    print(x, y)
--end

--frame_id = 0
--function OnFrame()
--    frame_id = frame_id + 1
--    print(frame_id)
--end
