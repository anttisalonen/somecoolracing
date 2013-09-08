varying vec2 vTexCoord;
varying vec3 vNormal;
varying float vPointLightDistance;

uniform sampler2D sTexture;
uniform vec3 uAmbientLight;
uniform vec3 uDirectionalLightDirection;
uniform vec3 uDirectionalLightColor;
uniform vec3 uPointLightColor;
uniform vec3 uPointLightAttenuation;
uniform bool uDirectionalLightEnabled;
uniform bool uPointLightEnabled;
uniform vec4 uColor;

void main()
{
	vec4 light;
	vec4 pointLight;
	float pointLightFactor;

	light = vec4(uAmbientLight, 1.0);

	if(uDirectionalLightColor != vec3(0.0, 0.0, 0.0)) {
		vec4 directionalLight;
		float directionalFactor;
		directionalFactor = dot(normalize(vNormal), -uDirectionalLightDirection);
		if(directionalFactor > 0.0) {
			directionalLight = vec4(uDirectionalLightColor, 1.0) * directionalFactor;
			light += directionalLight;
		}
	}

	if(uPointLightColor != vec3(0.0, 0.0, 0.0)) {
		pointLightFactor = 1.0 / (uPointLightAttenuation.x + uPointLightAttenuation.y * vPointLightDistance +
				uPointLightAttenuation.z * vPointLightDistance * vPointLightDistance);
		pointLightFactor = clamp(pointLightFactor, 0, 1);
		pointLight = vec4(pointLightFactor * uPointLightColor, 1.0);
		light += pointLight;
	}

	light = clamp(light, 0, 1);
	gl_FragColor = texture2D(sTexture, vTexCoord) * light * uColor;
}

