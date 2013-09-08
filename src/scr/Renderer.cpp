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
			0.1f, 0.5f, -0.1f,
			-0.1f, 0.5f, 0.1f,
			-0.1f, 0.5f, -0.1f};
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

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	mCarProgram = loadProgram("share/car.vert", "share/car.frag", {{0, "aPosition"}, {1, "aTexCoord"}});
	mHUDProgram = loadProgram("share/hud.vert", "share/hud.frag", {{0, "aPosition"}, {1, "aTexCoord"}});
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	loadTextures();
	loadDebugVBO();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	setSceneDrawMode();
	glDepthFunc(GL_LEQUAL);

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

void Renderer::setSceneDrawMode()
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glUseProgram(mCarProgram);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUniform1i(glGetUniformLocation(mCarProgram, "sTexture"), 0);
}

void Renderer::setHUDDrawMode()
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glUseProgram(mHUDProgram);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glUniform1f(glGetUniformLocation(mHUDProgram, "uOrientation"), 0.0f);
	glUniform1f(glGetUniformLocation(mHUDProgram, "uZoom"), 1.0f);
	glUniform1f(glGetUniformLocation(mHUDProgram, "uRight"), mWidth);
	glUniform1f(glGetUniformLocation(mHUDProgram, "uTop"), mHeight);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUniform1i(glGetUniformLocation(mHUDProgram, "sTexture"), 0);
}

float Renderer::setZoom(float z)
{
	mZoom = Common::clamp(0.01f, z, 2.0f);
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
	float w = car->getLength();
	GLfloat vertices[] = {w, 0.0f, w,
		w, 0.0f, -w,
		-w, 0.0f, w,
		-w, 0.0f, -w};
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
	GLfloat vertices[] = {tr.x, 0.0f, tr.y,
		tr.x, 0.0f, bl.y,
		bl.x, 0.0f, tr.y,
		bl.x, 0.0f, bl.y};
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
	auto segments = t->getTrackSegments();

	for(const auto& seg : segments) {
		TSRender r;

		glGenBuffers(2, r.VBO);

		auto triStrip = seg->getTriangleStrip();

		std::vector<GLfloat> vertexdata;
		std::vector<GLfloat> texcoorddata;
		for(const auto& t : triStrip) {
			vertexdata.push_back(t.x);
			vertexdata.push_back(-0.1f);
			vertexdata.push_back(t.y);
			texcoorddata.push_back(t.x * 0.08f);
			texcoorddata.push_back(t.y * 0.08f);
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

Matrix44 Renderer::rotationVectorToMatrix(const Vector2& rot)
{
	Matrix44 ret;
	float theta = atan2(rot.y, rot.x);
	float ct = cos(theta);
	float st = sin(theta);
	// rotation around Y
	ret.m[0] = ct;
	ret.m[2] = -st;
	ret.m[8] = st;
	ret.m[10] = ct;
	return ret;
}

void Renderer::calculateModelMatrix(const Vector2& pos, const Vector2& rot,
		Matrix44& modelMatrix, Matrix44& inverseModelMatrix)
{
	Matrix44 translation = translationMatrix(Vector3(pos.x, 0.0f, pos.y));
	Matrix44 rotation = rotationVectorToMatrix(rot);
	modelMatrix = rotation * translation;

	auto invTranslation(translation);
	invTranslation.m[3] = -invTranslation.m[3];
	invTranslation.m[7] = -invTranslation.m[7];
	invTranslation.m[11] = -invTranslation.m[11];

	auto invRotation = rotation.transposed();

	inverseModelMatrix = invTranslation * invRotation;
}

void Renderer::updateMVPMatrix(const Vector2& pos, const Vector2& rot)
{
	Matrix44 modelMatrix, inverseModelMatrix;
	calculateModelMatrix(pos, rot, modelMatrix, inverseModelMatrix);
	auto mvp = modelMatrix * mViewMatrix * mPerspectiveMatrix;

	// TODO: naming of inverseMVP is misleading
	glUniformMatrix4fv(glGetUniformLocation(mCarProgram, "uMVP"), 1, GL_FALSE, mvp.m);
	glUniformMatrix4fv(glGetUniformLocation(mCarProgram, "uInverseMVP"), 1, GL_FALSE, inverseModelMatrix.m);
}

Matrix44 Renderer::perspectiveMatrix(float fov, int screenwidth, int screenheight)
{
	const float aspect_ratio = screenwidth / screenheight;
	const float znear = 0.5f;
	const float zfar = 500.0f;
	const float h = 1.0 / tan(Math::degreesToRadians(fov * 0.5f));
	const float neg_depth = znear - zfar;

	Matrix44 pers = Matrix44::Identity;
	pers.m[0 * 4 + 0] = h / aspect_ratio;
	pers.m[1 * 4 + 1] = h;
	pers.m[2 * 4 + 2] = (zfar + znear) / neg_depth;
	pers.m[2 * 4 + 3] = -1.0;
	pers.m[3 * 4 + 2] = 2.0 * zfar * znear / neg_depth;
	pers.m[3 * 4 + 3] = 0.0;
	return pers;
}

Matrix44 Renderer::translationMatrix(const Vector3& v)
{
	Matrix44 translation = Matrix44::Identity;
	translation.m[3 * 4 + 0] = v.x;
	translation.m[3 * 4 + 1] = v.y;
	translation.m[3 * 4 + 2] = v.z;
	return translation;
}

Matrix44 Renderer::cameraRotationMatrix(const Vector3& tgt, const Vector3& up)
{
	Vector3 n(tgt.negated().normalized());
	auto u = up.normalized().cross(n);
	auto v = n.cross(u);
	auto m = Matrix44::Identity;
	m.m[0] = u.x;
	m.m[1] = v.x;
	m.m[2] = n.x;
	m.m[4] = u.y;
	m.m[5] = v.y;
	m.m[6] = n.y;
	m.m[8] = u.z;
	m.m[9] = v.z;
	m.m[10] = n.z;

	return m;
}

void Renderer::updateFrameMatrices(const Vector2& pos, const Vector2& dir)
{
	mPerspectiveMatrix = perspectiveMatrix(90.0f, mWidth, mHeight);
	// TODO: why is up at 0, -1, 0?
	auto camrot = cameraRotationMatrix(Vector3(dir.x, 0.0f, dir.y), Vector3(0.0f, -1.0f, 0.0f));
	// camera height is set here
	auto camtrans = translationMatrix(Vector3(-pos.x, 1.0f, -pos.y));
	mViewMatrix = camtrans * camrot;
}

void Renderer::drawFrame(const GameWorld* w)
{
	glViewport(0, 0, mWidth, mHeight);
	setSceneDrawMode();

	auto car = w->getCar();
	updateFrameMatrices(mCamPos, car->getBody()->orientation);

	glUniform3f(glGetUniformLocation(mCarProgram, "uAmbientLight"), 1.0f, 1.0f, 1.0f);

	auto track = w->getTrack();

	mScreenOrientation = mCamOrientation ? car->getOrientation() : 0.0f;

	if(mTrackSegments.empty()) {
		loadCarVBO(car);
		loadTrackVBO(track);
		loadGrassVBO(track);
	}

	auto carpos = car->getPosition();
	if(mAutoZoomEnabled) {
		auto s = std::max<float>(5, car->getSpeed());
		mAutoZoom = 0.5f + 0.0001f * s * s;
	} else {
		mAutoZoom = 1.0f;
	}

	mCamPos.x = carpos.x;
	mCamPos.y = carpos.y;

	drawTrack();
	drawGrass();
	//drawCar(car);
	if(mDebugDisplay)
		drawDebugPoints();

	setHUDDrawMode();
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
	if(!mDebugDisplay)
		return;

	auto car = w->getCar();
	auto pos = car->getPosition();
	std::vector<Vector2> spots;

	float width = car->getWidth() * 0.5f;
	float length = car->getWheelbase() * 0.5f;
	float o = -car->getOrientation();

	spots.push_back(pos + Math::rotate2D(Vector2(width, length), o));
	spots.push_back(pos + Math::rotate2D(Vector2(-width, length), o));
	spots.push_back(pos + Math::rotate2D(Vector2(width, -length), o));
	spots.push_back(pos + Math::rotate2D(Vector2(-width, -length), o));

	for(const auto& p : spots) {
		if(!w->getTrack()->onTrack(p)) {
			mDebugPoints.add(p, Color::Red);
		} else {
			mDebugPoints.add(p, Color::Blue);
		}
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
	updateMVPMatrix(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f));
	glUniform4f(glGetUniformLocation(mCarProgram, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, ts.VBO[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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
		float orient, const Common::Color& col)
{
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	Vector2 rot;

	rot.x = cos(orient);
	rot.y = sin(orient);

	updateMVPMatrix(pos, rot);
	glUniform4f(glGetUniformLocation(mCarProgram, "uColor"), col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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
	sprintf(buf, "Speed: %d Km/h", (int)(w->getCar()->getSpeed() * 3.6f));
	auto text = getText(buf);
	drawHUDQuad(text->vbo, text->texture.get(), Vector2(10, 10), Common::Color::White);

	sprintf(buf, "Lateral acceleration: %2.1f g", w->getCar()->getLateralAcceleration() / 9.8f);
	text = getText(buf);
	drawHUDQuad(text->vbo, text->texture.get(), Vector2(10, 20), Common::Color::White);

	int i = 2;
	for(const auto& p : mInfoTexts) {
		auto t = getText(p.c_str());
		drawHUDQuad(t->vbo, t->texture.get(), Vector2(mWidth - 100, mHeight - 10 - 10 * i), Common::Color::White);
		i++;
	}
}

void Renderer::drawHUDQuad(const GLuint vbo[3],
		const Common::Texture* texture, const Common::Vector2& pos,
		const Common::Color& col)
{
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	Vector2 rot = pos * 2.0f - Vector2(mWidth, mHeight);
	glUniform2f(glGetUniformLocation(mHUDProgram, "uCamera"), rot.x, rot.y);
	glUniform4f(glGetUniformLocation(mHUDProgram, "uColor"), col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);

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

void Renderer::toggleCamOrientation()
{
	mCamOrientation = !mCamOrientation;
}

void Renderer::toggleDebugDisplay()
{
	mDebugDisplay = !mDebugDisplay;
}

void Renderer::toggleAutoZoom()
{
	mAutoZoomEnabled = !mAutoZoomEnabled;
}


