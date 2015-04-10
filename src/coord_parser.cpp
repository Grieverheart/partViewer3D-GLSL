#include <string>
#include <sstream>
#include <fstream>
#include "include/coord_parser.h"

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

	for(int n = 0; std::getline(file, line); ++n){
        s.str(line);
        s.seekg(0);
        glm::vec3& v = config.pos[n];
        glm::vec4& r = config.rot[n];
        //int& mesh_id = config.mesh_id[line_count - 2];
        for(int i = 0; i < 3; i++) s >> v[i];
        for(int i = 0; i < 4; i++) s >> r[i];
        //s >> mesh_id;
        r.x = glm::radians(r.x);
	}
    return config;
}
