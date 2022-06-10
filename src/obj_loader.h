#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "base/vertex.h"

typedef struct{
	std::vector<glm::vec3> vertexPosition;
	std::vector<glm::vec2> vertexTexcoord;
	std::vector<glm::vec3> vertexNormal;
} attrib_t;

typedef struct{
	std::vector<glm::ivec3> positionIndex;
	std::vector<glm::ivec3> texcoordIndex;
	std::vector<glm::ivec3> normalIndex;
} index_t;

bool LoadObj(const std::string& _filepath, attrib_t& _attrib, index_t& _index); 
void SaveObj(attrib_t& _attrib, index_t& _index);
