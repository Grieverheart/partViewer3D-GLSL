
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

    ---- Generate a cone mesh

    --local mesh = Mesh()

    --local subdivision = 50
    --for i = 1, subdivision do
    --    local theta1 = 2.0 * math.pi * i / subdivision
    --    local theta2 = 2.0 * math.pi * (i + 1) / subdivision
    --    local x1 = math.sin(theta1)
    --    local z1 = math.cos(theta1)
    --    local x2 = math.sin(theta2)
    --    local z2 = math.cos(theta2)

    --    mesh:add_vertex(Vertex(glm.vec3(x1, 0.0, z1), glm.vec3(0.0, -1.0, 0.0)))
    --    mesh:add_vertex(Vertex(glm.vec3(0.0, 0.0, 0.0), glm.vec3(0.0, -1.0, 0.0)))
    --    mesh:add_vertex(Vertex(glm.vec3(x2, 0.0, z2), glm.vec3(0.0, -1.0, 0.0)))

    --    local theta3 = 2.0 * math.pi * (i - 0.5) / subdivision
    --    local theta4 = 2.0 * math.pi * (i + 0.5) / subdivision
    --    local theta5 = 2.0 * math.pi * (i + 1.5) / subdivision
    --    local normal1 = glm.normalize(glm.vec3(math.sin(math.pi * 0.25) * math.sin(theta3), math.cos(math.pi * 0.25), math.sin(math.pi * 0.25) * math.cos(theta3)))
    --    local normal2 = glm.normalize(glm.vec3(math.sin(math.pi * 0.25) * math.sin(theta4), math.cos(math.pi * 0.25), math.sin(math.pi * 0.25) * math.cos(theta4)))
    --    local normal3 = glm.normalize(glm.vec3(math.sin(math.pi * 0.25) * math.sin(theta5), math.cos(math.pi * 0.25), math.sin(math.pi * 0.25) * math.cos(theta5)))
    --    normal1 = glm.vec3(0.5) * (normal1 + normal2)
    --    normal3 = glm.vec3(0.5) * (normal2 + normal3)
    --    mesh:add_vertex(Vertex(glm.vec3(x2, 0.0, z2), normal3))
    --    mesh:add_vertex(Vertex(glm.vec3(0.0, 1.0, 0.0), normal2))
    --    mesh:add_vertex(Vertex(glm.vec3(x1, 0.0, z1), normal1))
    --end

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

    --pos = scene.get_view_position()
    --pos.x = pos.x + 12.0
    --scene.set_view_position(pos)

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

local projection_toggle = 0
local clip_toggle = 0
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

local function getArcballVec3(x, y)
    width, height = window.size()
    screen_min = height
    if width < screen_min then
        screen_min = width
    end

    P = glm.vec3((2.0 * x - width) / screen_min,
                 (height - 2.0 * y) / screen_min,
                 0.0);

    OP_squared = P.x * P.x + P.y * P.y;
    if OP_squared <= 0.5 then
        P.z = math.sqrt(1.0 - OP_squared)
    else
        P.z = 0.5 / math.sqrt(OP_squared)
        P = glm.normalize(P)
    end

    return P
end


local mouse = {
    last_mx = 0, last_my = 0,
    curr_mx = 0, curr_my = 0,
    -- TODO: Make these per button, i.e. mouse.button[btn].is_pressed
    dragging = false, pressed = false
}

function OnMouseClick(x, y, button, action, mods)
    if button == 0 then -- GLFW_MOUSE_BUTTON_LEFT
        if action == 1 then
            mouse.pressed = true
            mouse.last_mx = mouse.curr_mx
            mouse.last_my = mouse.curr_my
        else
            mouse.pressed = false
            if mouse.dragging == true then
                mouse.dragging = false
            elseif button == 0 then -- Normal clicks go here
                pid = scene.raytrace(x, y)
                if pid and (mods == 1) then
                    scene.select_particle(pid)
                elseif pid and (not scene.is_selected(pid)) then
                    scene.clear_selection()
                    scene.select_particle(pid)
                else
                    scene.clear_selection()
                end
            end
        end
    elseif button == 1 then
        pid = scene.raytrace(x, y)
        if pid and (action == 0) then
            scene.hide_particle(pid)
        end
    end
end

function OnMouseMotion(x, y)
    mouse.curr_mx = x
    mouse.curr_my = y

    if (mouse.curr_mx ~= mouse.last_mx) or (mouse.curr_my ~= mouse.last_my) then
        if (mouse.pressed == true) and (mouse.dragging == false) then
            mouse.dragging = true
        end
        if mouse.dragging == true then
            a = getArcballVec3(mouse.last_mx, mouse.last_my)
            b = getArcballVec3(mouse.curr_mx, mouse.curr_my)
            axis = glm.cross(a, b)

            dot = glm.dot(a, b)
            if dot > 1.0 then
                dot = 1.0
            end

            angle = math.acos(dot)

            scene.rotate(angle, axis)

            mouse.last_mx = mouse.curr_mx
            mouse.last_my = mouse.curr_my
        end
    end
end

function OnMouseScroll(y)
    scene.zoom(y)
end

--frame_id = 0
--function OnFrame()
--    frame_id = frame_id + 1
--    print(frame_id)
--end
