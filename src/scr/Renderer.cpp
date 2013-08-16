#include "Renderer.h"

#include "common/Math.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>


using namespace Common;

void DebugPointer::init()
{
	for(auto& p : Points) {
		p.VBO[0] = 0;
		p.VBO[1] = 0;
		p.VBO[2] = 0;
	}
}

void DebugPointer::add(const Common::Vector2& pos, const Common::Color& col)
{
	unsigned int i = DebugPointIndex++;
	DebugPointIndex = DebugPointIndex % Points.size();

	DebugPoint& p = Points[i];
	p.Pos = pos;
	p.Color = col;

	if(p.VBO[0] == 0) {
		glGenBuffers(3, p.VBO);

		GLfloat vertices[] = {0.1f, 0.1f,
			0.1f, -0.1f,
			-0.1f, 0.1f,
			-0.1f, -0.1f};
		GLfloat texcoord[] = {1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			0.0f, 1.0f};
		GLushort indices[] = {0, 2, 1,
			1, 2, 3};

		// vertices
		glBindBuffer(GL_ARRAY_BUFFER, p.VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// texcoord
		glBindBuffer(GL_ARRAY_BUFFER, p.VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);

		// indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p.VBO[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}
}


Text::Text(boost::shared_ptr<Common::Texture> t)
	: texture(t)
{
	glGenBuffers(3, vbo);

	float w = texture->getWidth();
	float h = texture->getHeight();

	GLfloat vertices[] = {w, h,
		w, 0.0f,
		0.0f, h,
		0.0f, 0.0f};
	GLfloat texcoord[] = {1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 1.0f};
	GLushort indices[] = {0, 2, 1,
		1, 2, 3};

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

Text::~Text()
{
	glDeleteBuffers(3, vbo);
}


Renderer::Renderer(int w, int h)
	: mWidth(w),
	mHeight(h),
	mCamPos(-1, -1),
	mTextRenderer("share/DejaVuSans.ttf", 24)
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
	mColorUniform   = glGetUniformLocation(mCarProgram, "uColor");
	glUseProgram(mCarProgram);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	loadTextures();
	loadDebugVBO();

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

void Renderer::setSteering(float throttle, float brake, float steering)
{
	mInfoTexts.clear();
	char buf[128];
	sprintf(buf, "Throttle: %d %%", (int)(throttle * 100));
	mInfoTexts.push_back(std::string(buf));
	sprintf(buf, "Brake: %d %%", (int)(brake * 100));
	mInfoTexts.push_back(std::string(buf));
	sprintf(buf, "Steering: %d %%", (int)(steering * 100));
	mInfoTexts.push_back(std::string(buf));
}

void Renderer::loadTextures()
{
	mCarTexture = new Common::Texture("share/car.png");
	mAsphaltTexture = new Common::Texture("share/asphalt.png");
	mGrassTexture = new Common::Texture("share/grass.png");
}

void Renderer::loadCarVBO(const Car* car)
{
	float w = car->getWidth();
	GLfloat vertices[] = {w, w,
		w, -w,
		-w, w,
		-w, -w};
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
	const float texScale = 0.01f;
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
	auto segments = t->getTrackSegments();

	for(const auto& seg : segments) {
		TSRender r;

		glGenBuffers(2, r.VBO);

		auto triStrip = seg->getTriangleStrip();

		std::vector<GLfloat> vertexdata;
		std::vector<GLfloat> texcoorddata;
		for(const auto& t : triStrip) {
			vertexdata.push_back(t.x);
			vertexdata.push_back(t.y);
			texcoorddata.push_back(t.x * 0.01f);
			texcoorddata.push_back(t.y * 0.01f);
		}

		glBindBuffer(GL_ARRAY_BUFFER, r.VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, vertexdata.size() * sizeof(GLfloat), &vertexdata[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, r.VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, texcoorddata.size() * sizeof(GLfloat), &texcoorddata[0], GL_STATIC_DRAW);

		r.ElemCount = triStrip.size();

		mTrackSegments.push_back(r);
	}
}

void Renderer::loadDebugVBO()
{
	mDebugPoints.init();
}

void Renderer::cleanup()
{
	delete mAsphaltTexture;
	delete mCarTexture;
	delete mGrassTexture;
	for(const auto& p : mDebugPoints.Points) {
		glDeleteBuffers(3, p.VBO);
	}
	for(const auto& t : mTrackSegments) {
		glDeleteBuffers(2, t.VBO);
	}
	glDeleteBuffers(3, mCarVBO);
	glDeleteBuffers(3, mGrassVBO);

	for(auto p : mTextCache) {
		delete p.second;
	}
}

void Renderer::drawFrame(const GameWorld* w)
{
	glViewport(0, 0, mWidth, mHeight);
	glUniform1f(mRightUniform, mWidth);
	glUniform1f(mTopUniform, mHeight);
	glUniform1i(mTextureUniform, 0);

	auto track = w->getTrack();
	auto car = w->getCar();

	mScreenOrientation = mCamOrientation ? car->getOrientation() : 0.0f;

	if(mTrackSegments.empty()) {
		loadCarVBO(car);
		loadTrackVBO(track);
		loadGrassVBO(track);
	}

	auto carpos = car->getPosition();
	glUniform1f(mZoomUniform, mZoom);

	mCamPos.x = carpos.x;
	mCamPos.y = carpos.y;

	drawGrass();
	drawTrack();
	drawCar(car);
	drawDebugPoints();
	
	glUniform1f(mZoomUniform, 1.0f);
	drawTexts(w);

	{
		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR) {
			fprintf(stderr, "GL error 0x%04x\n", err);
		}
	}

}

void Renderer::updateDebug(const GameWorld* w)
{
	auto car = w->getCar();
	auto pos = car->getPosition();

	if(!w->getTrack()->onTrack(pos)) {
		mDebugPoints.add(pos, Color::Red);
	} else {
		mDebugPoints.add(pos, Color::Blue);
	}
}

void Renderer::drawCar(const Car* car)
{
	drawQuad(mCarVBO, mCarTexture, car->getPosition(),
			car->getOrientation(), Color::White);
}

void Renderer::drawTrackSegment(const TSRender& ts)
{
	glBindTexture(GL_TEXTURE_2D, mAsphaltTexture->getTexture());
	auto cam = Math::rotate2D(mCamPos, mScreenOrientation);
	glUniform2f(mCameraUniform, -cam.x, -cam.y);
	glUniform1f(mOrientationUniform, -mScreenOrientation);
	glUniform4f(mColorUniform, 1.0f, 1.0f, 1.0f, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, ts.VBO[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, ts.VBO[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, ts.ElemCount);
}

void Renderer::drawTrack()
{
	for(const auto& t : mTrackSegments) {
		drawTrackSegment(t);
	}
}

void Renderer::drawGrass()
{
	drawQuad(mGrassVBO, mGrassTexture, Common::Vector2(), 0.0f, Color::White);
}

void Renderer::drawDebugPoints()
{
	for(const auto& p : mDebugPoints.Points) {
		if(p.VBO[0])
			drawQuad(p.VBO, mAsphaltTexture, p.Pos, 0.0f, p.Color);
	}
}

void Renderer::drawQuad(const GLuint vbo[3],
		const Common::Texture* texture, const Common::Vector2& pos,
		float orient, const Common::Color& col, bool isHud)
{
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	auto cam = isHud ? mCamPos : Math::rotate2D(mCamPos, mScreenOrientation);
	auto rotpos = isHud ? pos : Math::rotate2D(pos, mScreenOrientation);
	glUniform2f(mCameraUniform, -cam.x + rotpos.x, -cam.y + rotpos.y);
	if(isHud)
		glUniform1f(mOrientationUniform, 0.0f);
	else
		glUniform1f(mOrientationUniform, orient - mScreenOrientation);
	glUniform4f(mColorUniform, col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

Text* Renderer::getText(const char* s)
{
	auto texture = mTextRenderer.renderText(s, Common::Color::White);
	auto it = mTextCache.find(texture);
	if(it == mTextCache.end()) {
		auto text = new Text(texture);
		mTextCache.insert({texture, text});
		return text;
	} else {
		return it->second;
	}
}

void Renderer::drawTexts(const GameWorld* w)
{
	char buf[128];
	sprintf(buf, "Speed: %d Km/h", (int)w->getCar()->getSpeed());
	auto text = getText(buf);
	drawHUDQuad(text->vbo, text->texture.get(), Vector2(10, 10), 0.0f, Common::Color::White);

	int i = 1;
	for(const auto& p : mInfoTexts) {
		auto t = getText(p.c_str());
		drawHUDQuad(t->vbo, t->texture.get(), Vector2(10, 10 + 10 * i), 0.0f, Common::Color::White);
		i++;
	}
}

void Renderer::drawHUDQuad(const GLuint vbo[3],
		const Common::Texture* texture, const Common::Vector2& pos,
		float orient, const Common::Color& col)
{
	drawQuad(vbo, texture, mCamPos + Vector2(mWidth, mHeight) * -1.0f + pos * 2.0f,
			-mScreenOrientation + orient, col, true);
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

void Renderer::setCamOrientation(bool o)
{
	mCamOrientation = o;
}

