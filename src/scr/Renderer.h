#ifndef SCR_RENDERER_H
#define SCR_RENDERER_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <string>
#include <array>

#include "common/Texture.h"
#include "common/Vector2.h"
#include "common/TextRenderer.h"

#include "GameWorld.h"
#include "Car.h"
#include "Track.h"

struct TSRender {
	GLuint VBO[2];
	unsigned int ElemCount = 0;
};

struct DebugPointer {
	struct DebugPoint {
		GLuint VBO[3];
		Common::Vector2 Pos;
		Common::Color Color;
	};

	std::array<DebugPoint, 100> Points;
	unsigned int DebugPointIndex = 0;

	void init();
	void add(const Common::Vector2& pos, const Common::Color& col);
	void clear();
};

class Text {
	public:
		Text(boost::shared_ptr<Common::Texture> t);
		~Text();
		boost::shared_ptr<Common::Texture> texture;
		GLuint vbo[3];
};

class Renderer {
	public:
		Renderer(int width, int height);
		bool init();
		void drawFrame(const GameWorld* w);
		void cleanup();

		float setZoom(float z);
		void setSteering(float throttle, float brake, float steering);
		void toggleDebugDisplay();
		void updateDebug(const GameWorld* w);
		void setCamOrientation(bool o);

	private:
		bool initGL();
		void loadTextures();
		void loadCarVBO(const Car* car);
		void loadGrassVBO(const Track* t);
		void loadTrackVBO(const Track* t);
		void loadDebugVBO();

		void drawCar(const Car* car);
		void drawTrack();
		void drawGrass();
		void drawQuad(const GLuint vbo[3],
				const Common::Texture* texture, const Common::Vector2& pos,
				float orient, const Common::Color& col, bool isHud = false);
		void drawHUDQuad(const GLuint vbo[3],
				const Common::Texture* texture, const Common::Vector2& pos,
				float orient, const Common::Color& col);
		void drawTrackSegment(const TSRender& ts);
		void drawDebugPoints();
		void drawTexts(const GameWorld* w);
		Text* getText(const char* s);

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
		GLuint mColorUniform;
		GLuint mCarProgram;
		GLuint mCarVBO[3];
		GLuint mGrassVBO[3];

		std::vector<TSRender> mTrackSegments;
		DebugPointer mDebugPoints;
		Common::TextRenderer mTextRenderer;

		float mZoom = 0.01f;
		std::map<boost::shared_ptr<Common::Texture>, Text*> mTextCache;

		std::vector<std::string> mInfoTexts;
		float mScreenOrientation = 0.0f;
		bool mCamOrientation = false;
		bool mDebugDisplay = false;
};

#endif

