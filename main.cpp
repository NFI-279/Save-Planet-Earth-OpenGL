// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW and GLFW
#include "dependente\glew\glew.h"
#include "dependente\glfw\glfw3.h"

// Include GLM
#include "dependente\glm\glm.hpp"
#include "dependente\glm\gtc\matrix_transform.hpp"
#include "dependente\glm\gtc\type_ptr.hpp"

// Include our helper function for loading shaders
#include "shader.hpp"

// Global variables
GLFWwindow* window;
const int width = 1024, height = 1024;
float scaleX = 1.5f, scaleY = 0.5f, scaleZ = 0;

glm::vec3 personPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 monsterPos = glm::vec3(0.5f, 0.0f, 0.0f);
float speed = 0.001f; //Speed
bool moveUp = false;
bool moveDown = false;
bool moveLeft = false;
bool moveRight = false;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool isPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
	bool isReleased = (action == GLFW_RELEASE);

	if (key == GLFW_KEY_W) moveUp = isPressed ? true : (isReleased ? false : moveUp);
	if (key == GLFW_KEY_S) moveDown = isPressed ? true : (isReleased ? false : moveDown);
	if (key == GLFW_KEY_A) moveLeft = isPressed ? true : (isReleased ? false : moveLeft);
	if (key == GLFW_KEY_D) moveRight = isPressed ? true : (isReleased ? false : moveRight);

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		(void)getchar();
		return -1;
	}

	//GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	//const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	// Open a window and create its OpenGL context, retrieve the value into the global variable
	//window = glfwCreateWindow(width, height, "savetheearth", primaryMonitor, NULL);
	window = glfwCreateWindow(width, height, "savetheearth", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.");
		(void)getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		(void)getchar();
		glfwTerminate();
		return -1;
	}

	// Specify the size of the rendering window
	//int framebufferWidth, framebufferHeight;
	//glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	//glViewport(0, 0, framebufferWidth, framebufferHeight);

	// Clear framebuffer with dark blue color
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glfwSetKeyCallback(window, key_callback);

	// Use our helper function to load the shaders from the specified files
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	GLfloat vertices[] = {
		//body	
		0.05f,  0.05f, 0.0f,	// top right
		-0.05f, 0.05f, 0.0f,	// top left
		-0.05f, -0.05f, 0.0f,	// bottom left
		0.05f,  -0.05f, 0.0f,	// bottom left 
		//right arm
		0.125, 0.075, 0.0,	
		0.125, 0.025, 0.0,
		//left arm
		-0.125, 0.075, 0.0,
		-0.125, 0.025, 0.0,
		//left foot
		-0.1, -0.125, 0.0,
		-0.125, -0.1, 0.0,
		//right foot,
		0.1, -0.125, 0.0,
		0.125, -0.1, 0.0,
		//head
		0.025, 0.125, 0.0,
		-0.025, 0.125, 0.0,
		0.0, 0.05, 0.0,
	};

	GLuint indices[] = {
		//Person
		0, 1, 2,	//body
		0, 3, 2,
		0, 4, 5,	//right arm
		1, 6, 7,	//left arm
		2, 8, 9,	//left foot
		3, 10, 11,	//right foot
		12, 13, 14,	//head
	};

	// Create VAO, VBO and IBO
	GLuint vbo, vao, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	// Bind VAO
	glBindVertexArray(vao);

	// Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Describe the data in the buffer
	// This will be accesible in the shaders
	glVertexAttribPointer(
		0,                  // attribute 0, must match the layout in the shader.
		3,                  // size of each attribute (3 floats in this case)
		GL_FLOAT,           // type of data
		GL_FALSE,           // should be normalized?
		3 * sizeof(float),  // stride for this attribute
		(void*)0            // initial offset for array buffer
	);
	glEnableVertexAttribArray(0);

	// Create identity matrix for transforms
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, personPos);

	// Maybe we can play with different positions
	glm::vec3 positions[] = {
		glm::vec3(0.0f,  0.0f,  0),
	};

	// Check if the window was closed
	while (!glfwWindowShouldClose(window))
	{
		// Swap buffers
		glfwSwapBuffers(window);
		// Check for events
		glfwPollEvents();
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);
		// Use our shader
		glUseProgram(programID);
		// Bind VAO
		glBindVertexArray(vao);

		//Moving
		if (moveUp) personPos.y += speed;
		if (moveDown) personPos.y -= speed;
		if (moveLeft) personPos.x -= speed;
		if (moveRight) personPos.x += speed;

		// Clamp XY positions
		if (personPos.x < -1.0f + 0.125f) personPos.x = -1.0f + 0.125f;
		if (personPos.x > 1.0f - 0.125f) personPos.x = 1.0f - 0.125f;
		if (personPos.y < -1.0f + 0.125f) personPos.y = -1.0f + 0.125f;
		if (personPos.y > 1.0f - 0.125f) personPos.y = 1.0f - 0.125f;

		//Draw Person
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, personPos);
		// Send variables to shaders via uniforms
		unsigned int transformLoc = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		unsigned int transformLoc2 = glGetUniformLocation(programID, "color");
		glm::vec4 color = glm::vec4(0, 1.0f, 0, 1.0);
		glUniform4fv(transformLoc2, 1, glm::value_ptr(color));
		// Draw call
		glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, 0);

		//Draw Monster
		glm::mat4 transMonster = glm::mat4(1.0f);
		transMonster = glm::translate(transMonster, monsterPos);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transMonster));
		glm::vec4 monsterColor = glm::vec4(1.0f, 0, 0, 1.0);
		glUniform4fv(transformLoc2, 1, glm::value_ptr(monsterColor));
		// Draw call
		glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, 0); // draw monster

	}

	// Cleanup
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}