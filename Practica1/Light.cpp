#include "Light.h"

Light::Light(vec3 pos, vec3 dir, vec3 ambient, vec3 diffuse, vec3 specular, LType lType, int number) {
	switch (lType) {
	case DIRECTIONAL:
		break;
	case POINT:
		break;
	case SPOT:
		break;
	default:
		break;
	}
}


void Light::SetAtt(float constant, float lineal, float quadratic) {

}

void Light::SetAperture(float min, float max) {

}

void Light::SetLight(Shader *shad, vec3 CamPos) {
	std::string variable;

	glUniform3f(glGetUniformLocation(shad->Program, "viewPos"), CamPos.x, CamPos.y, CamPos.z);
	switch (LightType){
	case DIRECTIONAL:

		break;
	case POINT:		
		variable = "plight[" + std::to_string(lightNumber) + "]";		

		break;
	case SPOT:
		variable = "slight[" + std::to_string(lightNumber) + "]";

		break;
	default:
		break;
	}	
}

void Light::Rotate(vec3 rotation) {	
//opcional
}

void Light::SetDirection(vec3 dir) {

}

void Light::SetPosition(vec3 pos) {

}

vec3 Light::GetColor() {

}