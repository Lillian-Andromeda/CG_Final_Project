#pragma once

#include <glm/glm.hpp>
#include "texture.h"

struct Material {

};

struct PhongMaterial : public Material {
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float ns;
	std::shared_ptr<Texture2D> mapKd;
};