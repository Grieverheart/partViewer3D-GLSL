#include <string>
#include <fstream>
#include <sstream>
#include "../include/coord_parser.h"

CCoordParser::CCoordParser(void){
}

CCoordParser::~CCoordParser(void){
}

void CCoordParser::parse(const char *filename){
	
	// Hold the data from the object file
	
	std::ifstream file(filename, std::ios::in);
	if(!file){
		std::cout << "Cannot open " << filename << "." << std::endl;
		exit(1);
	}
	std::cout << "Parsing " << filename << std::endl;
	
	std::string line;
	int line_count = 0;
	while(std::getline(file, line)){
		std::istringstream s(line);
		if(line_count == 0) s >> npart;
		else if(line_count == 1){
			for(int i = 0; i < 9; i++) s >> boxMatrix[i/3][i%3];
		}
		else{
			glm::vec3 v;
			glm::vec4 r;
			for(int i = 0; i < 3; i++)s >> v[i];
			for(int i = 0; i < 4; i++)s >> r[i];
			rotations.push_back(r);
			centers.push_back(v);
		}
		line_count++;
	}
	std::cout << "Done Parsing " << filename << std::endl;
	std::cout << "Found " << npart << " particles" << std::endl;
}