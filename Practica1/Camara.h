#pragma once

//GLEW
#include <GL\glew.h>
//GLFW
#include <GLFW\glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//Recogiendo algunos valores de main.cpp que queremos para esta clase:
extern const GLint WIDTH;
extern const GLint HEIGHT;

class Camara
{
public:
	Camara(glm::vec3 position, glm::vec3 direction, GLfloat sensitivity, GLfloat fov);
	~Camara();
	//Funcion rastreadora de inputs por teclado (Traslada la camara):
	void DoMovement();
	//Funcion rastreadora de inputs por raton en la ventana (Cambia el angulo de la camara):
	void MouseMove(double xpos, double ypos);
	//Funcion rastreadora de inputs en el scroll del raton (Cambia el FOV):
	void MouseScroll(double xScroll, double yScroll);
	//Funci�n que genera la matriz lookAt que necesita la camara para decidir su siguiente posici�n y angulo de vision:
	glm::mat4 LookAt();
	//Obtener datos privados de la camara:
	glm::vec3 getPosition();
	//Nose para que queremos esta funci�n, pero aqu� esta, devuelve el FOV:
	GLfloat GetFOV();
	void SetDT(GLfloat deltatime);
private:
	//Vectores necesarios para la camara:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	//Variables de tiempo:
	GLfloat Deltatime;
	GLfloat Lastframe;
	//Componentes posicion previa del raton:
	GLfloat LastMx = WIDTH / 2; 
	GLfloat LastMy = HEIGHT / 2;
	//Sensitividad de movimiento de la camara:
	GLfloat Sensitivity;
	//Este booleano se utiliza en el metodo MouseMove() enseguida que movemos el rat�n, evitando as� un salto de camara al inicio:
	GLboolean firstMouse;
	//Angulos de camara:
	GLfloat PITCH = 0.0f;
	GLfloat YAW = -90.0f;
	//Variables de apertura de la camara en perspectiva:
	GLfloat FOV;
	GLfloat fovSens = 2.0f;
	GLfloat maxFov;
	GLfloat minFov;
};

