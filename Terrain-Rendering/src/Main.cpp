//Context
#include <glad/glad.h>
#include <GLFW/include/GLFW/glfw3.h>

//Maths
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

//C++
#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>

//Headers
#include "Render/Shader.hpp"
#include "Render/Camera.hpp"

unsigned int Screen_width = 1920;
unsigned int Screen_Height = 1080;
float LO = -1.0f;
float HI = 1.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

std::vector<glm::vec3> vertices;
std::vector<unsigned int> indices;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = Screen_width / 2.0f;
float lastY = Screen_Height / 2.0f;
bool firstMouse = true;


float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct Transform {

	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);
	glm::mat4 to_mat4() {
		glm::mat4 m = glm::translate(glm::mat4(1), position);
		m *= glm::mat4_cast(glm::quat(rotation));
		m = glm::scale(m, scale);
		return m;
	};
};

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(Screen_width, Screen_Height, "Terrain Rendering", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "GLFW Context is incorrect...You should fix it" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD isn't initalized properly" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader;
	shader.Load("Resource/Shader/shader.vert", "Resource/Shader/shader.frag");

	int indexOffset = 0;
	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 10; y++) {
			vertices.push_back(glm::vec3(x - 0.5, -0.5 + y, -5)); // BOTTOMLEFT
			vertices.push_back(glm::vec3(x - 0.5, 0.5 + y, -5)); // TOPLEFT
			vertices.push_back(glm::vec3(x + 0.5, -0.5 + y, -5)); // TOPRIGHT
			vertices.push_back(glm::vec3(x + 0.5, 0.5 + y, -5)); // BOTTOMRIGHT
			indices.push_back(2 + indexOffset);
			indices.push_back(1 + indexOffset);
			indices.push_back(0 + indexOffset);
			indices.push_back(2 + indexOffset);
			indices.push_back(3 + indexOffset);
			indices.push_back(1 + indexOffset);
			indexOffset += 4;
		}
	}

	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	for (auto index : indices) {
		std::cout << index << " ";
	}

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		//Render
		glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		shader.use();

		float nearPlane = 0.01f;
		float farPlane = 100.0f;
		float aspectRatio = (float)Screen_width / (float)Screen_Height;
		//glm::mat4 projection = glm::perspective(1.0f, aspectRatio, nearPlane, farPlane);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)Screen_width / (float)Screen_Height, 0.1f, 200.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 1.0f));
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	Screen_width = width;
	Screen_Height = height;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}