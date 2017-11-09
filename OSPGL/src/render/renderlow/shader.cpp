#include "shader.h"



void shader::load(std::string vfs, std::string ffs)
{
	log->info("Loading shaders, vertex: {}, fragment: {}", vfs, ffs);

	std::string vs, fs;
	vs = file_util::load_file(vfs);
	fs = file_util::load_file(ffs);

	const char* vs_cstr = vs.c_str();
	const char* fs_cstr = fs.c_str();

	// vertex

	int vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_cstr, NULL);
	glCompileShader(vertex);

	int success;
	char infoLog[512];

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
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
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	// check for linking errors
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) 
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		log->error("Error linking shaders ({},{}): {}", vfs, ffs, std::string(infoLog));
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);


}

shader::shader(std::string vertex, std::string pixel)
{
	this->log = spd::get("OSP");

	load(vertex, pixel);

}

shader::shader()
{
	this->log = spd::get("OSP");
}

shader::~shader()
{
}
