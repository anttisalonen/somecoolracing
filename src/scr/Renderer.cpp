#include "Renderer.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>

Renderer::Renderer()
{
}

bool Renderer::init()
{
	GLenum glewerr;

	glewerr = glewInit();
	if (glewerr != GLEW_OK) {
		fprintf(stderr, "Unable to initialise GLEW.\n");
		return false;
	}
	if (!GLEW_VERSION_2_1) {
		fprintf(stderr, "OpenGL 2.1 not supported.\n");
		return false;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	//auto carProgram = loadProgram("share/car.vert", "share/car.frag", std::vector());
	//glUseProgram(carProgram);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	return true;
}

void Renderer::drawFrame(const GameWorld* w)
{
	auto car = w->getCar();
	std::cout << car->getPosition() << "\n";
}

GLuint Renderer::loadShader(const char* src, GLenum type)
{
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);

	if(shader == 0)
		return 0;

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled) {
		GLint infolen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
		if(infolen > 1) {
			char* infolog = (char*)malloc(infolen);
			glGetShaderInfoLog(shader, infolen, NULL, infolog);
			fprintf(stderr, "Error compiling %s shader: %s\n",
					type == GL_VERTEX_SHADER ?
					"vertex" : "fragment",
					infolog);
			free(infolog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

std::string Renderer::loadTextFile(const char* filename)
{
	std::ifstream in(filename);
	std::string contents((std::istreambuf_iterator<char>(in)), 
			    std::istreambuf_iterator<char>());
	return contents;
}

GLuint Renderer::loadProgram(const char* vertfilename, const char* fragfilename,
		const std::vector<std::pair<int, std::string>>& attribbindings)
{
	GLuint vshader;
	GLuint fshader;
	GLuint programobj;
	GLint linked;

	std::string vshader_src = loadTextFile("share/shader.vert");
	if(vshader_src.empty())
		return 0;

	std::string fshader_src = loadTextFile("share/shader.frag");
	if(fshader_src.empty()) {
		return 0;
	}

	vshader = loadShader(vshader_src.c_str(), GL_VERTEX_SHADER);
	fshader = loadShader(fshader_src.c_str(), GL_FRAGMENT_SHADER);

	if(!vshader || !fshader)
		return 0;

	programobj = glCreateProgram();
	if(!programobj)
		return 0;

	glAttachShader(programobj, vshader);
	glAttachShader(programobj, fshader);

	for(const auto& p : attribbindings) {
		glBindAttribLocation(programobj, p.first, p.second.c_str());
	}

	glLinkProgram(programobj);

	glGetProgramiv(programobj, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLint infolen = 0;
		glGetProgramiv(programobj, GL_INFO_LOG_LENGTH, &infolen);
		if(infolen > 1) {
			char* infolog = (char*)malloc(infolen);
			glGetProgramInfoLog(programobj, infolen, NULL, infolog);
			fprintf(stderr, "Error linking shader: %s\n", infolog);
			free(infolog);
		}
		glDeleteProgram(programobj);
		return 0;
	}

	return programobj;
}


