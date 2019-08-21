#include "texture.h"



glm::ivec2 Texture::get_size()
{
	return size;
}

bool Texture::is_uploaded()
{
	return texture != 0;
}



Texture::Texture()
{
	this->texture = 0;
}

Texture::Texture(GLuint tex)
{
	this->texture = tex;
}

Texture::Texture(std::string path)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		if (nrChannels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		spdlog::get("OSP")->error("Couldn't load texture");
	}
	stbi_image_free(data);

	size = glm::ivec2(width, height);
}

Texture::Texture(const Image& img)
{
	size = glm::ivec2(img.width, img.height);

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	// TODO: Allow the user to change this
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels.data());
}

void Texture::unload()
{
	glDeleteTextures(1, &texture);
	texture = 0;
}


Texture::~Texture()
{
	// TODO: Proper constructor unloading
	/*if (is_uploaded())
	{
		unload();
	}*/
}
