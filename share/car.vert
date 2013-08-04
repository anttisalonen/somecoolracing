attribute vec4 aPosition;
attribute vec2 aTexcoord;

varying vec2 vTexcoord;

uniform vec2 uCamera;
uniform float uRight;
uniform float uTop;
uniform float uZoom;

void main()
{
	float right  = uRight * uZoom;
	float left   = -uRight * uZoom;
	float top    = uTop * uZoom;
	float bottom = -uTop * uZoom;
	const float far    = 1.0;
	const float near   = -1.0;
	vTexcoord = aTexcoord;
	mat4 window_scale = mat4(vec4(2.0 / (right - left), 0.0, 0.0, 0.0),
		vec4(0.0, 2.0 / (top - bottom), 0.0, 0.0),
		vec4(0.0, 0.0, -2.0 / (far - near), 0.0),
		vec4(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0));
	gl_Position = window_scale * (aPosition + vec4(uCamera, 0.0, 0.0));
}
