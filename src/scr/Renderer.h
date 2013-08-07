#ifndef SCR_RENDERER_H
#define SCR_RENDERER_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <string>

#include "common/Texture.h"
#include "common/Vector2.h"

#include "GameWorld.h"
#include "Car.h"

class Renderer {
	public:
		Renderer(int width, int height);
		bool init();
		void drawFrame(const GameWorld* w);
		void cleanup();

	private:
		bool initGL();
		void loadTextures();
		void loadCarVBO();

		void drawCar(const Car* car);

		GLuint loadProgram(const char* vertfilename, const char* fragfilename,
				const std::vector<std::pair<int, std::string>>& attribbindings);
		GLuint loadShader(const char* src, GLenum type);
		std::string loadTextFile(const char* filename);

		Common::Texture* mCarTexture = nullptr;
		int mWidth;
		int mHeight;
		Common::Vector2 mCamPos;
		GLuint mCameraUniform;
		GLuint mOrientationUniform;
		GLuint mZoomUniform;
		GLuint mRightUniform;
		GLuint mTopUniform;
		GLuint mTextureUniform;
		GLuint mCarProgram;
		GLuint mCarVBO[3];
};

#endif

