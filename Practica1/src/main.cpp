//GLEW
#define GLEW_STATIC
#include <GL\glew.h>
//GLFW
#include <GLFW\glfw3.h>
#include <iostream>

using namespace std;

//Librer�a de cargado de imagenes:
#include <SOIL.h>

//Librer�as de matematicas de OpenGL:
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//Clases:
#include "../Shader.h"
#include "../Camara.h"
#include "../Object.h"
#include "../material.h"
#include "../Model.h"
#include "../Mesh.h"

//Para sqrt()
#include <math.h>

const GLint WIDTH = 800, HEIGHT = 600; //Dimensiones de la ventana que creamos mas adelante

enum toStencil {
	CHARMANDER, BULBASAUR, SQUIRTLE, NONE
};

//Invocaci�n de la clase camara, para todas las funcionalidades de camara necesarias:
//Los 2 primeros valores son puntos con los que la clase forma los vectores que necesitamos para la camara:
Camara* camara = new Camara(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 60.0f); //Posicion, Direccion, Sensibilidad de camara, fov de camara.

//Definicion inicial de funciones manager de inputs:
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode); //Funcion rasteadora de inputs por teclado en la ventana 
void mouse_move_input(GLFWwindow* window, double xpos, double ypos) 
{
	camara->MouseMove(xpos, ypos);
}
void mouse_scroll_input(GLFWwindow* window, double xOffset, double yOffset)
{
	camara->MouseScroll(xOffset, yOffset);
}
void drawStencil(toStencil whatToDraw, glm::mat4 model, Shader *lightShader, Shader stencilShader, Model charmander, Model bulbasaur, Model squirtle, Model charmanderStencil, Model bulbasaurStencil, Model squirtleStencil);
toStencil selectPokemon(glm::vec3 camPos, glm::vec3 camDir);

//Variables calculo de tiempo:
GLfloat deltaTime = 0.0f;
GLfloat prevFrame = 0.0f;

//Variables posici�n rat�n (para camara)
GLfloat mPrevX = WIDTH/2, mPrevY = HEIGHT/2; //Componentes posicion previa del raton

//Variables de la camara:
//Angulos de camara iniciales (Se influencian con el raton)
float pitch = 0.0f; //(Arriba-Abajo  ejes: Y-X/Z)
float yaw = -90.0f; //(Izquierda-Derecha  ejes: Z/X)
//Variable que contiene el angulo de field of vision (la modificamos en el multi input):
float fov = 60.0f;
float fovSens = 2.0f;
float minFov = 20.0f;
float maxFov = 80.0f;
//Obtenemos posicion de la camara mediante metodo (Necesario para pasar a shaders mas abajo):
glm::vec3 camaraPos;

//CUBO LUZ Y GENERACION DE LUZ:
//Posicion:
GLfloat lightPosOffsetX = 0.0f;
GLfloat lightPosOffsetY = 0.0f;
GLfloat lightPosOffsetZ = 0.0f;

glm::vec3 lightPos(0.0f, 20.0f, 0.0f); //La posicion de la luz que usaremos tambien como posicion de traslado del cubo emisor de luz
glm::vec3 lightDir(0.0f, -1.0f, 0.0f); //La direccion de la luz que usamos para todas las luces

GLfloat spotLightInnerRadius = glm::cos(glm::radians(13.0f));
GLfloat spotLightOuterRadius = glm::cos(glm::radians(20.0f));

//Variables calculo de atenuacion de luz puntual:
float constant = 1.0f; //Componente constante que asegura que el valor de atenuacion que genere la formula no sea menor a 1.
float linear = 0.09f; //Se multiplica por la distancia, reduciendo la intensidad en estilo linear.
float quadratic = 0.032f; //Se multiplica por el quadrante de la distancia y establece un descenso de intensidad para el origen de luz.

float normalScene = true;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

bool keys[1024]; //Guarda que teclas se estan pulsando, ya que el sistema de input de glfw no puede procesar m�s de 1 tecla a la vez.

void multiInputChecker() { //Funci�n para revisar diversos inputs simultaneos:

	//Inputs de movimiento de cubo luz:
	if (keys[GLFW_KEY_1]) {
		normalScene = true;
	} else if (keys[GLFW_KEY_2]) {
		normalScene = false;
	}
}

int main() {

	//initGLFW
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	//comprobar que GLFW estaactivo
	if (!glfwInit())
	exit(EXIT_FAILURE);

	//set GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//create a window
	window = glfwCreateWindow(WIDTH, HEIGHT, "Stencil Testing", nullptr, nullptr);
	if (!window) {
		cout << "Error al crear la ventana" << endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	//set GLEW and inicializate
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		cout << "Error al iniciar glew" << endl;
		glfwTerminate();
		return NULL;
	}

	//Definicion de funciones de rastreo de input (Determinar que funcion va a esperar inputs de que tipo): Parametros: En que ventana se usar� esta funci�n, que funci�n.
	glfwSetKeyCallback(window, key_callback); //Definicion de funci�n de input por teclado
	glfwSetCursorPosCallback(window, mouse_move_input); //Definicion de funci�n de input por posicion de raton
	glfwSetScrollCallback(window, mouse_scroll_input); //Definicion de funcion de input por scroll de raton

	//set windows and viewport
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	// Opciones del Stencil Testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Encapsula el rat�n para que no pueda salir de las coordenadas internas a la ventana, y lo esconde.

	//Activar Z-Buffer (Buffer de profundidad de fragmentos)
	glEnable(GL_DEPTH_TEST); //Esto comprobar� que las caras de los poligonos no se fuckeen una encima de la otra, asegurando mostrar con prioridad las que est�n m�s cerca de la camara.

	//Crear shaders:
	//Posibles directorios segun el tipo de iluminacion que queramos:
	const GLchar* simplePath = "./src/FragmentLight.fragmentshader"; //Luces ambiental, Difusa y Especular combinadas (Phong)
	const GLchar* directionalPath = "./src/FragmentDirectionalLight.fragmentshader"; //Luz direccional
	const GLchar* pointPath = "./src/FragmentPointLight.fragmentshader"; //Luz puntual
	const GLchar* spotPath = "./src/FragmentSpotLight.fragmentshader"; //Luz focal
	//Shader para objetos:
	Shader *lightShader = new Shader("./src/VertexLight.vertexshader", simplePath); //<- Cambiar el tipo de fragshader segun disponibles arriba: Se trata del shader de reflejos de luz sobre objetos
	//Shader especifico para el cubo emisor de luz:
	Shader emitterShader = Shader::Shader("./src/VertexEmitter.vertexshader", "./src/FragmentEmitter.fragmentshader"); //Color base, solo para el cubo de la luz
	//Shader del Stencil
	Shader stencilShader = Shader::Shader("./src/VertexStencil.vertexshader", "./src/FragmentStencil.fragmentshader");

	//Generating light cube object with first transformations:
	vec3 lightCubeScale = vec3(0.1f, 0.1f, 0.1f);
	vec3 lightCubeRotate = vec3(1.f, 1.f, 1.f);
	Object lightCube(lightCubeScale, lightCubeRotate, lightPos); //CUBE B, la posicion es la posicion de la luz definida globalmente.

	//Generacion de material:
	//Material material("./Materials/difuso.png", "./Materials/especular.png", 16);

	//material.SetMaterial(lightShader); //Pasar el shader por referencia a la clase material que le asigna una textura difusa y especular.

	// Modelos
	Model gba("./OBJs/GBASP/gbasp.obj");
	Model charmander("./OBJs/NewPoke/Charmander/DolHitokage.obj");
	Model bulbasaur("./OBJs/NewPoke/Bulbasaur/DolFushigidane.obj");
	Model squirtle("./OBJs/NewPoke/Squirtle/squirtle.obj");

	Model charmanderStencil("./OBJs/NewPoke/Stencil/charmander.obj");
	Model squirtleStencil("./OBJs/NewPoke/Stencil/squirtle.obj");
	Model bulbasaurStencil("./OBJs/NewPoke/Stencil/bulbasaur.obj");
	

	//BUCLE DE DIBUJO:
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); //Comprobar si se ha activado algun evento y ejecutar la funcion correspondiente (Las hemos definido antes fuera del main)
		multiInputChecker(); //Llamando comprobaci�n de inputs simultaneos

		//Actualizando valores de tiempo:
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - prevFrame;
		prevFrame = currentFrame;
		camara->SetDT(deltaTime); //Pasamos el delta time a la propiedad de la clase

		//Actualizar posicion de camara
		camaraPos = camara->getPosition();

		//Mirar si se ha pulsado alguna tecla que mueva la posicion de la camara:
		camara->DoMovement();

		//Clearing color buffer array from OpenGL in order to ensure there is no colors being applied from last iteration:
		//Also clearing Z-Buffer.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		

		//Aplicar lightShader (Shader especifico cubo):
		lightShader->USE(); //Shaders para el CUBO
		//material.SetShininess(lightShader); //Pasar el shader por referencia a la clase material que le asigna el valor de brillo
		//material.ActivateTextures();
			//Localizando las variables uniform de color del shader de luces, e inicializandolas mediante transferencia:
			//-------------------------
			GLint objectColorLoc = glGetUniformLocation(lightShader->Program, "objectColor");
			GLint lightColorLoc = glGetUniformLocation(lightShader->Program, "lightColor");
			GLint lightPosLoc = glGetUniformLocation(lightShader->Program, "lightPosition");
			GLint lightDirPos = glGetUniformLocation(lightShader->Program, "lightDirection");
			GLint lightSpotInnerCone = glGetUniformLocation(lightShader->Program, "innerConeRadius"); 
			GLint lightSpotOuterCone = glGetUniformLocation(lightShader->Program, "outerConeRadius");
			GLint viewPosLoc = glGetUniformLocation(lightShader->Program, "viewPos");
			//glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f); //Enviar Color del objeto
			glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); //Enviar Color de la luz
			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z); //Enviar Posicion de la luz
			glUniform3f(lightDirPos, lightDir.x, lightDir.y, lightDir.z); //Enviar Direccion para cualquier luz
			glUniform1f(lightSpotInnerCone, spotLightInnerRadius); //Enviar angulo que determina el radio de apertura de la luz focal
			glUniform1f(lightSpotOuterCone, spotLightOuterRadius); //Enviar angulo que determina el radio de apertura de la luz focal
			glUniform3f(viewPosLoc, camaraPos.x, camaraPos.y, camaraPos.z); //Enviar Posicion de la camara

			//Variables de calculo de atenuacion en point light:
			glUniform1f(glGetUniformLocation(lightShader->Program, "constant"), constant);
			glUniform1f(glGetUniformLocation(lightShader->Program, "linear"), linear);
			glUniform1f(glGetUniformLocation(lightShader->Program, "quadratic"), quadratic);
			//-------------------------

			//Localizar donde van las matrices:
			GLint viewLoc = glGetUniformLocation(lightShader->Program, "view");
			GLint projLoc = glGetUniformLocation(lightShader->Program, "projection");
			GLint modelLoc = glGetUniformLocation(lightShader->Program, "model");

			glm::mat4 originMatrix; //Una matriz con sus valores en 0 (Para resetear la matriz modelo)

			//Matriz Vista:
			glm::mat4 view = camara->LookAt(); //Trasladamos la escena en la direcci�n contraria hacia donde queremos mover la camara, causando el efecto de que la camara se ha movido:
			//Matriz Proyeccion:
			glm::mat4 projection = glm::perspective(glm::radians(camara->GetFOV()), (float)(screenWidth / screenHeight), 0.1f, 100.0f); //angulo de fov, tama�o pantalla (tiene que ser un float), plano near, plano far.
			//Matriz Modelo:
			glm::mat4 model; //La matriz modelo la define el generador de matriz del cubo.

			//Enviar matrices a sus localizaciones:
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			stencilShader.USE();
			glUniformMatrix4fv(glGetUniformLocation(stencilShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(stencilShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		//DIBUJAR GBA SP:
		lightShader->USE();
		
		if (normalScene) {
			glStencilMask(0x00); // La gba no va a influir en el stencil por lo que su Mask se setea a 0x00
			gba.Draw(*lightShader, GL_STATIC_DRAW);

			drawStencil(selectPokemon(camara->getPosition(),camara->cameraFront), model, lightShader, stencilShader, charmander, bulbasaur, squirtle, charmanderStencil, bulbasaurStencil, squirtleStencil);
		}
		else if (!normalScene) {
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
			//DIBUJAR CHARMANDER:
			//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
			model = glm::translate(model, glm::vec3(1.6f, 4.3f, -6.2f));
			model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
			model = glm::scale(model, glm::vec3(0.107, 0.107, 0.107));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			//Dibujar:
			charmander.Draw(*lightShader, GL_STATIC_DRAW);
			
			glBindVertexArray(0);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);

			model = originMatrix;
			//DIBUJAR BULBASAUR:
			//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
			model = glm::translate(model, glm::vec3(0.0f, 4.3f, -6.2f));
			model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
			model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			//Dibujar:
			bulbasaur.Draw(*lightShader, GL_STATIC_DRAW);

			

			model = originMatrix; //Reset matriz modelo

			//DIBUJAR SQUIRTLE:
			//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
			model = glm::translate(model, glm::vec3(-1.7f, 4.3f, -6.2f));
			model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
			model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			//Dibujar:
			squirtle.Draw(*lightShader, GL_STATIC_DRAW);

			glStencilMask(0xFF);
			glEnable(GL_DEPTH_TEST);
			
		}
		

		model = originMatrix; //Reset matriz modelo

		

		//Aplicar emitterShader (Shader especifico cubo emisor de luz)
		emitterShader.USE();
			//Ajustar uniforms en las localizaciones pertinentes:
			//Localizar donde van las matrices:
			modelLoc = glGetUniformLocation(emitterShader.Program, "model");
			viewLoc = glGetUniformLocation(emitterShader.Program, "view");
			projLoc = glGetUniformLocation(emitterShader.Program, "projection");

			//Preparaciones en el cubo para la matriz modelo:
			 lightCube.Translate(glm::vec3(lightPos.x /*+ lightPosOffsetX*/, lightPos.y /*+ lightPosOffsetY*/, lightPos.z /*+ lightPosOffsetZ*/)); //Descomentando los offsets podemos cambiar posicion del cubo luz
			 
			//Cambiamos la matriz modelo, (las demas matrices pueden quedarse igual)
			model = glm::mat4(lightCube.generateModelMatrix()); //Obtener una nueva matriz

			//Enviar matrices a sus localizaciones:
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//DIBUJAR CUBO DE LUZ:
		lightCube.drawCube();
		
		

		//Cambia framebuffer por buffer de ventana:
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW. No need for even returning 0.
	exit(EXIT_SUCCESS);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {

	//-------------------------------
	//INPUTS INDIVIDUALES:
	//-------------------------------

	//Cierre de ventana:
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//------------------------------------------------
	//INPUTS SIMULTANEOS - Estados de teclas pulsadas:
	//------------------------------------------------

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void drawStencil(toStencil whatToDraw, glm::mat4 model, Shader *lightShader, Shader stencilShader, Model charmander, Model bulbasaur, Model squirtle, Model charmanderStencil, Model bulbasaurStencil, Model squirtleStencil) {
	switch (whatToDraw) {
	case CHARMANDER:
		model = glm::mat4(); //Reset matriz modelo
		lightShader->USE();
		//DIBUJAR BULBASAUR:
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(0.0f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		bulbasaur.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo

		//DIBUJAR SQUIRTLE:
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(-1.7f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		squirtle.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo

							 //DIBUJAR BULBASAUR:
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		//Dibujar CHARMANDER:
		model = glm::translate(model, glm::vec3(1.6f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.107, 0.107, 0.107));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		charmander.Draw(*lightShader, GL_STATIC_DRAW);
		glBindVertexArray(0);
		
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		//glDisable(GL_DEPTH_TEST);
		stencilShader.USE();
		model = glm::mat4(); //Reset matriz modelo
		model = glm::translate(model, glm::vec3(1.6f, 4.24f, -6.2f));
		model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.117, 0.117, 0.117));
		glUniformMatrix4fv(glGetUniformLocation(stencilShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		charmanderStencil.Draw(stencilShader, GL_STATIC_DRAW);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		break;

	case BULBASAUR:
		model = glm::mat4(); //Reset matriz modelo
		lightShader->USE();
		//DIBUJAR CHARMANDER:
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(1.6f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.107, 0.107, 0.107));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		charmander.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo

		//DIBUJAR SQUIRTLE:
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(-1.7f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		squirtle.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo

		//DIBUJAR BULBASAUR:
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(0.0f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		bulbasaur.Draw(*lightShader, GL_STATIC_DRAW);
		glBindVertexArray(0);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		//glDisable(GL_DEPTH_TEST);
		stencilShader.USE();
		model = glm::mat4(); //Reset matriz modelo
		model = glm::translate(model, glm::vec3(0.0f, 4.25f, -6.2f));
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.11, 0.11, 0.11));
		glUniformMatrix4fv(glGetUniformLocation(stencilShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		bulbasaurStencil.Draw(stencilShader, GL_STATIC_DRAW);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		break;

	case SQUIRTLE:
		model = glm::mat4(); //Reset matriz modelo
		lightShader->USE();
		model = glm::translate(model, glm::vec3(0.0f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		bulbasaur.Draw(*lightShader, GL_STATIC_DRAW);
		
		model = glm::mat4(); //Reset matriz modelo
		model = glm::translate(model, glm::vec3(1.6f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.107, 0.107, 0.107));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		charmander.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(-1.7f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		squirtle.Draw(*lightShader, GL_STATIC_DRAW);
		glBindVertexArray(0);

		model = glm::mat4(); //Reset matriz modelo
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		//glDisable(GL_DEPTH_TEST);
		stencilShader.USE();
		model = glm::translate(model, glm::vec3(-1.7f, 4.24f, -6.18f));
		model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.11, 0.11, 0.11));
		glUniformMatrix4fv(glGetUniformLocation(stencilShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		squirtleStencil.Draw(stencilShader, GL_STATIC_DRAW);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		break;

	case NONE:
		model = glm::mat4(); //Reset matriz modelo
		lightShader->USE();
		model = glm::translate(model, glm::vec3(0.0f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		bulbasaur.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo
		model = glm::translate(model, glm::vec3(1.6f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.107, 0.107, 0.107));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		charmander.Draw(*lightShader, GL_STATIC_DRAW);

		model = glm::mat4(); //Reset matriz modelo
		//Transformaciones previas de matriz modelo para ajustar posicion rotacion y escalado:
		model = glm::translate(model, glm::vec3(-1.7f, 4.3f, -6.2f));
		model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
		glUniformMatrix4fv(glGetUniformLocation(lightShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//Dibujar:
		squirtle.Draw(*lightShader, GL_STATIC_DRAW);
		glBindVertexArray(0);
		break;
	}
}

toStencil selectPokemon(glm::vec3 camPos, glm::vec3 camDir) {
	float radius = 0.75f;

	//camDir*(camPos-BALLPOS)^2 - glm::dot(camPos,BALLPOS)^2 - radius^2
//	printf("%f, %f, %f\n", camPos.x, camPos.y, camPos.z);
	//sqrt(camDir*(camPos - BALLPOS)) - sqrt(glm::dot(camPos, BALLPOS)) - sqrt(radius)
	glm::vec3 charmanderPos = glm::vec3(1.6f, 5.f, -6.2f);
	glm::vec3 squirtlePos = glm::vec3(-1.7f, 5.f, -6.2f);
	glm::vec3 bulbasaurPos = glm::vec3(0.0f, 5.f, -6.2f);

	float tmp1Char = pow(glm::dot(camDir, (camPos - charmanderPos)), 2);
	float tmp1Squirt = pow(glm::dot(camDir, (camPos - squirtlePos)), 2);
	float tmp1Bulb = pow(glm::dot(camDir, (camPos - bulbasaurPos)), 2);

	float lChar = glm::length(camPos - charmanderPos);
	float lSquirt = glm::length(camPos - squirtlePos);
	float lBulb = glm::length(camPos - bulbasaurPos);

	float tmp2Char = pow(lChar, 2) - pow(radius, 2);
	float tmp2Squirt = pow(lSquirt, 2) - pow(radius, 2);
	float tmp2Bulb = pow(lBulb, 2) - pow(radius, 2);

	// Charmander
	if ((tmp1Char - tmp2Char) >= 0) {
		return CHARMANDER;
	}
	// Squirtle
	else if ((tmp1Squirt - tmp2Squirt) >= 0) {
		return SQUIRTLE;
	}
	// Bulbasaur
	else if ((tmp1Bulb - tmp2Bulb) >= 0) {
		return BULBASAUR;
	} else { return NONE; }
}