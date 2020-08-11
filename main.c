#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct {
	double x, y;
} Vec2;

unsigned char palletData[][3] = {
#if 0
	{   0,   0,   0 },
	{ 200,   0,   0 },
	{ 252, 200,   0 },
	{ 252, 252, 252 },
	{ 252, 200,   0 },
	{ 200,   0,   0 }
#else
	{   0,   7, 100 },
	{  32, 107, 203 },
	{ 237, 255, 255 },
	{ 255, 170,   0 },
	{ 106,  53,   3 }
#endif
};

float aspect = 800 / 600;
double maxIter  = 255;
Vec2 center     = { -0.5, 0.0 };
float  rotation = 0;
double zoom     = 2.0;

Vec2 sCenter     = { -0.5, 0.0 };
float  sRotation = 0;
double sZoom     = 3.0;

static void callback_framebuffer_size(GLFWwindow* window, int width, int height);
static void die(const char *errstr, ...);
static double lerp(double a, double b, double t);
static char *file_read(char const *filename);
static GLuint createShader(char const *vertSource, char const *fragSource);

int
main(void)
{
	/* glfw */
	if (!glfwInit())
		die("GLFW failed to initialize\n");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *wnd = glfwCreateWindow(800, 600, "Mandelbrot", NULL, NULL);
	if (!wnd)
		die("GLFW failed to create window\n");

	glfwMakeContextCurrent(wnd);
	glfwSetFramebufferSizeCallback(wnd, callback_framebuffer_size);

	/* glew */
	glewExperimental = 1;
	if (glewInit() != GLEW_OK && GL_ARB_gpu_shader_fp64)
		die("GLEW failed to initialize\n");

	/* shaders */
	GLuint VBO, VAO;
	float vertices[] = {
		-1.0f, -1.0f,
		-1.0f,  3.0f,
		 3.0f, -1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
	glEnableVertexAttribArray(0);

	char *fragmentSource = file_read("mandel.glsl");
	if (!fragmentSource)
		die("Failed to read fragment shader: main.glsl\n");
	GLuint shader = createShader(
		"#version 330 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"out vec2 pos;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos, 0.0, 1.0);\n"
		"	pos = aPos.xy;\n"
		"}\n",
		fragmentSource);
	free(fragmentSource);

	glUseProgram(shader);

	/* create palette */
	GLuint pallet;
	glGenTextures(1, &pallet);
	glBindTexture(GL_TEXTURE_1D, pallet);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB,
	             sizeof palletData / sizeof *palletData, 0,
	             GL_RGB, GL_UNSIGNED_BYTE, palletData);

	/* uniform locations */
	GLuint maxIterLoc = glGetUniformLocation(shader, "maxIter");
	GLuint centerLoc = glGetUniformLocation(shader, "center");
	GLuint rotationLoc = glGetUniformLocation(shader, "rotation");
	GLuint zoomLoc = glGetUniformLocation(shader, "zoom");

	float lastTime = 0, dt = 0;

	while (!glfwWindowShouldClose(wnd)) {
		double const scalar = dt * zoom;
		Vec2 dir = {0};

		/* time */
		double const currTime = glfwGetTime();
		dt = currTime - lastTime;
		lastTime = currTime;

		/* input */
		if (glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(wnd, 1);

		if (glfwGetKey(wnd, GLFW_KEY_J))
			zoom -= scalar;
		if (glfwGetKey(wnd, GLFW_KEY_K))
			zoom += scalar;

		dir.x = scalar * cos(rotation);
		dir.y = scalar * sin(rotation);

		if (glfwGetKey(wnd, GLFW_KEY_A)) {
			center.x -= dir.x;
			center.y -= dir.y;
		}
		if (glfwGetKey(wnd, GLFW_KEY_D)) {
			center.x += dir.x;
			center.y += dir.y;
		}
		if (glfwGetKey(wnd, GLFW_KEY_S)) {
			center.x += dir.y;
			center.y -= dir.x;
		}
		if (glfwGetKey(wnd, GLFW_KEY_W)) {
			center.x -= dir.y;
			center.y += dir.x;
		}

		if (glfwGetKey(wnd, GLFW_KEY_E))
			maxIter += dt * 100.0;
		if (glfwGetKey(wnd, GLFW_KEY_Q))
			maxIter -= dt * 100.0;
		if (maxIter < 2)
			maxIter = 2;

		if (glfwGetKey(wnd, GLFW_KEY_H))
			rotation += dt * 2.0;
		if (glfwGetKey(wnd, GLFW_KEY_L))
			rotation -= dt * 2.0;

		if (glfwGetKey(wnd, GLFW_KEY_SPACE)) {
			center.x = -0.5;
			center.y =  0.0;
			rotation = 0.0;
			zoom = 2.0;
		}
		if (glfwGetKey(wnd, GLFW_KEY_C)) {
			center.x = sCenter.x;
			center.y = sCenter.y;
			rotation = sRotation;
			zoom = sZoom;
		}

		/* update uniforms */
		sCenter.x = lerp(sCenter.x, center.x, dt * 2.0);
		sCenter.y = lerp(sCenter.y, center.y, dt * 2.0);
		sRotation = lerp(sRotation, rotation, dt * 1.5);
		sZoom     = lerp(sZoom,     zoom,     dt * 1.5);

		glUniform1ui(maxIterLoc, maxIter);
		glUniform2d(centerLoc, sCenter.x, sCenter.y);
		glUniform1f(rotationLoc, sRotation);
		if (aspect > 1)
			glUniform2d(zoomLoc, sZoom, sZoom / aspect);
		else
			glUniform2d(zoomLoc, sZoom * aspect, sZoom);

		/* render */
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}

	return EXIT_SUCCESS;
}

static void
callback_framebuffer_size(GLFWwindow* window, int width, int height)
{
	(void)window;
	aspect = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

static void
die(const char *errstr, ...)
{
	va_list args;
	va_start(args, errstr);
	vfprintf(stderr, errstr, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

static double
lerp(double a, double b, double t)
{
	return a + (b - a) * t;
}

static char *
file_read(char const *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f) return NULL;

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	char *str = malloc(size + 1);
	fread(str, sizeof *str, size, f);
	str[size] = 0;

	fclose(f);

	return str;
}

static char logBuffer[1024];

static GLuint
compileShaders(char const *source, GLenum type)
{
	int success;
	GLuint shader = glCreateShader(type);
	if (!shader)
		die("failed to create shader\n");

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(
				shader,
				sizeof logBuffer / sizeof *logBuffer,
				NULL, logBuffer);
		die("Failed to compile shader:\n%s\n\n%s\n", source, logBuffer);
	}
	return shader;
}

static GLuint
createShader(char const *vert, char const *frag)
{
	int success;
	GLuint program = glCreateProgram();
	GLuint vertShader = compileShaders(vert, GL_VERTEX_SHADER);
	GLuint fragShader = compileShaders(frag, GL_FRAGMENT_SHADER);

	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glValidateProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(
				program,
				sizeof logBuffer / sizeof *logBuffer,
				NULL, logBuffer);
		die("Failed to link program:\n%s\n", logBuffer);
	}
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}
