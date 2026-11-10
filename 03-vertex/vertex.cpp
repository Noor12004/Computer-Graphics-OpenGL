#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
using namespace std;
#define numVAOs 1
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint createShaderProgram() {
const char *vshaderSource =
"#version 430 \n"
"layout (location = 0) in float n; \n"
"out float result; \n"
"void main(void) \n"
"{ \n"
" gl_Position = vec4(0.1 * n - 1.0, 0.02 * (20.0 * n + 10.0) -1.0, 0.0, 1.0); \n"
" result = 20.0 * n + 10.0; \n"
"}";
const char *fshaderSource =
"#version 430 \n"
"in float result; \n"
"out vec4 color; \n"
"void main(void) \n"
"{ \n"
" color = vec4(result / 255.0, 0.0, 1.0, 1.0); \n"
"}";
GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(vShader, 1, &vshaderSource, NULL);
glShaderSource(fShader, 1, &fshaderSource, NULL);
glCompileShader(vShader);
glCompileShader(fShader);
GLuint vfProgram = glCreateProgram();
glAttachShader(vfProgram, vShader);
glAttachShader(vfProgram, fShader);
glLinkProgram(vfProgram);
return vfProgram;
}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram();
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	}
	void display(GLFWwindow* window, double currentTime) {
	glUseProgram(renderingProgram);
	// More sample values for 'n'
	float nValues[] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
	9.0, 10.0 };
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(nValues), nValues,
	GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_LINE_STRIP, 0, sizeof(nValues) /
	sizeof(nValues[0]));
	}
	int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600, 600, "Week 2 - Program 1", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);
	init(window);
	while (!glfwWindowShouldClose(window)) {
	display(window, glfwGetTime());
	glfwSwapBuffers(window);
	glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	}