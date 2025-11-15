// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <deque>

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

GLuint circleVAO, circleVBO, squareVAO, squareVBO, gunVAO, gunVBO, heartVAO, heartVBO, borderVAO, borderVBO;
GLsizei circleVertexCount = 0;

struct bullet {
	glm::vec3 pos;
	glm::vec3 dir;
	float speed = 1.0f;
	bool active;
	float rotation = 0.0f;
	float rotSpeed = 180.0f;
};
std::vector<bullet> bullets;
float lastShoot = 0.0f;
float interval = 1.0f;

struct collectible {
	glm::vec3 pos;
	bool active;
};
std::deque<collectible> collectibles;
float lastSpawn = 0.0f;
float spawnInterval = 1.0f;
int score = 0;
int collectedCount = 0;
float OFFSCREEN = 0.95f;

struct syringe {
	glm::vec3 pos;
	glm::vec3 dir;
	float speed = 1.5f;
	bool active;
	float rotation = 0.0f;
};
std::vector<syringe> syringes;
int syringeCount = 0;
bool monsterAlive = true;

glm::vec3 originalPos = glm::vec3(-0.5f, 0.0f, 0.0f);
glm::vec3 personPos = glm::vec3(-0.5f, 0.0f, 0.0f);
glm::vec3 monsterPos = glm::vec3(0.0f, 0.0f, 0.0f);
float speed = 0.001f; //Speed
bool playerHit = false;
float playerTimer = 0.0f;
bool moveUp = false;
bool moveDown = false;
bool moveLeft = false;
bool moveRight = false;

// Health Points
int playerHP = 3;
const int maxHP = 3;

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

void bulletToCollectible(const bullet& b) {
	collectible c;
	glm::vec3 p = b.pos;

	// Snap to the nearest wall depending on what was crossed
	p.x = glm::clamp(p.x, -OFFSCREEN, OFFSCREEN);
	p.y = glm::clamp(p.y, -OFFSCREEN, OFFSCREEN);

	c.pos = p;
	c.active = true;
	collectibles.push_back(c);
	if (collectibles.size() > 5)
		collectibles.pop_front();
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

void createBottle() {
	GLfloat bottleVertices[] = {
		-0.03f, -0.05f, 0.0f, 
		 0.03f, -0.05f, 0.0f, 
		 0.03f,  0.05f, 0.0f,
		-0.03f,  0.05f, 0.0f,  
	};

	GLuint bottleIndices[] = {
		0,1,2,
		0,2,3,
	};

	GLuint EBO;

	glGenVertexArrays(1, &squareVAO);
	glGenBuffers(1, &squareVBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(squareVAO);

	glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottleVertices), bottleVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bottleIndices), bottleIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void createSyringe() {
	float w = 0.1f; // length of syringe
	float h = 0.01f; // thickness
	// the syringe starts at origin (hand) and extends to the right (tip)
	GLfloat gunVertices[] = { 
		 0.0f, -h, 0.0f, // left-bottom (hand side)
		 w,   -h, 0.0f,  // right-bottom (tip)
		 w,    h, 0.0f,  // right-top (tip)
		 0.0f,  h, 0.0f   // left-top (hand side)
	};
	GLuint indices[] = {
		0, 1, 2, 
		0, 2, 3
	};

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

void createHearts() {
	GLfloat heartVertices[] = {
		// Top left lobe triangle
		-0.20f, 0.10f, 0.0f,  // left
		-0.05f, 0.30f, 0.0f,  // top
		0.00f, 0.10f, 0.0f,  // center

		// Top right lobe triangle
		0.20f, 0.10f, 0.0f,  // right
		0.05f, 0.30f, 0.0f,  // top
		0.00f, 0.10f, 0.0f,  // center

		// Bottom inverted triangle
		-0.20f, 0.10f, 0.0f,  // left
		0.20f, 0.10f, 0.0f,  // right
		0.00f, -0.25f, 0.0f  // bottom tip
	};

	GLuint heartIndices[] = {
		0, 1, 2,  // top left
		3, 4, 5,  // top right
		6, 7, 8   // bottom tip
	};

	GLuint EBO;

	glGenVertexArrays(1, &heartVAO);
	glGenBuffers(1, &heartVBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(heartVAO);

	glBindBuffer(GL_ARRAY_BUFFER, heartVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(heartVertices), heartVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(heartIndices), heartIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void createBorder() {
	// thin rectangle border (two triangles per side, total 4 sides = 8 triangles)
	float thickness = 0.01f;
	float w = 0.5f; // half-width
	float h = 0.1f; // half-height

	GLfloat borderVertices[] = {
		-w,  h, 0.0f,   w,  h, 0.0f,   w,  h - thickness, 0.0f,
		-w,  h, 0.0f,   w,  h - thickness, 0.0f,  -w, h - thickness, 0.0f, // top

		-w, -h + thickness, 0.0f,   w, -h + thickness, 0.0f,   w, -h, 0.0f,
		-w, -h + thickness, 0.0f,   w, -h, 0.0f,           -w, -h, 0.0f, // bottom

		-w,  h, 0.0f,   -w + thickness,  h, 0.0f,   -w + thickness, -h, 0.0f,
		-w,  h, 0.0f,   -w + thickness, -h, 0.0f,   -w, -h, 0.0f, // left

		w - thickness,  h, 0.0f,   w,  h, 0.0f,    w, -h, 0.0f,
		w - thickness,  h, 0.0f,   w, -h, 0.0f,   w - thickness, -h, 0.0f // right
	};

	glGenVertexArrays(1, &borderVAO);
	glGenBuffers(1, &borderVBO);

	glBindVertexArray(borderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);

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

	//// Create identity matrix for transforms
	//glm::mat4 trans = glm::mat4(1.0f);
	//trans = glm::translate(trans, personPos);

	//// Maybe we can play with different positions
	//glm::vec3 positions[] = {
	//	glm::vec3(0.0f,  0.0f,  0),
	//};

	createCircle();
	createBottle();
	createSyringe();
	createHearts();
	createBorder();

	// Time bookkeeping for frame delta
	float lastTime = (float)glfwGetTime();

	// Check if the window was closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programID);

		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		if (playerHit) {
			playerTimer -= deltaTime;
			if (playerTimer <= 0.0f) {
				playerHit = false;
			}
		}

		// Add bullets: shoot toward current personPos
		if (monsterAlive && currentTime - lastShoot > interval) {
			bullet b;
			glm::vec3 rawDir = personPos - monsterPos;	// Direction from monster to player
			b.dir = glm::normalize(rawDir);
			b.active = true;
			b.pos = monsterPos + b.dir * deltaTime;
			bullets.push_back(b);
			lastShoot = currentTime;
		}

		// Update bullets
		for (auto& b : bullets) {
			if (!b.active) 
				continue;
			b.pos += b.dir * b.speed * deltaTime;
			b.rotation += b.rotSpeed * deltaTime;
			// if bullet crosses the visible playfield, transform it into a collectible
			if (b.pos.x <= -OFFSCREEN || b.pos.x >= OFFSCREEN ||
				b.pos.y <= -OFFSCREEN || b.pos.y >= OFFSCREEN) {

				bulletToCollectible(b);
				b.active = false;
				continue;
			}

			// monster bullet: check collision with player
			if (glm::distance(b.pos, personPos) < 0.15f) {
				playerHit = true;
				playerTimer = 0.25;
				b.active = false;

				// health points implementation
				playerHP--;

				if (playerHP <= 0) {
					collectedCount = 0; // reset collectibles count on death (but number of syringes in the inventory is kept)
					playerHP = maxHP;
					personPos = originalPos;
					std::cout << "Player died, HP reset!" << std::endl;
				}
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
				std::cout << "Monster healed!" << std::endl;
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
			personPos = originalPos;
			playerHit = true;
			playerTimer = 0.25f;

			// health points implementation
			playerHP--;

			if (playerHP <= 0) {
				collectedCount = 0;
				playerHP = maxHP;
				personPos = originalPos;
				std::cout << "Player died, HP reset!" << std::endl;
			}
		}

		// Collision with collectibles
		for (auto& c : collectibles) {
			if (!c.active) continue;
			if (glm::distance(personPos, c.pos) < 0.12f) {
				c.active = false;
				score++;
				collectedCount++;
				std::cout << "Collectibles: " << collectedCount << std::endl;

				// every 5 collectibles = give 1 syringe
				while (collectedCount >= 5) {
					syringeCount++;
					collectedCount -= 5;
					std::cout << "Syringes available: " << syringeCount << std::endl;

					// Health regeneration if below maxHP
					if (playerHP < maxHP) {
						playerHP++;
						std::cout << "Health regenerated! Current HP: " << playerHP << std::endl;
					}
				}
			}
		}

		glm::vec4 colorPerson;
		if (playerHit) {
			colorPerson = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);// RED
		}
		else
			colorPerson = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // GREEN

		//Body
		glm::mat4 transPerson = glm::translate(glm::mat4(1.0f), personPos);
		GLuint transformLoc = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transPerson));

		GLuint colorLoc = glGetUniformLocation(programID, "color");
		glUniform4fv(colorLoc, 1, glm::value_ptr(colorPerson));

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

		//Head
		glm::mat4 transHead = glm::mat4(1.0f);
		transHead = glm::translate(transHead, glm::vec3(personPos.x, personPos.y + 0.1f, 0.0f)); 
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transHead));

		//glm::vec4 headColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		glUniform4fv(colorLoc, 1, glm::value_ptr(colorPerson));

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
			transBullet = glm::rotate(transBullet, b.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transBullet));

			glm::vec4 bulletColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
			glUniform4fv(colorLoc, 1, glm::value_ptr(bulletColor));

			glBindVertexArray(squareVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

		// Draw syringe (rotation working)
		if (syringeCount > 0) {
			glm::vec3 handPos = personPos + glm::vec3(0.08f, 0.03f, 0.0f);

			// Convert screen coordinates to OpenGL coordinates [-1, 1]
			int windowWidth, windowHeight;
			glfwGetWindowSize(window, &windowWidth, &windowHeight);
			float targetX = (mouseX / windowWidth) * 2.0f - 1.0f;
			float targetY = 1.0f - (mouseY / windowHeight) * 2.0f; // flip Y

			glm::vec3 toMouse = glm::vec3(targetX, targetY, 0.0f) - handPos;
			float angle = atan2(toMouse.y, toMouse.x) * 60; //idk why but sometimes it works like this

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

		// Draw hearts (HP bar)
		for (int i = 0; i < playerHP; i++) {
			glm::mat4 th = glm::mat4(1.0f); // hearts don't move with world transforms, so reseted transform to identity

			th = glm::translate(th, glm::vec3(-0.15f + i * 0.15f, 0.85f, 0.0f));
			th = glm::scale(th, glm::vec3(0.2f, 0.2f, 1.0f));

			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(th));

			glm::vec4 hpColor = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
			glUniform4fv(colorLoc, 1, glm::value_ptr(hpColor));

			glBindVertexArray(heartVAO);
			glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
		}

		// Draw syringe inventory
		float startX = -0.9f;   // left edge of row
		float startY = 0.85f;    // top of screen
		float spacingX = 0.1f;  // spacing
		float width = 0.2f;    // width of syringe
		float height = 5.0f;   // height of syringe

		for (int i = 0; i < syringeCount; i++) {
			glm::mat4 sIcon = glm::mat4(1.0f);
			sIcon = glm::translate(sIcon, glm::vec3(startX + i * spacingX, startY, 0.0f));
			sIcon = glm::scale(sIcon, glm::vec3(width, height, 1.0f)); // vertical

			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(sIcon));

			glm::vec4 sColor = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f); // green
			glUniform4fv(colorLoc, 1, glm::value_ptr(sColor));

			glBindVertexArray(gunVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		// Health bar border
		glm::vec4 borderColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // gray
		glUniform4fv(colorLoc, 1, glm::value_ptr(borderColor));

		glm::mat4 healthBorder = glm::mat4(1.0f);
		healthBorder = glm::translate(healthBorder, glm::vec3(0.0f, 0.85f, 0.0f));
		healthBorder = glm::scale(healthBorder, glm::vec3(0.5f, 0.75f, 1.0f)); // scale to fixed size

		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(healthBorder));
		glBindVertexArray(borderVAO);
		glDrawArrays(GL_TRIANGLES, 0, 48);

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