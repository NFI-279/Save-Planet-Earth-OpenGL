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

GLuint circleVAO, circleVBO, squareVAO, squareVBO, gunVAO, gunVBO;
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

struct collectible {
	glm::vec3 pos;
	bool active;
};
std::vector<collectible> collectibles;
float lastSpawn = 0.0f;
float spawnInterval = 1.0f;
int score = 0;
int collectedCount = 0; // counts all collectibles collected

struct syringe {
	glm::vec3 pos;
	glm::vec3 dir;
	float speed = 1.5f;
	bool active;
	float rotation;
};
std::vector<syringe> syringes;
int syringeCount = 0; // number of syringes player has
bool monsterAlive = true; // bullets stop when monster is defeated

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

bool mousePressed = false;
double mouseX, mouseY;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mousePressed = true;
		glfwGetCursorPos(window, &mouseX, &mouseY); // store mouse coordinates
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	mouseX = xpos;
	mouseY = ypos;
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

void spawnCollectible() {
	collectible c;
	// random position in range [-0.9, 0.9]
	c.pos = glm::vec3(
		((float)rand() / RAND_MAX) * 1.8f - 0.9f,
		((float)rand() / RAND_MAX) * 1.8f - 0.9f,
		0.0f
	);
	c.active = true;
	collectibles.push_back(c);
}

void createSquare() {
	float size = 0.05f;
	GLfloat squareVertices[] = {
		-size, -size, 0.0f,
		 size, -size, 0.0f,
		 size,  size, 0.0f,
		-size,  size, 0.0f
	};
	GLuint indices[] = { 0, 1, 2, 0, 2, 3 };

	GLuint squareEBO;
	glGenVertexArrays(1, &squareVAO);
	glGenBuffers(1, &squareVBO);
	glGenBuffers(1, &squareEBO);

	glBindVertexArray(squareVAO);
	glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void createGun() {
	float w = 0.1f; // length of syringe
	float h = 0.01f; // thickness
	// the syringe starts at origin (hand) and extends to the right (tip)
	GLfloat gunVertices[] = {
		 0.0f, -h, 0.0f, // left-bottom (hand side)
		 w,   -h, 0.0f,  // right-bottom (tip)
		 w,    h, 0.0f,  // right-top (tip)
		 0.0f,  h, 0.0f   // left-top (hand side)
	};
	GLuint indices[] = { 0, 1, 2, 0, 2, 3 };

	GLuint gunEBO;
	glGenVertexArrays(1, &gunVAO);
	glGenBuffers(1, &gunVBO);
	glGenBuffers(1, &gunEBO);

	glBindVertexArray(gunVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gunVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gunVertices), gunVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gunEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

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

	createSquare();

	createGun(); // aka syringe

	// Time bookkeeping for frame delta
	float lastTime = (float)glfwGetTime();

	// Check if the window was closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programID);

		//Add bulets
		float currentTime = glfwGetTime();

		// Compute deltaTime
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Add bullets: shoot toward current personPos
		if (monsterAlive && currentTime - lastShoot > interval) {
			bullet b;
			// direction pointing from monster to person
			glm::vec3 rawDir = personPos - monsterPos;
			if (glm::length(rawDir) < 1e-6f) {
				// fallback direction if on top of each other
				rawDir = glm::vec3(-1.0f, 0.0f, 0.0f);
			}
			b.dir = glm::normalize(rawDir);
			// spawn slightly in front of monster
			b.pos = monsterPos + b.dir * 0.08f;
			b.active = true;
			b.speed = 1.2f;
			bullets.push_back(b);
			lastShoot = currentTime;
		}

		// Update bullets
		for (auto& b : bullets) {
			if (!b.active) continue;

			b.pos += b.dir * b.speed * deltaTime;

			// deactivate offscreen
			if (b.pos.x < -1.2f || b.pos.x > 1.2f || b.pos.y < -1.2f || b.pos.y > 1.2f)
				b.active = false;

			// monster bullet: check collision with player
			if (glm::distance(b.pos, personPos) < 0.15f) {
				personPos = orignalPos;
				b.active = false;
			}
		}

		if (mousePressed && syringeCount > 0) {
			// Convert screen coordinates to OpenGL coordinates [-1, 1]
			int windowWidth, windowHeight;
			glfwGetWindowSize(window, &windowWidth, &windowHeight);
			float targetX = (mouseX / windowWidth) * 2.0f - 1.0f;
			float targetY = 1.0f - (mouseY / windowHeight) * 2.0f; // flip Y

			syringe s;
			s.pos = personPos + glm::vec3(0.08f, 0.03f, 0.0f); // start at hand
			glm::vec3 target = glm::vec3(targetX, targetY, 0.0f);
			glm::vec3 dir = target - s.pos;

			if (glm::length(dir) < 1e-6f)
				dir = glm::vec3(1.0f, 0.0f, 0.0f); // fallback

			s.dir = glm::normalize(dir);
			s.active = true;
			s.rotation = atan2(dir.y, dir.x);

			syringes.push_back(s);
			syringeCount--; // consume

			std::cout << "Syringe shot! Remaining: " << syringeCount << std::endl;

			mousePressed = false; // reset
		}

		// Update syringes
		for (auto& s : syringes) {
			if (!s.active) continue;

			s.pos += s.dir * s.speed * deltaTime;

			// deactivate offscreen
			if (s.pos.x < -1.2f || s.pos.x > 1.2f || s.pos.y < -1.2f || s.pos.y > 1.2f)
				s.active = false;

			// collision with monster
			if (monsterAlive && glm::distance(s.pos, monsterPos) < 0.15f) {
				monsterAlive = false;
				s.active = false;
				std::cout << "Monster defeated!" << std::endl;
			}
		}

		// Clean up inactive syringes
		syringes.erase(
			std::remove_if(syringes.begin(), syringes.end(),
				[](const syringe& s) { return !s.active; }),
			syringes.end()
		);

		// Last implementation of bullets
		/*if (currentTime - lastShoot > interval) {
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
		}*/

		// Spawn collectibles
		if (currentTime - lastSpawn > spawnInterval) {
			spawnCollectible();
			lastSpawn = currentTime;
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

		// Collision with collectibles
		for (auto& c : collectibles) {
			if (!c.active) continue;
			if (glm::distance(personPos, c.pos) < 0.12f) {
				c.active = false;
				score++;
				collectedCount++;
				std::cout << "Score: " << score << std::endl;

				// every 5 collectibles = give 1 syringe
				while (collectedCount >= 5) {
					syringeCount++;
					collectedCount -= 5;
					std::cout << "Syringes available: " << syringeCount << std::endl;
				}
			}
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

		// Draw bullets
		for (const auto& b : bullets) {
			if (!b.active) continue;

			glm::mat4 transBullet = glm::translate(glm::mat4(1.0f), b.pos);
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transBullet));

			glm::vec4 bulletColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
			glUniform4fv(colorLoc, 1, glm::value_ptr(bulletColor));

			glBindVertexArray(circleVAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertexCount);
		}

		// Draw collectibles (squares)
		for (const auto& c : collectibles) {
			if (!c.active) continue;

			glm::mat4 transC = glm::translate(glm::mat4(1.0f), c.pos);
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transC));

			glm::vec4 cColor = glm::vec4(0.0f, 0.7f, 1.0f, 1.0f); // blue-ish
			glUniform4fv(colorLoc, 1, glm::value_ptr(cColor));

			glBindVertexArray(squareVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		// Draw syringes (dynamic rotation not working properly)
		if (syringeCount > 0) {
			glm::vec3 handPos = personPos + glm::vec3(0.08f, 0.03f, 0.0f);

			// Convert screen coordinates to OpenGL coordinates [-1, 1]
			int windowWidth, windowHeight;
			glfwGetWindowSize(window, &windowWidth, &windowHeight);
			float targetX = (mouseX / windowWidth) * 2.0f - 1.0f;
			float targetY = 1.0f - (mouseY / windowHeight) * 2.0f; // flip Y

			glm::vec3 toMouse = glm::vec3(targetX, targetY, 0.0f) - handPos;
			float angle = atan2(toMouse.y, toMouse.x);

			glm::mat4 transGun = glm::mat4(1.0f);
			transGun = glm::translate(transGun, handPos);
			transGun = glm::rotate(transGun, angle, glm::vec3(0.0f, 0.0f, 1.0f));

			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transGun));

			glm::vec4 gunColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			glUniform4fv(colorLoc, 1, glm::value_ptr(gunColor));

			glBindVertexArray(gunVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		for (const auto& s : syringes) {
			if (!s.active) continue;

			glm::mat4 transSyringe = glm::mat4(1.0f);
			transSyringe = glm::translate(transSyringe, s.pos); // move to current position
			transSyringe = glm::rotate(transSyringe, s.rotation, glm::vec3(0.0f, 0.0f, 1.0f)); // rotate

			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transSyringe));

			glm::vec4 syringeColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			glUniform4fv(colorLoc, 1, glm::value_ptr(syringeColor));

			glBindVertexArray(gunVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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