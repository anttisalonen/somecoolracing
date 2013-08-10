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
#include "Track.h"

class Renderer {
	public:
		Renderer(int width, int height);
		bool init();
		void drawFrame(const GameWorld* w);
		void cleanup();

		float setZoom(float z);

	private:
		bool initGL();
		void loadTextures();
		void loadCarVBO();
		void loadGrassVBO(const Track* t);
		void loadTrackVBO(const Track* t);

		void drawCar(const Car* car);
		void drawTrack();
		void drawGrass();
		void drawQuad(const GLuint vbo[3],
				const Common::Texture* texture, const Common::Vector2& pos,
				float orient);

		GLuint loadProgram(const char* vertfilename, const char* fragfilename,
				const std::vector<std::pair<int, std::string>>& attribbindings);
		GLuint loadShader(const char* src, GLenum type);
		std::string loadTextFile(const char* filename);

		Common::Texture* mCarTexture = nullptr;
		Common::Texture* mAsphaltTexture = nullptr;
		Common::Texture* mGrassTexture = nullptr;
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
		GLuint mTrackVBO[2];
		unsigned int mTrackElemCount = 0;
		GLuint mGrassVBO[3];

		float mZoom = 0.01f;
};

#endif

