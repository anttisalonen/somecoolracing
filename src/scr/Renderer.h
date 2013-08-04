#ifndef SCR_RENDERER_H
#define SCR_RENDERER_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <string>

#include "GameWorld.h"

class Renderer {
	public:
		Renderer();
		bool init();
		void drawFrame(const GameWorld* w);

	private:
		bool initGL();
		GLuint loadProgram(const char* vertfilename, const char* fragfilename,
				const std::vector<std::pair<int, std::string>>& attribbindings);
		GLuint loadShader(const char* src, GLenum type);
		std::string loadTextFile(const char* filename);
};

#endif

