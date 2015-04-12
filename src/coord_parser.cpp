#include <string>
#include <sstream>
#include <fstream>
#include "include/coord_parser.h"
#include "include/obj_parser.h"

//TODO: use cstdio for parsing
SimConfig parse_config(const char* file_path){
	SimConfig config;
	std::string line;
    std::ifstream file(file_path, std::ios::in);

    std::istringstream s;

    std::getline(file, line);
    s.str(line);
    s >> config.n_part;
    config.pos = new glm::vec3[config.n_part];
    config.rot = new glm::vec4[config.n_part];
    config.mesh_id = new int[config.n_part];
    std::getline(file, line);
    s.str(line);
    s.seekg(0);
    for(int i = 0; i < 9; i++) s >> config.box[i / 3][i % 3];

    int n_meshes = 0;
	for(int n = 0; n < config.n_part; ++n){
        std::getline(file, line);
        s.str(line);
        s.seekg(0);
        glm::vec3& v = config.pos[n];
        glm::vec4& r = config.rot[n];
        int& mesh_id = config.mesh_id[n];
        for(int i = 0; i < 3; i++) s >> v[i];
        for(int i = 0; i < 4; i++) s >> r[i];
        s >> mesh_id;
        if(mesh_id > n_meshes) n_meshes = mesh_id;
        r.x = glm::radians(r.x);
	}
    ++n_meshes;
    config.n_meshes = n_meshes;
    //TODO: Fix
    std::getline(file, line);
    s.str(line);
    s.seekg(0);
    int mesh_id;
    std::string mesh_type;
    std::string mesh_name;
    s >> mesh_id >> mesh_type >> mesh_name;
    config.meshes = new std::vector<Vertex>[n_meshes];
	parse_obj(("obj/" + mesh_name + ".obj").c_str(), config.meshes[0], "flat");
    return config;
}
