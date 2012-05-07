#include <string>
#include <fstream>
#include <sstream>
#include "../include/obj_parser.h"

CObjParser::CObjParser(void){
}

CObjParser::~CObjParser(void){
}

void CObjParser::parse(const char *filename, CMesh *mesh){
	
	// Hold the data from the object file
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec3> vnormals;
	std::vector<GLushort> normal_elements;
	std::vector<GLushort> elements;
	
	std::ifstream file(filename, std::ios::in);
	if(!file){
		std::cout << "Cannot open " << filename << "." << std::endl;
		exit(1);
	}
	
	std::string line;
	while(std::getline(file, line)){
		if(line.substr(0,2) == "v "){
			std::istringstream s(line.substr(2));
			glm::vec3 v;
			s >> v.x >> v.y >> v.z;
			temp_vertices.push_back(v);
		}
		else if(line.substr(0,2) == "vn"){
			std::istringstream s(line.substr(3));
			glm::vec3 v;
			s >> v.x >> v.y >> v.z;
			vnormals.push_back(v);
		}
		else if(line.substr(0,2) == "f "){
			std::istringstream s(line.substr(2));
			std::string tempstring[3];
			s >> tempstring[0] >> tempstring[1] >> tempstring[2];
			GLushort f[3],fn[3];
			for(int i = 0; i < 3; i++){
				tempstring[i].replace(tempstring[i].find("//"),2," ");
				std::istringstream ss(tempstring[i]);
				ss >> f[i] >> fn[i];
				f[i]--;
				fn[i]--;
				elements.push_back(f[i]);
				normal_elements.push_back(fn[i]);
			}
		}
	}
	// Make the vertex and normal lists
	std::vector<glm::vec3> vertexnormals;
	
	int v_size = temp_vertices.size();
	int f_size = elements.size();
	
	for(int i = 0; i < v_size; i++){ //For every vertex
		glm::vec3 normal(0.0,0.0,0.0);
		for(int j = 0; j < f_size; j++){ //For every vertex of the triangles
			if(i == elements[j]){ //If they share the vertex
				normal+=vnormals[normal_elements[j]];
			}
		}
		glm::normalize(normal);
		vertexnormals.push_back(normal);
	}
	mesh->data(temp_vertices,vertexnormals,elements);
}