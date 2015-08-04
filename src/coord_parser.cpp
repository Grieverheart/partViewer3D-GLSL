#include <string>
#include <sstream>
#include <fstream>
#include "include/coord_parser.h"
#include "include/obj_parser.h"

//TODO: Use xml parsing
//TODO: Handle errors
SimConfig parse_config(const char* file_path){
	SimConfig config;
	std::string line;
    std::ifstream file(file_path, std::ios::in);

    std::istringstream s;

    std::getline(file, line);
    s.str(line);
    s >> config.n_part;
    config.particles = new Particle[config.n_part];
    std::getline(file, line);
    s.str(line);
    s.seekg(0);
    for(int i = 0; i < 9; i++) s >> config.box[i / 3][i % 3];

    int n_shapes = 0;
	for(int n = 0; n < config.n_part; ++n){
        std::getline(file, line);
        s.str(line);
        s.seekg(0);
        //TODO: Temporary
        config.particles[n].size = 1.0f;
        glm::vec3& v = config.particles[n].pos;
        glm::vec4& r = config.particles[n].rot;
        for(int i = 0; i < 3; i++) s >> v[i];
        for(int i = 0; i < 4; i++) s >> r[i];
        int& shape_id = config.particles[n].shape_id;
        s >> shape_id;
        if(shape_id > n_shapes) n_shapes = shape_id;
        r.x = glm::radians(r.x);
	}
    ++n_shapes;
    config.n_shapes = n_shapes;
    //TODO: Fix
    std::getline(file, line);
    s.str(line);
    s.seekg(0);
    int shape_id;
    std::string shape_type;
    std::string shape_name;
    s >> shape_id >> shape_type >> shape_name;
    config.shapes = new Shape[n_shapes];

    if(shape_type == "polyhedron"){
        config.shapes[shape_id].type = Shape::MESH;
        parse_obj(("obj/" + shape_name + ".obj").c_str(), config.shapes[shape_id].mesh, "flat");
    }
    else if(shape_type == "sphere"){
        config.shapes[shape_id].type = Shape::SPHERE;
    }
    else{
        config.shapes[shape_id].type = Shape::OTHER;
    }

    return config;
}
