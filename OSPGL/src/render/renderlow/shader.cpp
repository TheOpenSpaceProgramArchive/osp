#include "shader.h"



void Shader::load(std::string vfs, std::string ffs)
{
	log->info("Loading shaders, vertex: {}, fragment: {}", vfs, ffs);

	std::string vs, fs;
	vs = FileUtil::load_file(vfs);
	fs = FileUtil::load_file(ffs);

	const char* vs_cstr = vs.c_str();
	const char* fs_cstr = fs.c_str();

	// vertex

	int Vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(Vertex, 1, &vs_cstr, NULL);
	glCompileShader(Vertex);

	int success;
	char infoLog[512];

	glGetShaderiv(Vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(Vertex, 512, NULL, infoLog);
		log->error("Error compiling vertex shader ({}): {}", vfs, std::string(infoLog));
	}

	// fragment

	int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_cstr, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		log->error("Error compiling fragment shader ({}): {}", ffs, std::string(infoLog));
	}

	program = glCreateProgram();
	glAttachShader(program, Vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	// check for linking errors
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) 
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		log->error("Error linking shaders ({},{}): {}", vfs, ffs, std::string(infoLog));
	}

	glDeleteShader(Vertex);
	glDeleteShader(fragment);


}

void Shader::use()
{
	glUseProgram(program);
}

void Shader::setf(std::string name, float val)
{
	use();
	glUniform1f(glGetUniformLocation(program, name.c_str()), val);
}
void Shader::seti(std::string name, int val)
{
	use();
	glUniform1i(glGetUniformLocation(program, name.c_str()), val);
}

void Shader::setb(std::string name, bool val)
{
	use();
	glUniform1i(glGetUniformLocation(program, name.c_str()), val);
}

void Shader::setvec2(std::string name, glm::vec2 vec)
{
	use();
	glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &vec[0]);
}

void Shader::setvec3(std::string name, glm::vec3 vec)
{
	use();
	glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &vec[0]);
}

void Shader::setvec4(std::string name, glm::vec4 vec)
{
	use();
	glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &vec[0]);
}

void Shader::setmat4(std::string name, glm::mat4 mat)
{
	use();
	glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

Shader::Shader(std::string Vertex, std::string pixel)
{
	this->log = spd::get("OSP");

	load(Vertex, pixel);

}

Shader::Shader()
{
	this->log = spd::get("OSP");
}

Shader::~Shader()
{
}
