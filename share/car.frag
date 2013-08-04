#ifdef GL_ES
precision mediump float;
#endif

varying vec2 vTexcoord;

uniform sampler2D sTexture;

void main()
{
	gl_FragColor = texture2D(sTexture, vTexcoord);
}
