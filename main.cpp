// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

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

GLuint circleVAO, circleVBO;
GLsizei circleVertexCount = 0;

glm::vec3 orignalPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 personPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 monsterPos = glm::vec3(0.5f, 0.0f, 0.0f);
float speed = 0.001f; //Speed
bool moveUp = false;
bool moveDown = false;
bool moveLeft = false;
bool moveRight = false;

struct bullet {
	glm::vec3 pos;
	glm::vec3 dir;
	float speed = 0.001f;
	bool active;
};
std::vector<bullet> bullets;
float lastShoot = 0.0f;
float interval = 1.0f;

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

void createCircle() {
	int num_segments = 100;
	float radius = 0.05;
	double pi = 3.14159;
	std::vector<float> circleVertices;

	circleVertices.push_back(0.0f); // x
	circleVertices.push_back(0.0f); // y
	circleVertices.push_back(0.0f); // z

	for (int i = 0; i <= num_segments; i++) {
		float angle = 2.0f * pi * float(i) / float(num_segments);
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		circleVertices.push_back(x);
		circleVertices.push_back(y);
		circleVertices.push_back(0.0f);
	}

	circleVertexCount = (GLsizei)(circleVertices.size() / 3);

	glGenVertexArrays(1, &circleVAO);
	glGenBuffers(1, &circleVBO);
	glBindVertexArray(circleVAO);

	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
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
		0.05, 0.025, 0.0f,
		0.125, 0.025, 0.0,
		0.125, 0.05, 0.0,
		//left arm
		-0.05, 0.025, 0.0f,
		-0.125, 0.025, 0.0,
		-0.125, 0.05, 0.0,
		//left foot
		-0.05f, -0.125f, 0.0f,
		-0.015f, -0.125f, 0.0f,
		-0.015f, -0.0f, 0.0f,
		//right foot,
		0.05f, -0.125f, 0.0f,
		0.015f, -0.125f, 0.0f,
		0.015f, -0.0f, 0.0f,
	};

	GLuint indices[] = {
		//Person
		0, 1, 2,	//body
		0, 3, 2,
		0, 4, 5,	//right arm
		0, 6, 5,
		1, 7, 8,	//left arm
		1, 9, 8,
		2, 10, 11,	//left foot
		2, 12, 11,
		3, 13, 14,	//right foot
		3, 15, 14,
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

	createCircle();

	// Check if the window was closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programID);

		//Add bulets
		float currentTime = glfwGetTime();
		if (currentTime - lastShoot > interval) {
			bullet b;
			b.pos = monsterPos; 
			b.dir = glm::vec3(-1.0f, 0.0f, 0.0f);
			b.active = true;
			bullets.push_back(b);
			lastShoot = currentTime;
		}

		//Collision with bullet
		for (auto& b : bullets) {
			if (b.active) {
				b.pos += b.dir * b.speed;
				if (b.pos.x < -1.2f) {
					b.active = false;
				}
				if ((abs(personPos.x - b.pos.x) < 0.15 && abs(personPos.y - b.pos.y) < 0.15) ||
					(sqrt(pow(personPos.x - b.pos.x, 2) + pow(personPos.y - b.pos.y, 2)) < 0.15)) {
					personPos = orignalPos;
				}
			}
		}

		//Moving
		if (moveUp) personPos.y += speed;
		if (moveDown) personPos.y -= speed;
		if (moveLeft) personPos.x -= speed;
		if (moveRight) personPos.x += speed;
		personPos.x = glm::clamp(personPos.x, -1.0f + 0.125f, 1.0f - 0.125f);
		personPos.y = glm::clamp(personPos.y, -1.0f + 0.125f, 1.0f - 0.125f);

		//Collision with monster
		if ((abs(personPos.x - monsterPos.x) < 0.25 && abs(personPos.y - monsterPos.y) < 0.25) ||
			(sqrt(pow(personPos.x-monsterPos.x, 2) + pow(personPos.y - monsterPos.y, 2)) < 0.25)) {
			personPos = orignalPos;
		}

		//Body
		glm::mat4 transPerson = glm::translate(glm::mat4(1.0f), personPos);
		GLuint transformLoc = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transPerson));

		GLuint colorLoc = glGetUniformLocation(programID, "color");
		glm::vec4 colorPerson = glm::vec4(0, 1.0f, 0, 1.0);
		glUniform4fv(colorLoc, 1, glm::value_ptr(colorPerson));

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

		//Head
		glm::mat4 transHead = glm::mat4(1.0f);
		transHead = glm::translate(transHead, glm::vec3(personPos.x, personPos.y + 0.1f, 0.0f)); 
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transHead));

		glm::vec4 headColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		glUniform4fv(colorLoc, 1, glm::value_ptr(headColor));

		glBindVertexArray(circleVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertexCount);

		//Monster
		glm::mat4 transEnemy = glm::translate(glm::mat4(1.0f), monsterPos);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transEnemy));

		glm::vec4 colorEnemy = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		glUniform4fv(colorLoc, 1, glm::value_ptr(colorEnemy));

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

		//Head
		glm::mat4 transEnemyHead = glm::mat4(1.0f);
		transEnemyHead = glm::translate(glm::mat4(1.0f), glm::vec3(monsterPos.x, monsterPos.y + 0.1f, 0.0f));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transEnemyHead));

		glm::vec4 colorEnemyHead = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		glUniform4fv(colorLoc, 1, glm::value_ptr(colorEnemyHead));

		glBindVertexArray(circleVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertexCount);

		//Bullets
		for (const auto& b : bullets) {
			if (!b.active) continue;

			glm::mat4 transBullet = glm::translate(glm::mat4(1.0f), b.pos);
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transBullet));

			glm::vec4 bulletColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
			glUniform4fv(colorLoc, 1, glm::value_ptr(bulletColor));

			glBindVertexArray(circleVAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertexCount);
		}

		glfwSwapBuffers(window);
	}


	glDeleteVertexArrays(1, &circleVAO);
	glDeleteBuffers(1, &circleVBO);

	// Cleanup
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}