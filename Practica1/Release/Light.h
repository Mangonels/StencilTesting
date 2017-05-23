#pragma once
#define GLEW_STATIC
#include <GL\glew.h>
#include <glm.hpp>
#include "Shader.h"
#include<glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <string>

using namespace glm;

enum LType {
	DIRECTIONAL = 0,
	POINT = 1,
	SPOT = 2
};

class Light {
public:
	Light(vec3 pos, vec3 dir, vec3 ambient, vec3 diffuse, vec3 specular, LType lType, int number);
	void SetAtt(float constant, float lineal, float quadratic);
	void SetLight(Shader *shad, vec3 CamPos);
	void Rotate(vec3 rotation);
	void SetDirection(vec3 dir);
	void SetAperture(float min, float max);
	void SetPosition(vec3 pos);
	vec3 GetColor();

private:
	vec3 Ldirection;
	vec3 Lpos;
	vec3 Lambient;
	vec3 Ldiffuse;
	vec3 Lspecular;
	LType LightType;
	float MinAperture;
	float MaxAperture;
	float c1, c2, c3;
	int lightNumber;
};

