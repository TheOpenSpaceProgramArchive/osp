#pragma once

#include <string>
#include "../../util/file_util.h"
#include <glad/glad.h>

#include "../../util/defines.h"

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
	Important: All use[x] functions will use the shader!


*/
class Shader
{
public:

	int program;

	logger log;

	void load(std::string Vertex, std::string fragment);

	void use();

	void setf(std::string name, float val);
	void seti(std::string name, int val);
	void setb(std::string name, bool val);
	
	void setvec2(std::string name, glm::vec2 vec);
	void setvec3(std::string name, glm::vec3 vec);
	void setvec4(std::string name, glm::vec4 vec);

	void setmat4(std::string name, glm::mat4 mat);

	Shader(std::string Vertex, std::string fragment);
	Shader();
	~Shader();
};

