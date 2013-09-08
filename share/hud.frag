#ifdef GL_ES
precision mediump float;
#endif

varying vec2 vTexcoord;

uniform sampler2D sTexture;
uniform vec4 uColor;

void main()
{
	gl_FragColor = texture2D(sTexture, vTexcoord) * uColor;
}
