//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>


std::vector<const GLchar*> faces;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;



glm::mat4 model;
glm::mat4 lightModel;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(-39.0f, -1.0f, 153.0f),
	glm::vec3(-39.0f, -1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D myModel;
gps::Model3D boat;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;
bool cameraAnimation = false;
float cameraOffset = 0.0f;
float cameraAngle = 0.0f;
bool cameraClockwise = false;
int nightTime = 0;

// fog
int foginit = 0;
GLint foginitLoc;
GLfloat fogDensity;

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch;
bool mouse = true;


bool wireframeView = false;

int pointinit = 0;
glm::vec3 lightPos1;
GLuint lightPos1Loc;
glm::vec3 lightPos2;
GLuint lightPos2Loc;

int spotinit;
float spotlight1;
float spotlight2;

glm::vec3 spotLightDirection;
glm::vec3 spotLightPosition;

float boatMoveX;
float boatMoveZ;
float boatRotation;

bool animationStart = false;
bool boatToCenter = false;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	
	// for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	// set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_B && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_T && action == GLFW_PRESS) {

		wireframeView = !wireframeView;

		if (wireframeView) {

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (mouse)
	{
		lastX = xpos;
		lastY = ypos;
		mouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; //reverse
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.5f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);

}

void initFaces()
{

	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/front.jpg");
	faces.push_back("skybox/back.jpg");
}

void initObjects() {
	myModel.LoadModel("objects/Scenary.obj");
	boat.LoadModel("objects/Boat.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); 
	const GLubyte* version = glGetString(GL_VERSION); 
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);

	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initUniforms() {

	myCustomShader.useShaderProgram();

	model = glm::mat4(2.0f);

	model = glm::translate(model, glm::vec3(7.0f, 0.0f, 0.0f)); 

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(2.0f, 12.0f, 11.0f);
	lightRotation = glm::rotate(lightModel, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)); 

	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 0.8f, 0.8f); //orange ?
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	

	// pointlight
	lightPos1 = glm::vec3(11.0f, -1.0f, -15.0f); // lantern
	lightPos1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1");
	glUniform3fv(lightPos1Loc, 1, glm::value_ptr(lightPos1));

	lightPos2 = glm::vec3(88.0f, 59.0f, -75.0f); // lighthouse
	lightPos2Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2");
	glUniform3fv(lightPos2Loc, 1, glm::value_ptr(lightPos2));
	


	// spotlight
	spotlight1 = glm::cos(glm::radians(40.5f));

	spotLightDirection = glm::vec3(1, 0, 0);
	spotLightPosition = glm::vec3(88.0f, 59.0f, -75.0f);

	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotlight1"), spotlight1);

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightDirection"), 1, glm::value_ptr(spotLightDirection));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));


	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	//bind nothing to attachment points
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind until ready to use
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void initSkyBoxShader()
{
	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

void processMovement()
{

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}
	if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}
	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);

		view = myCamera.getViewMatrix();
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	// start fog
	if (pressedKeys[GLFW_KEY_F]) {

		myCustomShader.useShaderProgram();
		foginit = 1;
		glUseProgram(myCustomShader.shaderProgram);
		foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "foginit");
		glUniform1i(foginitLoc, foginit);
	}

	// stop fog
	if (pressedKeys[GLFW_KEY_G]) {
		myCustomShader.useShaderProgram();
		foginit = 0;
		glUseProgram(myCustomShader.shaderProgram);
		foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "foginit");
		glUniform1i(foginitLoc, foginit);

	}

	// increase the intensity of fog
	if (pressedKeys[GLFW_KEY_O])
	{
		fogDensity = glm::min(fogDensity + 0.0001f, 1.0f);
		glUseProgram(myCustomShader.shaderProgram);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
	}

	// decrease the intensity of fog
	if (pressedKeys[GLFW_KEY_P])
	{
		fogDensity = glm::max(fogDensity - 0.0001f, 0.0f);
		glUseProgram(myCustomShader.shaderProgram);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
	}

	//turn point light on
	if (pressedKeys[GLFW_KEY_L]) {
		myCustomShader.useShaderProgram();
		pointinit = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
	}

	// turn point light off
	if (pressedKeys[GLFW_KEY_K]) {
		myCustomShader.useShaderProgram();
		pointinit = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
	}
	// line view
	if (pressedKeys[GLFW_KEY_9]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	// point view
	if (pressedKeys[GLFW_KEY_8]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	// normal view
	if (pressedKeys[GLFW_KEY_0]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_Z]) {
		glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (pressedKeys[GLFW_KEY_X]) {
		glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (pressedKeys[GLFW_KEY_1]) {
		cameraAnimation = true;
	}
	if (pressedKeys[GLFW_KEY_2]) {
		cameraAnimation = false;
	}
	if (pressedKeys[GLFW_KEY_N]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_M]) {
		lightAngle += 1.0f;
	}

	glm::vec3 cameraPosition = myCamera.getCameraPosition();
	std::cout << "Camera Position: " << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << std::endl;

}


glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 1.0f, far_plane = 200.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;

}

void renderScene() {

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// depth maps creation pass
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);


	depthMapShader.useShaderProgram();


	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	

	depthMapShader.useShaderProgram();
	if (boatToCenter)model = glm::translate(model, glm::vec3(48.21f + boatMoveX, 0.0f, 46.17f + boatMoveZ));
	model = glm::rotate(model, glm::radians(0.0f + boatRotation), glm::vec3(0, 1, 0));
	if (boatToCenter)model = glm::translate(model, glm::vec3(-48.21f - boatMoveX, 0.0f, -46.17f - boatMoveZ));
	model = glm::translate(model, glm::vec3(0.0f + boatMoveX, 0.0f, 0.0f + boatMoveZ));
	model = glm::scale(model, glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	boat.Draw(depthMapShader);



	depthMapShader.useShaderProgram();
	model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	myModel.Draw(depthMapShader);



	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// final scene rendering pass (with shadows)
	glViewport(0, 0, retina_width, retina_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myCustomShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	// bind the shadow map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),1,GL_FALSE,glm::value_ptr(computeLightSpaceTrMatrix()));


	// draw other objects
	myCustomShader.useShaderProgram();


	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


	
	myCustomShader.useShaderProgram();
	
	if(boatToCenter)model = glm::translate(model, glm::vec3(48.21f + boatMoveX, 0.0f, 46.17f + boatMoveZ));
	model = glm::rotate(model, glm::radians(0.0f + boatRotation), glm::vec3(0, 1, 0));
	if (boatToCenter)model = glm::translate(model, glm::vec3(-48.21f - boatMoveX, 0.0f, -46.17f - boatMoveZ));
	model = glm::translate(model, glm::vec3(0.0f + boatMoveX, 0.0f, 0.0f + boatMoveZ));
	model = glm::scale(model, glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	// compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	boat.Draw(myCustomShader);



	myCustomShader.useShaderProgram();

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	// compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	myModel.Draw(myCustomShader);


	mySkyBox.Draw(skyboxShader, view, projection);
}


void cameraAnim() {
	if (cameraAnimation == true) {
		if (cameraClockwise == false) {
			if (cameraAngle < 360)cameraAngle += 0.1f;
			else cameraClockwise = true;
		}
		else {
			if (cameraAngle > -200) {
				cameraAngle -= 0.1f;
				if (cameraOffset < 6)cameraOffset += 0.03f;
			}
			else cameraClockwise = false;
		}

		myCamera.animation(cameraOffset, cameraAngle);
	}
}

void boatAnim() {

	boatToCenter = true;

	if (boatMoveX < 90.0f && boatMoveZ >= 0.0f) {
		boatMoveX += 0.11f;
	}
	
	if (boatMoveX >= 90.0f && boatMoveZ >= 0.0f) {
		boatRotation += 90.0f;
	}
	
	if (boatMoveX >= 90.0f && boatMoveZ >-160.0f) {
		boatMoveZ -= 0.11f;
	}
	
	if (boatMoveX >= 90.0f && boatMoveZ <= -160.0f) {
		boatRotation += 90.0f;
	}
	
	if (boatMoveX > -130.0f && boatMoveZ <= -160.0f) {
		boatMoveX -= 0.11f;
	}
	
	if (boatMoveX <= -130.0f && boatMoveZ <= -160.0f) {
		boatRotation += 90.0f;
	}
	
	if (boatMoveX <=-130.0f && boatMoveZ < 0.0f) {
		boatMoveZ += 0.11f;
	}
	
	if (boatMoveX <= -130.0f && boatMoveZ >= 0.0f) {
		boatRotation += 90.0f;
	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();

	initObjects();
	initShaders();

	initUniforms();

	initFBO();
	initFaces();
	initSkyBoxShader();

	glCheckError();


	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();
		cameraAnim();
		boatAnim();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}