#include "Renderer.h"

#include "common/Math.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>

Renderer::Renderer(int w, int h)
	: mWidth(w),
	mHeight(h),
	mCamPos(-1, -1)
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

	mCarProgram = loadProgram("share/car.vert", "share/car.frag", {{0, "aPosition"}, {1, "aTexcoord"}});
	mCameraUniform  = glGetUniformLocation(mCarProgram, "uCamera");
	mOrientationUniform  = glGetUniformLocation(mCarProgram, "uOrientation");
	mZoomUniform    = glGetUniformLocation(mCarProgram, "uZoom");
	mRightUniform   = glGetUniformLocation(mCarProgram, "uRight");
	mTopUniform     = glGetUniformLocation(mCarProgram, "uTop");
	mTextureUniform = glGetUniformLocation(mCarProgram, "sTexture");
	glUseProgram(mCarProgram);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	loadTextures();
	loadCarVBO();

	bool error = false;
	{
		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR) {
			fprintf(stderr, "GL error 0x%04x\n", err);
			error = true;
		}
	}

	return !error;
}

float Renderer::setZoom(float z)
{
	mZoom = Common::clamp(0.01f, z, 1.0f);
	return mZoom;
}

void Renderer::loadTextures()
{
	mCarTexture = new Common::Texture("share/car.png");
	mAsphaltTexture = new Common::Texture("share/asphalt.png");
	mGrassTexture = new Common::Texture("share/grass.png");
}

void Renderer::loadCarVBO()
{
	GLfloat vertices[] = {0.5f, 0.5f,
		0.5f, -0.5f,
		-0.5f, 0.5f,
		-0.5f, -0.5f};
	GLfloat texcoord[] = {1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 1.0f};
	GLushort indices[] = {0, 2, 1,
		1, 2, 3};

	glGenBuffers(3, mCarVBO);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, mCarVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// texcoord
	glBindBuffer(GL_ARRAY_BUFFER, mCarVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mCarVBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Renderer::loadGrassVBO(const Track* t)
{
	Common::Vector2 bl, tr;
	t->getLimits(bl, tr);
	GLfloat vertices[] = {tr.x, tr.y,
		tr.x, bl.y,
		bl.x, tr.y,
		bl.x, bl.y};
	const float texScale = 0.1f;
	GLfloat texcoord[] = {tr.x * texScale, bl.y * texScale,
		tr.x * texScale, tr.y * texScale,
		bl.x * texScale, bl.y * texScale,
		bl.x * texScale, tr.y * texScale};
	GLushort indices[] = {0, 2, 1,
		1, 2, 3};

	glGenBuffers(3, mGrassVBO);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, mGrassVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// texcoord
	glBindBuffer(GL_ARRAY_BUFFER, mGrassVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGrassVBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Renderer::loadTrackVBO(const Track* t)
{
	glGenBuffers(2, mTrackVBO);

	auto segments = t->getTrackSegments();

	assert(segments.size() == 1);

	auto seg = segments[0];
	auto triStrip = seg->getTriangleStrip();

	std::vector<GLfloat> vertexdata;
	std::vector<GLfloat> texcoorddata;
	for(const auto& t : triStrip) {
		vertexdata.push_back(t.x);
		vertexdata.push_back(t.y);
		texcoorddata.push_back(t.x * 0.1f);
		texcoorddata.push_back(t.y * 0.1f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, mTrackVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexdata.size() * sizeof(GLfloat), &vertexdata[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, mTrackVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, texcoorddata.size() * sizeof(GLfloat), &texcoorddata[0], GL_STATIC_DRAW);

	mTrackElemCount = triStrip.size();
}

void Renderer::cleanup()
{
	delete mAsphaltTexture;
	delete mCarTexture;
	delete mGrassTexture;
	glDeleteBuffers(2, mTrackVBO);
	glDeleteBuffers(3, mCarVBO);
	glDeleteBuffers(3, mGrassVBO);
	mTrackElemCount = 0;
}

void Renderer::drawFrame(const GameWorld* w)
{
	glViewport(0, 0, mWidth, mHeight);
	glUniform1f(mRightUniform, mWidth);
	glUniform1f(mTopUniform, mHeight);
	glUniform1i(mTextureUniform, 0);

	auto track = w->getTrack();
	if(!mTrackElemCount) {
		loadTrackVBO(track);
		loadGrassVBO(track);
	}

	auto car = w->getCar();
	auto carpos = car->getPosition();
	glUniform1f(mZoomUniform, mZoom);
	drawGrass();
	drawTrack();
	drawCar(car);

	mCamPos.x = carpos.x;
	mCamPos.y = carpos.y;

	{
		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR) {
			fprintf(stderr, "GL error 0x%04x\n", err);
		}
	}

}

void Renderer::drawCar(const Car* car)
{
	drawQuad(mCarVBO, mCarTexture, car->getPosition(),
			car->getOrientation());
}

void Renderer::drawTrack()
{
	glBindTexture(GL_TEXTURE_2D, mAsphaltTexture->getTexture());
	glUniform2f(mCameraUniform, -mCamPos.x, -mCamPos.y);
	glUniform1f(mOrientationUniform, 0.0f);

	glBindBuffer(GL_ARRAY_BUFFER, mTrackVBO[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, mTrackVBO[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, mTrackElemCount);
}

void Renderer::drawGrass()
{
	drawQuad(mGrassVBO, mGrassTexture, Common::Vector2(), 0.0f);
}

void Renderer::drawQuad(const GLuint vbo[3],
		const Common::Texture* texture, const Common::Vector2& pos,
		float orient)
{
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	glUniform2f(mCameraUniform, -mCamPos.x + pos.x, -mCamPos.y + pos.y);
	glUniform1f(mOrientationUniform, orient);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
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

	std::string vshader_src = loadTextFile(vertfilename);
	if(vshader_src.empty())
		return 0;

	std::string fshader_src = loadTextFile(fragfilename);
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


