#include "Light.h"

Light::Light(vec3 pos, vec3 dir, vec3 ambient, vec3 diffuse, vec3 specular, LType lType, int number) { //Crear una luz, estableciendo el tipo y sus atributos

	Ldirection = dir;
	Lpos = pos;
	Lambient = ambient;
	Ldiffuse = diffuse;
	Lspecular = specular;
	LightType = lType;

	lightNumber = number;

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


void Light::SetAtt(float constant, float lineal, float quadratic) { //Modificador de atributos adicionales para el borde de la luz focal
	c1 = constant;
	c2 = lineal;
	c3 = quadratic;
}

void Light::SetAperture(float min, float max) { //Establecer una apertura para la luz focal
	MinAperture = cos(radians(min));
	MaxAperture = cos(radians(max));
}

void Light::SetLight(Shader *shad, vec3 CamPos) { //Metodo que asigna en el shader los valores necesarios para calcular la luz.
	std::string variable;
	glUniform3f(glGetUniformLocation(shad->Program, "viewPos"), CamPos.x, CamPos.y, CamPos.z);
	switch (LightType) {
	case DIRECTIONAL:
		glUniform3f(glGetUniformLocation(shad->Program, "dlight.direction"), Ldirection.x, Ldirection.y, Ldirection.z);
		glUniform3f(glGetUniformLocation(shad->Program, "dlight.ambient"), Lambient.x, Lambient.y, Lambient.z);
		glUniform3f(glGetUniformLocation(shad->Program, "dlight.diffuse"), Ldiffuse.x, Ldiffuse.y, Ldiffuse.z);
		glUniform3f(glGetUniformLocation(shad->Program, "dlight.specular"), Lspecular.x, Lspecular.y, Lspecular.z);
		break;
	case POINT:
		variable = "plight[" + std::to_string(lightNumber) + "].";
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "position").c_str()), Lpos.x, Lpos.y, Lpos.z);
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "ambient").c_str()), Lambient.x, Lambient.y, Lambient.z);
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "diffuse").c_str()), Ldiffuse.x, Ldiffuse.y, Ldiffuse.z);
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "specular").c_str()), Lspecular.x, Lspecular.y, Lspecular.z);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "constant").c_str()), c1);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "linear").c_str()), c2);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "quadratic").c_str()), c3);
		break;
	case SPOT:
		variable = "slight[" + std::to_string(lightNumber) + "].";
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "position").c_str()), Lpos.x, Lpos.y, Lpos.z);
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "direction").c_str()), Ldirection.x, Ldirection.y, Ldirection.z);
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "ambient").c_str()), Lambient.x, Lambient.y, Lambient.z);
		glUniform3f(glGetUniformLocation(shad->Program, (variable + "diffuse").c_str()), Ldiffuse.x, Ldiffuse.y, Ldiffuse.z);
		//glUniform3f(glGetUniformLocation(shad->Program, (variable + "specular").c_str()), Lspecular.x, Lspecular.y, Lspecular.z);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "constant").c_str()), c1);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "linear").c_str()), c2);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "quadratic").c_str()), c3);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "innerConeRadius").c_str()), MinAperture);
		glUniform1f(glGetUniformLocation(shad->Program, (variable + "outerConeRadius").c_str()), MaxAperture);
		break;
	default:
		break;
	}
}

void Light::Rotate(vec3 rotation) {	
//opcional
}

void Light::SetDirection(vec3 dir) {
	Ldirection = dir;
}

void Light::SetPosition(vec3 pos) {
	Lpos = pos;
}

/*vec3 Light::GetColor() {

}*/