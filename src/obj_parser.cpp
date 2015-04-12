#include "include/obj_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <glm/glm.hpp>

//TODO: Use cstdio. We must have done this for the game engine code.
void parse_obj(const char *filename, std::vector<Vertex>& mesh, std::string shading_model){
	bool has_texture = false;
	
	// Hold the data from the object file
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec3> temp_vnormals;
	std::vector<unsigned short> normal_elements;
	std::vector<unsigned short> vertex_elements;
	
	std::ifstream file(filename, std::ios::in);
	if(!file){
		printf("Cannot open %s.\n", filename);
		exit(1);
	}
	std::string line;
	while(std::getline(file, line)){
		std::string flag;
		std::istringstream s(line);
		s >> flag;
		if(flag == "v"){
			glm::vec3 v;
			s >> v.x >> v.y >> v.z;
			temp_vertices.push_back(v);
		}
		else if(flag == "vn"){
			glm::vec3 v;
			s >> v.x >> v.y >> v.z;
			temp_vnormals.push_back(v);
		}
		else if(flag == "vt") has_texture = true;
		else if(flag == "f"){
			std::string tempstring[3];
			s >> tempstring[0] >> tempstring[1] >> tempstring[2];
			unsigned short f,fn;
			if(!has_texture){
				for(int i = 0; i < 3; i++){
					tempstring[i].replace(tempstring[i].find("//"),2," ");
					std::istringstream ss(tempstring[i]);
					ss >> f >> fn;
					f--;
					fn--;
					vertex_elements.push_back(f);
					normal_elements.push_back(fn);
				}
			}
			else{
				unsigned short vt;
				for(int i = 0; i < 3; i++){
					replace(tempstring[i].begin(), tempstring[i].end(), '/', ' ');
					std::istringstream ss(tempstring[i]);
					ss >> f >> vt >> fn;
					f--;
					fn--;
					vertex_elements.push_back(f);
					normal_elements.push_back(fn);
				}
			}
		}
	}
	// Make the vertex and normal lists
	int f_size = vertex_elements.size();
	
	if(shading_model == "smooth"){
		
		std::vector<glm::vec3> vertexnormals;
		int v_size = temp_vertices.size();
		
		for(int i = 0; i < v_size; i++){ //For every vertex
			glm::vec3 normal(0.0,0.0,0.0);
			for(int j = 0; j < f_size; j++){ //For every vertex of the triangles
				if(i == vertex_elements[j]){ //If they share the vertex
					normal+=temp_vnormals[normal_elements[j]];
				}
			}
			glm::normalize(normal);
			vertexnormals.push_back(normal);
		}
        for(int i = 0; i < f_size; i++){
            Vertex vertex(temp_vertices[vertex_elements[i]], vertexnormals[vertex_elements[i]]);
            mesh.push_back(vertex);
        }
	}
	else{
        for(int i = 0; i < f_size; i++){
            Vertex vertex(temp_vertices[vertex_elements[i]], temp_vnormals[normal_elements[i]]);
            mesh.push_back(vertex);
        }
	}
}
