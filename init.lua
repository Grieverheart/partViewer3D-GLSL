
local function load_obj(filepath)
    print("Loading "..tostring(filepath)..".")
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

    local mesh = scene.Mesh()

    for _, face in ipairs(face_indices) do
        for _, fi in ipairs(face) do
            mesh:add_vertex(vertices[fi[1]], vertex_normals[fi[3]])
        end
    end

    return mesh
end

local function load_scene(filepath)
    local fp = io.open(filepath, "r")

    local box = {}
    local particles = {}
    local shapes = {}

    n_part = fp:read("*n") --NOTE: global!
    fp:read() --skip newline

    -- Read box data
    local idx = 1
    for match in string.gmatch(fp:read(), '(%S+)') do
        box[idx] = tonumber(match)
        idx = idx + 1
    end

    -- Read particle data
    for pid = 1, n_part do
        local match = string.gmatch(fp:read(), '(%S+)')
        local particle = scene.Particle()
        particle.pos = glm.vec3(tonumber(match()), tonumber(match()), tonumber(match()))
        particle.rot = glm.vec4(math.pi * tonumber(match()) / 180.0, tonumber(match()), tonumber(match()), tonumber(match()))
        particle.shape_id = tonumber(match())
        particle.size = 1.0
        particles[pid] = particle
    end

    -- Read shape data
    for line in fp:lines() do
        local shape_id, shape_info = string.match(line, '(%d+)%s+(.*)')
        shape_id = tonumber(shape_id) + 1
        local shape_type, shape_info = string.match(shape_info, '([0-z]+)%s*(.*)')

        if string.lower(shape_type) == 'sphere' then
            shapes[shape_id] = scene.Sphere()
        elseif string.lower(shape_type) == 'mesh' then
            shapes[shape_id] = load_obj(shape_info)
        else
            fp:close()
            error("Unknown shape type, "..shape_type)
        end
    end

    fp:close()

    scene.load(box, particles, shapes)

    return true
end

local start_time = 0
function OnInit(argv)
    start_time = os.clock()
    scene.set_fov_degrees(47.5)

    if #argv > 0 and argv[1] ~= nil then
        print("Loading "..argv[1])
        return load_scene(argv[1])
    end

    print("Configuration not provided!")
    return false
end

local projection_toggle = false
local clip_toggle = false
local draw_fps = false
function OnKey(key, action, mods)
    if key == keyboard.key['b'] then
        if action == keyboard.action.release then
            scene.toggle_box()
        end
    elseif key == keyboard.key['o'] then
        if action == keyboard.action.release then
            if projection_toggle == false then
                scene.set_projection_type('orthographic')
            else
                scene.set_projection_type('perspective')
            end
            projection_toggle = not projection_toggle;
        end
    elseif key == keyboard.key['p'] then
        if action == keyboard.action.release then
            for pid = 0, n_part - 1 do
                scene.toggle_particle_point_drawing(pid)
            end
        end
    elseif key == keyboard.key['c'] then
        if action == keyboard.action.release then
            if clip_toggle == false then
                scene.enable_clip_plane();
            else
                scene.disable_clip_plane();
            end
            clip_toggle = not clip_toggle;
        end
    elseif key == keyboard.key['r'] and mods == keyboard.mod['ctrl'] then
        if action == keyboard.action.release then
            draw_fps = not draw_fps
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


local mouse_ = {
    last_mx = 0, last_my = 0,
    curr_mx = 0, curr_my = 0,
    -- TODO: Make these per button, i.e. mouse_.button[btn].is_pressed
    dragging = false, pressed = false
}

function OnMouseClick(x, y, button, action, mods)
    if button == mouse.button.left then
        if action == mouse.action.press then
            mouse_.pressed = true
            mouse_.last_mx = mouse_.curr_mx
            mouse_.last_my = mouse_.curr_my
        else
            mouse_.pressed = false
            if mouse_.dragging == true then
                mouse_.dragging = false
            elseif button == mouse.button.left then -- Normal clicks go here
                pid = scene.raytrace(x, y)
                if pid and (mods == keyboard.mod.shift) then
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
        if pid and (action == mouse.action.release) then
            scene.hide_particle(pid)
        end
    end
end

function OnMouseMotion(x, y)
    mouse_.curr_mx = x
    mouse_.curr_my = y

    if (mouse_.curr_mx ~= mouse_.last_mx) or (mouse_.curr_my ~= mouse_.last_my) then
        if (mouse_.pressed == true) and (mouse_.dragging == false) then
            mouse_.dragging = true
        end
        if mouse_.dragging == true then
            a = getArcballVec3(mouse_.last_mx, mouse_.last_my)
            b = getArcballVec3(mouse_.curr_mx, mouse_.curr_my)
            axis = glm.cross(a, b)

            dot = glm.dot(a, b)
            if dot > 1.0 then
                dot = 1.0
            end

            angle = math.acos(dot)

            scene.rotate(angle, axis)

            mouse_.last_mx = mouse_.curr_mx
            mouse_.last_my = mouse_.curr_my
        end
    end
end

function OnMouseScroll(dy)
    scene.set_fov_degrees(scene.get_fov_degrees() - dy)
end

local fps = 0
local frames = 0
function OnFrame()
    frames = frames + 1
    props = scene.TextProperties("/usr/share/fonts/TTF/Inconsolata-Regular.ttf", 24, glm.vec4(1.0, 1.0, 0.0, 1.0), 0, 20)
    local end_time = os.clock()
    if end_time - start_time > 0.2 then
        fps = math.floor(frames / (end_time - start_time))
        start_time = end_time
        frames = 0
    end
    if draw_fps == true then
        scene.draw_text(tostring(fps), props)
    end
end
