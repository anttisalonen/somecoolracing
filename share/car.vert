attribute vec3 aPosition;
attribute vec2 aTexCoord;
attribute vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uInverseMVP;
uniform vec3 uPointLightPosition;

varying vec2 vTexCoord;
varying vec3 vNormal;
varying float vPointLightDistance;

void main()
{
	gl_Position = uMVP * vec4(aPosition, 1.0);
	vTexCoord = aTexCoord;
	vNormal = vec3(vec4(aNormal, 1.0) * uInverseMVP);
	vPointLightDistance = distance(aPosition, uPointLightPosition);
}

