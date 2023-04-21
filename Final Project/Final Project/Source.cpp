#include <GLEW\glew.h>
#include <GLFW\glfw3.h>
#include <iostream>

// GLM Library
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

using namespace std;

int width, height;

void init(GLFWwindow* window) {}

// Input Callback Function Prototypes
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Process Input Prototype
void processInput(GLFWwindow* window);

// Declare view matrix
glm::mat4 viewMatrix = glm::mat4(1.0f);

// Define perspective or ortho
bool is3D = true;

// Declare getProjection prototype
glm::mat4 getProjection();

// Reset camera function prototype
void resetCamera();

// Define Camera Attributes
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 12.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

// Pitch and Yaw
GLfloat rawYaw = -90.0f;
GLfloat rawPitch = 0.0f;
GLfloat degYaw, degPitch;

// Define variables so that object speed is not determined by processor speed
GLfloat deltaTime, lastFrame = 0.0f;
GLfloat lastX = width / 2;
GLfloat lastY = height / 2;
GLfloat xOffset, yOffset;

// Detect initial mouse movement
bool firstMouseMove = true;

// Define camera speed
GLfloat speedModifier = 10.0f;

// Light Source Position
glm::vec3 lightPosition1(0.0f, 6.0f, 3.0f);
glm::vec3 lightPosition2(6.0f, 6.0f, -5.0f);
glm::vec3 lightPosition3(-6.0f, 6.0f, 0.0f);

// Nut Texture List
GLuint nutTexList[24];

// Draw primitive(s)
void draw(GLsizei indices) {
	GLenum mode = GL_TRIANGLES;

	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);
}

// Create and compile shaders
static GLuint compileShader(const string& source, GLuint type) {
	// Create shader object
	GLuint shaderID = glCreateShader(type);
	const char* src = source.c_str();

	// Attach source code to shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile shader
	glCompileShader(shaderID);

	// Return ID of compiled shader
	return shaderID;
}

// Create program object
static GLuint createShaderProgram(const string& vertexShader, const string& fragmentShader) {
	// Compile vertex shader
	GLuint vShader = compileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fShader = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);

	// Link program to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	// Return shader program
	return shaderProgram;
}

int main(void) {
	width = 800;
	height = 600;

	if (!glfwInit()) { exit(EXIT_FAILURE); }

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);

	// Set Input Callback Functions
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// Capture mouse for input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }

	glfwSwapInterval(1);

	// Square Vertex Data
	GLfloat squareVertices[]{
		-0.5f,  0.0f, -0.5f, // Vert 0
		 0.0f,  1.0f,  0.0f, // Green
		 0.0f,  0.0f,        // UV (BL)
		 0.0f,  1.0f,  0.0f, // Normal: Positive Y

		 0.5f,  0.0f, -0.5f, // Vert 1
		 0.0f,  0.0f,  1.0f, // Blue
		 1.0f,  0.0f,        // UV (BR)
		 0.0f,  1.0f,  0.0f, // Normal: Positive Y

		 0.5f,  0.0f,  0.5f, // Vert 2
		 1.0f,  0.0f,  0.0f, // Red
		 1.0f,  1.0f,        // UV (TR)
		 0.0f,  1.0f,  0.0f, // Normal: Positive Y

		-0.5f,  0.0f,  0.5f, // Vert 3
		 1.0f,  1.0f,  0.0f, // Yellow
		 0.0f,  1.0f,        // UV (TL)
		 0.0f,  1.0f,  0.0f  // Normal: Positive Y
	};

	// Square Element Indices Data
	GLubyte squareIndices[] = {
		0, 1, 2, // Triangle 1
		0, 2, 3 // Triangle 2
	};

	// Laptop Base Positions
	glm::vec3 basePositions[] = {
		glm::vec3(0.0f, 0.0f, 0.0f),    // Bottom
		glm::vec3(0.0f, 0.125f, -2.5f), // Back
		glm::vec3(0.0f, 0.25f, 0.0f),   // Top
		glm::vec3(0.0f, 0.125f, 2.5f),  // Front
		glm::vec3(-4.0f, 0.125f, 0.0f), // Left
		glm::vec3(4.0f, 0.125f, 0.0f)   // Right
	};

	// Laptop Base Rotations
	glm::float32 baseRotationsX[] = {
		 0.0f, // Bottom
		90.0f, // Back
		 0.0f, // Top
		90.0f, // Front
		 0.0f, // Left
		 0.0f  // Right
	};

	glm::float32 baseRotationsZ[] = {
		  0.0f, // Bottom
		  0.0f, // Back
		  0.0f, // Top
		  0.0f, // Front
		 90.0f, // Left
		-90.0f  // Right
	};

	// Laptop Base Scaling
	glm::vec3 baseScaling[] = {
		glm::vec3(8.0f, 1.0f, 5.0f),  // Bottom
		glm::vec3(8.0f, 1.0f, 0.25f), // Back
		glm::vec3(8.0f, 1.0f, 5.0f),  // Top
		glm::vec3(8.0f, 1.0f, 0.25f), // Front
		glm::vec3(0.25f, 1.0f, 5.0f), // Left
		glm::vec3(0.25f, 1.0f, 5.0f)  // Right
	};

	// Laptop Monitor Positions
	glm::vec3 monitorPositions[] = {
		glm::vec3(0.0f, 0.25f, -2.45f),  // Bottom
		glm::vec3(0.0f, 2.75f, -2.5f),   // Back
		glm::vec3(0.0f, 5.25f, -2.45f),  // Top
		glm::vec3(0.0f, 2.75f, -2.4f),   // Front
		glm::vec3(-4.0f, 2.75f, -2.45f), // Left
		glm::vec3(4.0f, 2.75f, -2.45f)   // Right
	};

	// Laptop Monitor Rotations
	glm::float32 monitorRotationsX[] = {
		  0.0f, // Bottom
		270.0f, // Back
		  0.0f, // Top
		 90.0f, // Front
		  0.0f, // Left
		  0.0f  // Right
	};

	glm::float32 monitorRotationsZ[] = {
		  0.0f, // Bottom
		  0.0f, // Back
		  0.0f, // Top
		  0.0f, // Front
		 90.0f, // Left
		-90.0f  // Right
	};

	// Laptop Monitor Scaling
	glm::vec3 monitorScaling[] = {
		glm::vec3(8.0f, 1.0f, 0.1f), // Bottom
		glm::vec3(8.0f, 1.0f, 5.0f), // Back
		glm::vec3(8.0f, 1.0f, 0.1f), // Top
		glm::vec3(8.0f, 1.0f, 5.0f), // Front
		glm::vec3(5.0f, 1.0f, 0.1f), // Left
		glm::vec3(5.0f, 1.0f, 0.1f)  // Right
	};

	// Teabox Positions
	glm::vec3 teaboxPositions[] = {
		glm::vec3(7.0f, 0.0f, -0.5f), // Bottom
		glm::vec3(7.0f, 0.5f, -1.0f), // Back
		glm::vec3(7.0f, 1.0f, -0.5f), // Top
		glm::vec3(7.0f, 0.5f, 0.0f),  // Front
		glm::vec3(6.0f, 0.5f, -0.5f), // Left
		glm::vec3(8.0f, 0.5f, -0.5f)  // Right
	};

	// Teabox Rotations
	glm::float32 teaboxRotationsX[] = {
		180.0f, // Bottom
		270.0f, // Back
		  0.0f, // Top
		 90.0f, // Front
		 90.0f, // Left
		 90.0f  // Right
	};

	glm::float32 teaboxRotationsZ[] = {
		  0.0f, // Bottom
		  0.0f, // Back
		  0.0f, // Top
		  0.0f, // Front
		 90.0f, // Left
		-90.0f  // Right
	};

	// Teabox Scaling
	glm::vec3 teaboxScaling[] = {
		glm::vec3(2.0f, 1.0f, 1.0f), // Bottom
		glm::vec3(2.0f, 1.0f, 1.0f), // Back
		glm::vec3(2.0f, 1.0f, 1.0f), // Top
		glm::vec3(2.0f, 1.0f, 1.0f), // Front
		glm::vec3(1.0f, 1.0f, 1.0f), // Left
		glm::vec3(1.0f, 1.0f, 1.0f)  // Right
	};

	// Tea Bottle Positions
	glm::vec3 teaBottlePositions[] = {
		glm::vec3(-6.0f, 0.0f, -2.5f),  // Bottom
		glm::vec3(-6.0f, 0.75f, -3.0f),   // Back
		glm::vec3(-6.0f, 1.5f, -2.5f),  // Top
		glm::vec3(-6.0f, 0.75f, -2.0f),   // Front
		glm::vec3(-6.5f, 0.75f, -2.5f), // Left
		glm::vec3(-5.5f, 0.75f, -2.5f)   // Right
	};

	// Tea Bottle Rotations
	glm::float32 teaBottleRotationsX[] = {
		  0.0f, // Bottom
		270.0f, // Back
		  0.0f, // Top
		 90.0f, // Front
		  0.0f, // Left
		  0.0f  // Right
	};

	glm::float32 teaBottleRotationsZ[] = {
		  0.0f, // Bottom
		  0.0f, // Back
		  0.0f, // Top
		  0.0f, // Front
		 90.0f, // Left
		-90.0f  // Right
	};

	// Tea Bottle Scaling
	glm::vec3 teaBottleScaling[] = {
		glm::vec3(1.0f, 1.0f, 1.0f), // Bottom
		glm::vec3(1.0f, 1.0f, 1.5f), // Back
		glm::vec3(1.0f, 1.0f, 1.0f), // Top
		glm::vec3(1.0f, 1.0f, 1.5f), // Front
		glm::vec3(1.5f, 1.0f, 1.0f), // Left
		glm::vec3(1.5f, 1.0f, 1.0f)  // Right
	};

	// Pyramid Vertex Data
	GLfloat pyramidVertices[]{
		 0.5f,  0.0f,  0.5f, // Vert 0
		 1.0f,  0.0f,  0.0f, // Red
		 1.0f,  0.0f,        // UV (BR)
		 0.0f,  0.447214f, 0.894427f, // Normal

		-0.5f,  0.0f,  0.5f, // Vert 1
		 1.0f,  1.0f,  0.0f, // Yellow
		 0.0f,  0.0f,        // UV (BL)
		 0.0f,  0.447214f, 0.894427f, // Normal

		 0.0f,  1.0f,  0.0f, // Vert 3
		 0.0f,  0.0f,  0.0f, // Black
		 0.5f,  1.0f,        // UV (TM)
		 0.0f,  0.447214f, 0.894427f  // Normal
	};

	// Pyramid Element Indices Data
	GLubyte pyramidIndices[] = {
		0, 1, 2 // Triangle 1
	};

	// Pyramid rotations: Y-axis
	glm::float32 pyramidRotationsY[] = {
		0.0f, 90.0f, 180.0f, 270.0f
	};

	// Cylinder Vertex Data
	GLfloat cylinderVertices[] = {
		// Base
		0.0f,  0.0f, 0.0f, // Vert 0
		1.0f,  1.0f, 1.0f, // White
		0.5f,  1.0f,       // UV (TM)
		0.0f, -1.0f, 0.0f, // Normal: Negative Y

		-sinf(glm::radians(7.5)), 0.0f, cosf(glm::radians(7.5)), // Vert 1
		1.0f,  1.0f, 1.0f, // White
		0.0f,  0.0f,       // UV (BL)
		0.0f, -1.0f, 0.0f, // Normal: Negative Y

		sinf(glm::radians(7.5)), 0.0f, cosf(glm::radians(7.5)), // Vert 2
		0.0f,  0.0f, 0.0f, // Black
		1.0f,  0.0f,       // UV (BR)
		0.0f, -1.0f, 0.0f, // Normal: Negative Y

		// Side
		-sinf(glm::radians(7.5)), 0.0f, cosf(glm::radians(7.5)), // Vert 3
		1.0f,  0.0f, 0.0f, // Red
		0.0f,  0.0f,       // UV (BL)
		0.0f,  0.0f, 1.0f, // Normal: Positive Z

		-sinf(glm::radians(7.5)), 1.0f, cosf(glm::radians(7.5)), // Vert 4
		1.0f,  0.0f, 0.0f, // Red
		0.0f,  1.0f,       // UV (TL)
		0.0f,  0.0f, 1.0f, // Normal: Positive Z

		sinf(glm::radians(7.5)), 0.0f, cosf(glm::radians(7.5)), // Vert 5
		0.0f,  1.0f, 0.0f, // Green
		1.0f,  0.0f,       // UV (BR)
		0.0f,  0.0f, 1.0f, // Normal: Positive Z

		sinf(glm::radians(7.5)), 1.0f, cosf(glm::radians(7.5)), // Vert 6
		0.0f,  1.0f, 0.0f, // Green
		1.0f,  1.0f,       // UV (TR)
		0.0f,  0.0f, 1.0f, // Normal: Positive Z

		// Top
		-sinf(glm::radians(7.5)), 1.0f, cosf(glm::radians(7.5)), // Vert 7
		1.0f,  1.0f, 1.0f, // White
		0.0f,  1.0f,       // UV (TL)
		0.0f,  1.0f, 0.0f, // Normal: Positive Y

		sinf(glm::radians(7.5)), 1.0f, cosf(glm::radians(7.5)), // Vert 8
		0.0f,  0.0f, 0.0f, // Black
		1.0f,  1.0f,       // UV (TR)
		0.0f,  1.0f, 0.0f, // Normal: Positive Y

		0.0f,  1.0f, 0.0f, // Vert 9
		1.0f,  1.0f, 1.0f, // White
		0.5f,  0.0f,       // UV (BM)
		0.0f,  1.0f, 0.0f  // Normal: Positive Y
	};

	// Cylinder Element Indices Data
	GLubyte cylinderIndices[] = {
		0, 1, 2, // Triangle 1
		3, 4, 5, // Triangle 2
		4, 5, 6, // Triangle 3
		7, 8, 9  // Triangle 4
	};

	// Lamp Vertex Data
	GLfloat lampVertices[] = {
		-0.5f,  0.5f, 0.0f, // Vert 0
		-0.5f, -0.5f, 0.0f, // Vert 1
		 0.5f, -0.5f, 0.0f, // Vert 2
		 0.5f,  0.5f, 0.0f  // Vert 3
	};

	// Lamp positions
	glm::vec3 lampPositions[] = {
		glm::vec3(0.0f, 0.0f, 0.5f),
		glm::vec3(0.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -0.5f),
		glm::vec3(-0.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.5f, 0.0f),
		glm::vec3(0.0f, -0.5f, 0.0f)
	};

	// Lamp rotations
	glm::float32 lampRotations[] = {
		0.0f, 90.0f, 180.0f, -90.0f, -90.0f, 90.0f
	};

	GLuint squareVAO, squareVBO, squareEBO;
	glGenVertexArrays(1, &squareVAO); // Create VAO
	glGenBuffers(1, &squareVBO); // Create VBO
	glGenBuffers(1, &squareEBO); // Create EBO

	glBindVertexArray(squareVAO); // Bind VAO

	glBindBuffer(GL_ARRAY_BUFFER, squareVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW); // Load element indices

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VAO

	GLuint pyramidVAO, pyramidVBO, pyramidEBO;
	glGenVertexArrays(1, &pyramidVAO); // Create VAO
	glGenBuffers(1, &pyramidVBO); // Create VBO
	glGenBuffers(1, &pyramidEBO); // Create EBO

	glBindVertexArray(pyramidVAO); // Bind VAO

	glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW); // Load element indices

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VAO

	GLuint cylinderVAO, cylinderVBO, cylinderEBO;
	glGenVertexArrays(1, &cylinderVAO); // Create VAO
	glGenBuffers(1, &cylinderVBO); // Create VBO
	glGenBuffers(1, &cylinderEBO); // Create EBO

	glBindVertexArray(cylinderVAO); // Bind VAO

	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderVertices), cylinderVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylinderIndices), cylinderIndices, GL_STATIC_DRAW); // Load element indices

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VAO

	GLuint lampVAO, lampVBO, lampEBO;
	glGenVertexArrays(1, &lampVAO); // Create VAO
	glGenBuffers(1, &lampVBO); // Create VBO
	glGenBuffers(1, &lampEBO); // Create EBO

	glBindVertexArray(lampVAO); // Bind VAO

	glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lampEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW); // Load element indices

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);

	// Wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Load Textures
	int keyboardTexWidth, keyboardTexHeight;
	unsigned char* keyboardImage = SOIL_load_image("keyboardEdit.jpg", &keyboardTexWidth, &keyboardTexHeight, 0, SOIL_LOAD_RGB);

	int laptop_lidTexWidth, laptop_lidTexHeight;
	unsigned char* laptop_lidImage = SOIL_load_image("laptop_lidEdit.jpg", &laptop_lidTexWidth, &laptop_lidTexHeight, 0, SOIL_LOAD_RGB);

	int laptop_rimTexWidth, laptop_rimTexHeight;
	unsigned char* laptop_rimImage = SOIL_load_image("laptop_rim.jpg", &laptop_rimTexWidth, &laptop_rimTexHeight, 0, SOIL_LOAD_RGB);

	int monitorTexWidth, monitorTexHeight;
	unsigned char* monitorImage = SOIL_load_image("monitorEdit.jpg", &monitorTexWidth, &monitorTexHeight, 0, SOIL_LOAD_RGB);

	int teabox_backTexWidth, teabox_backTexHeight;
	unsigned char* teabox_backImage = SOIL_load_image("teabox_backEdit.jpg", &teabox_backTexWidth, &teabox_backTexHeight, 0, SOIL_LOAD_RGB);

	int teabox_bottomTexWidth, teabox_bottomTexHeight;
	unsigned char* teabox_bottomImage = SOIL_load_image("teabox_bottomEdit.jpg", &teabox_bottomTexWidth, &teabox_bottomTexHeight, 0, SOIL_LOAD_RGB);

	int teabox_frontTexWidth, teabox_frontTexHeight;
	unsigned char* teabox_frontImage = SOIL_load_image("teabox_frontEdit.jpg", &teabox_frontTexWidth, &teabox_frontTexHeight, 0, SOIL_LOAD_RGB);

	int teabox_leftTexWidth, teabox_leftTexHeight;
	unsigned char* teabox_leftImage = SOIL_load_image("teabox_leftEdit.jpg", &teabox_leftTexWidth, &teabox_leftTexHeight, 0, SOIL_LOAD_RGB);

	int teabox_rightTexWidth, teabox_rightTexHeight;
	unsigned char* teabox_rightImage = SOIL_load_image("teabox_rightEdit.jpg", &teabox_rightTexWidth, &teabox_rightTexHeight, 0, SOIL_LOAD_RGB);

	int teabox_topTexWidth, teabox_topTexHeight;
	unsigned char* teabox_topImage = SOIL_load_image("teabox_topEdit.jpg", &teabox_topTexWidth, &teabox_topTexHeight, 0, SOIL_LOAD_RGB);

	int teabottle_labelTexWidth, teabottle_labelTexHeight;
	unsigned char* teabottle_labelImage = SOIL_load_image("teabottle_labelEdit.jpg", &teabottle_labelTexWidth, &teabottle_labelTexHeight, 0, SOIL_LOAD_RGB);

	int teabottle_descTexWidth, teabottle_descTexHeight;
	unsigned char* teabottle_descImage = SOIL_load_image("teabottle_descEdit.jpg", &teabottle_descTexWidth, &teabottle_descTexHeight, 0, SOIL_LOAD_RGB);

	int teabottle_nutrTexWidth, teabottle_nutrTexHeight;
	unsigned char* teabottle_nutrImage = SOIL_load_image("teabottle_nutrEdit.jpg", &teabottle_nutrTexWidth, &teabottle_nutrTexHeight, 0, SOIL_LOAD_RGBA);

	int teaTexWidth, teaTexHeight;
	unsigned char* teaImage = SOIL_load_image("tea.jpg", &teaTexWidth, &teaTexHeight, 0, SOIL_LOAD_RGB);

	int lidTexWidth, lidTexHeight;
	unsigned char* lidImage = SOIL_load_image("lid.jpg", &lidTexWidth, &lidTexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit1TexWidth, nutsEdit1TexHeight;
	unsigned char* nutsEdit1Image = SOIL_load_image("nutsEdit1.jpg", &nutsEdit1TexWidth, &nutsEdit1TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit2TexWidth, nutsEdit2TexHeight;
	unsigned char* nutsEdit2Image = SOIL_load_image("nutsEdit2.jpg", &nutsEdit2TexWidth, &nutsEdit2TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit3TexWidth, nutsEdit3TexHeight;
	unsigned char* nutsEdit3Image = SOIL_load_image("nutsEdit3.jpg", &nutsEdit3TexWidth, &nutsEdit3TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit4TexWidth, nutsEdit4TexHeight;
	unsigned char* nutsEdit4Image = SOIL_load_image("nutsEdit4.jpg", &nutsEdit4TexWidth, &nutsEdit4TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit5TexWidth, nutsEdit5TexHeight;
	unsigned char* nutsEdit5Image = SOIL_load_image("nutsEdit5.jpg", &nutsEdit5TexWidth, &nutsEdit5TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit6TexWidth, nutsEdit6TexHeight;
	unsigned char* nutsEdit6Image = SOIL_load_image("nutsEdit6.jpg", &nutsEdit6TexWidth, &nutsEdit6TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit7TexWidth, nutsEdit7TexHeight;
	unsigned char* nutsEdit7Image = SOIL_load_image("nutsEdit7.jpg", &nutsEdit7TexWidth, &nutsEdit7TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit8TexWidth, nutsEdit8TexHeight;
	unsigned char* nutsEdit8Image = SOIL_load_image("nutsEdit8.jpg", &nutsEdit8TexWidth, &nutsEdit8TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit9TexWidth, nutsEdit9TexHeight;
	unsigned char* nutsEdit9Image = SOIL_load_image("nutsEdit9.jpg", &nutsEdit9TexWidth, &nutsEdit9TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit10TexWidth, nutsEdit10TexHeight;
	unsigned char* nutsEdit10Image = SOIL_load_image("nutsEdit10.jpg", &nutsEdit10TexWidth, &nutsEdit10TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit11TexWidth, nutsEdit11TexHeight;
	unsigned char* nutsEdit11Image = SOIL_load_image("nutsEdit11.jpg", &nutsEdit11TexWidth, &nutsEdit11TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit12TexWidth, nutsEdit12TexHeight;
	unsigned char* nutsEdit12Image = SOIL_load_image("nutsEdit12.jpg", &nutsEdit12TexWidth, &nutsEdit12TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit13TexWidth, nutsEdit13TexHeight;
	unsigned char* nutsEdit13Image = SOIL_load_image("nutsEdit13.jpg", &nutsEdit13TexWidth, &nutsEdit13TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit14TexWidth, nutsEdit14TexHeight;
	unsigned char* nutsEdit14Image = SOIL_load_image("nutsEdit14.jpg", &nutsEdit14TexWidth, &nutsEdit14TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit15TexWidth, nutsEdit15TexHeight;
	unsigned char* nutsEdit15Image = SOIL_load_image("nutsEdit15.jpg", &nutsEdit15TexWidth, &nutsEdit15TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit16TexWidth, nutsEdit16TexHeight;
	unsigned char* nutsEdit16Image = SOIL_load_image("nutsEdit16.jpg", &nutsEdit16TexWidth, &nutsEdit16TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit17TexWidth, nutsEdit17TexHeight;
	unsigned char* nutsEdit17Image = SOIL_load_image("nutsEdit17.jpg", &nutsEdit17TexWidth, &nutsEdit17TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit18TexWidth, nutsEdit18TexHeight;
	unsigned char* nutsEdit18Image = SOIL_load_image("nutsEdit18.jpg", &nutsEdit18TexWidth, &nutsEdit18TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit19TexWidth, nutsEdit19TexHeight;
	unsigned char* nutsEdit19Image = SOIL_load_image("nutsEdit19.jpg", &nutsEdit19TexWidth, &nutsEdit19TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit20TexWidth, nutsEdit20TexHeight;
	unsigned char* nutsEdit20Image = SOIL_load_image("nutsEdit20.jpg", &nutsEdit20TexWidth, &nutsEdit20TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit21TexWidth, nutsEdit21TexHeight;
	unsigned char* nutsEdit21Image = SOIL_load_image("nutsEdit21.jpg", &nutsEdit21TexWidth, &nutsEdit21TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit22TexWidth, nutsEdit22TexHeight;
	unsigned char* nutsEdit22Image = SOIL_load_image("nutsEdit22.jpg", &nutsEdit22TexWidth, &nutsEdit22TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit23TexWidth, nutsEdit23TexHeight;
	unsigned char* nutsEdit23Image = SOIL_load_image("nutsEdit23.jpg", &nutsEdit23TexWidth, &nutsEdit23TexHeight, 0, SOIL_LOAD_RGB);

	int nutsEdit24TexWidth, nutsEdit24TexHeight;
	unsigned char* nutsEdit24Image = SOIL_load_image("nutsEdit24.jpg", &nutsEdit24TexWidth, &nutsEdit24TexHeight, 0, SOIL_LOAD_RGB);

	int woodTexWidth, woodTexHeight;
	unsigned char* woodImage = SOIL_load_image("wood.jpg", &woodTexWidth, &woodTexHeight, 0, SOIL_LOAD_RGB);

	// Generate Textures
	GLuint keyboardTexture;
	glm::vec3 keyboardColor;
	glGenTextures(1, &keyboardTexture);
	glBindTexture(GL_TEXTURE_2D, keyboardTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, keyboardTexWidth, keyboardTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, keyboardImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	keyboardColor = glm::vec3(0.1f, 0.1f, 0.09f);
	SOIL_free_image_data(keyboardImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint laptop_lidTexture;
	glm::vec3 laptop_lidColor;
	glGenTextures(1, &laptop_lidTexture);
	glBindTexture(GL_TEXTURE_2D, laptop_lidTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, laptop_lidTexWidth, laptop_lidTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, laptop_lidImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	laptop_lidColor = glm::vec3(0.12f, 0.12f, 0.11f);
	SOIL_free_image_data(laptop_lidImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint laptop_rimTexture;
	glm::vec3 laptop_rimColor;
	glGenTextures(1, &laptop_rimTexture);
	glBindTexture(GL_TEXTURE_2D, laptop_rimTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, laptop_rimTexWidth, laptop_rimTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, laptop_rimImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	laptop_rimColor = glm::vec3(0.08f, 0.08f, 0.07f);
	SOIL_free_image_data(laptop_rimImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint monitorTexture;
	glm::vec3 monitorColor;
	glGenTextures(1, &monitorTexture);
	glBindTexture(GL_TEXTURE_2D, monitorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, monitorTexWidth, monitorTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, monitorImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	monitorColor = glm::vec3(0.08f, 0.09f, 0.08f);
	SOIL_free_image_data(monitorImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabox_backTexture;
	glm::vec3 teabox_backColor;
	glGenTextures(1, &teabox_backTexture);
	glBindTexture(GL_TEXTURE_2D, teabox_backTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabox_backTexWidth, teabox_backTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabox_backImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabox_backColor = glm::vec3(0.16f, 0.15f, 0.14f);
	SOIL_free_image_data(teabox_backImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabox_bottomTexture;
	glm::vec3 teabox_bottomColor;
	glGenTextures(1, &teabox_bottomTexture);
	glBindTexture(GL_TEXTURE_2D, teabox_bottomTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabox_bottomTexWidth, teabox_bottomTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabox_bottomImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabox_bottomColor = glm::vec3(0.22f, 0.21f, 0.19f);
	SOIL_free_image_data(teabox_bottomImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabox_frontTexture;
	glm::vec3 teabox_frontColor;
	glGenTextures(1, &teabox_frontTexture);
	glBindTexture(GL_TEXTURE_2D, teabox_frontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabox_frontTexWidth, teabox_frontTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabox_frontImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabox_frontColor = glm::vec3(0.18f, 0.19f, 0.21f);
	SOIL_free_image_data(teabox_frontImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabox_leftTexture;
	glm::vec3 teabox_leftColor;
	glGenTextures(1, &teabox_leftTexture);
	glBindTexture(GL_TEXTURE_2D, teabox_leftTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabox_leftTexWidth, teabox_leftTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabox_leftImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabox_leftColor = glm::vec3(0.25f, 0.21f, 0.18f);
	SOIL_free_image_data(teabox_leftImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabox_rightTexture;
	glm::vec3 teabox_rightColor;
	glGenTextures(1, &teabox_rightTexture);
	glBindTexture(GL_TEXTURE_2D, teabox_rightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabox_rightTexWidth, teabox_rightTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabox_rightImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabox_rightColor = glm::vec3(0.21f, 0.21f, 0.19f);
	SOIL_free_image_data(teabox_rightImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabox_topTexture;
	glm::vec3 teabox_topColor;
	glGenTextures(1, &teabox_topTexture);
	glBindTexture(GL_TEXTURE_2D, teabox_topTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabox_topTexWidth, teabox_topTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabox_topImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabox_topColor = glm::vec3(0.14f, 0.12f, 0.1f);
	SOIL_free_image_data(teabox_topImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabottle_labelTexture;
	glm::vec3 teabottle_labelColor;
	glGenTextures(1, &teabottle_labelTexture);
	glBindTexture(GL_TEXTURE_2D, teabottle_labelTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabottle_labelTexWidth, teabottle_labelTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabottle_labelImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabottle_labelColor = glm::vec3(0.17f, 0.19f, 0.22f);
	SOIL_free_image_data(teabottle_labelImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabottle_descTexture;
	glm::vec3 teabottle_descColor;
	glGenTextures(1, &teabottle_descTexture);
	glBindTexture(GL_TEXTURE_2D, teabottle_descTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teabottle_descTexWidth, teabottle_descTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teabottle_descImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabottle_descColor = glm::vec3(0.09f, 0.09f, 0.07f);
	SOIL_free_image_data(teabottle_descImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teabottle_nutrTexture;
	glm::vec3 teabottle_nutrColor;
	glGenTextures(1, &teabottle_nutrTexture);
	glBindTexture(GL_TEXTURE_2D, teabottle_nutrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, teabottle_nutrTexWidth, teabottle_nutrTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, teabottle_nutrImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teabottle_nutrColor = glm::vec3(0.14f, 0.12f, 0.10f);
	SOIL_free_image_data(teabottle_nutrImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint teaTexture;
	glm::vec3 teaColor;
	glGenTextures(1, &teaTexture);
	glBindTexture(GL_TEXTURE_2D, teaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, teaTexWidth, teaTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, teaImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	teaColor = glm::vec3(0.28f, 0.12f, 0.0f);
	SOIL_free_image_data(teaImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint lidTexture;
	glm::vec3 lidColor;
	glGenTextures(1, &lidTexture);
	glBindTexture(GL_TEXTURE_2D, lidTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lidTexWidth, lidTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, lidImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	lidColor = glm::vec3(0.18f, 0.18f, 0.18f);
	SOIL_free_image_data(lidImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint nutsEdit1Texture;
	glm::vec3 nutsEditColor;
	glGenTextures(1, &nutsEdit1Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit1Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit1TexWidth, nutsEdit1TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit1Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	nutsEditColor = glm::vec3(0.31f, 0.2f, 0.08f);
	SOIL_free_image_data(nutsEdit1Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[0] = nutsEdit1Texture;

	GLuint nutsEdit2Texture;
	glGenTextures(1, &nutsEdit2Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit2Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit2TexWidth, nutsEdit2TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit2Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit2Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[1] = nutsEdit2Texture;

	GLuint nutsEdit3Texture;
	glGenTextures(1, &nutsEdit3Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit3Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit3TexWidth, nutsEdit3TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit3Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit3Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[2] = nutsEdit3Texture;

	GLuint nutsEdit4Texture;
	glGenTextures(1, &nutsEdit4Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit4Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit4TexWidth, nutsEdit4TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit4Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit4Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[3] = nutsEdit4Texture;

	GLuint nutsEdit5Texture;
	glGenTextures(1, &nutsEdit5Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit5Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit5TexWidth, nutsEdit5TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit5Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit5Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[4] = nutsEdit5Texture;

	GLuint nutsEdit6Texture;
	glGenTextures(1, &nutsEdit6Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit6Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit6TexWidth, nutsEdit6TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit6Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit6Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[5] = nutsEdit6Texture;

	GLuint nutsEdit7Texture;
	glGenTextures(1, &nutsEdit7Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit7Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit7TexWidth, nutsEdit7TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit7Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit7Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[6] = nutsEdit7Texture;

	GLuint nutsEdit8Texture;
	glGenTextures(1, &nutsEdit8Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit8Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit8TexWidth, nutsEdit8TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit8Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit8Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[7] = nutsEdit8Texture;

	GLuint nutsEdit9Texture;
	glGenTextures(1, &nutsEdit9Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit9Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit9TexWidth, nutsEdit9TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit9Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit9Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[8] = nutsEdit9Texture;

	GLuint nutsEdit10Texture;
	glGenTextures(1, &nutsEdit10Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit10Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit10TexWidth, nutsEdit10TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit10Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit10Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[9] = nutsEdit10Texture;

	GLuint nutsEdit11Texture;
	glGenTextures(1, &nutsEdit11Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit11Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit11TexWidth, nutsEdit11TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit11Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit11Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[10] = nutsEdit11Texture;

	GLuint nutsEdit12Texture;
	glGenTextures(1, &nutsEdit12Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit12Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit12TexWidth, nutsEdit12TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit12Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit12Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[11] = nutsEdit12Texture;

	GLuint nutsEdit13Texture;
	glGenTextures(1, &nutsEdit13Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit13Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit13TexWidth, nutsEdit13TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit13Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit13Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[12] = nutsEdit13Texture;

	GLuint nutsEdit14Texture;
	glGenTextures(1, &nutsEdit14Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit14Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit14TexWidth, nutsEdit14TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit14Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit14Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[13] = nutsEdit14Texture;

	GLuint nutsEdit15Texture;
	glGenTextures(1, &nutsEdit15Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit15Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit15TexWidth, nutsEdit15TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit15Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit15Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[14] = nutsEdit15Texture;

	GLuint nutsEdit16Texture;
	glGenTextures(1, &nutsEdit16Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit16Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit16TexWidth, nutsEdit16TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit16Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit16Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[15] = nutsEdit16Texture;

	GLuint nutsEdit17Texture;
	glGenTextures(1, &nutsEdit17Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit17Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit17TexWidth, nutsEdit17TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit17Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit17Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[16] = nutsEdit17Texture;

	GLuint nutsEdit18Texture;
	glGenTextures(1, &nutsEdit18Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit18Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit18TexWidth, nutsEdit18TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit18Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit18Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[17] = nutsEdit18Texture;

	GLuint nutsEdit19Texture;
	glGenTextures(1, &nutsEdit19Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit19Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit19TexWidth, nutsEdit19TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit19Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit19Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[18] = nutsEdit19Texture;

	GLuint nutsEdit20Texture;
	glGenTextures(1, &nutsEdit20Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit20Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit20TexWidth, nutsEdit20TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit20Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit20Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[19] = nutsEdit20Texture;

	GLuint nutsEdit21Texture;
	glGenTextures(1, &nutsEdit21Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit21Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit21TexWidth, nutsEdit21TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit21Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit21Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[20] = nutsEdit21Texture;

	GLuint nutsEdit22Texture;
	glGenTextures(1, &nutsEdit22Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit22Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit22TexWidth, nutsEdit22TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit22Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit22Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[21] = nutsEdit22Texture;

	GLuint nutsEdit23Texture;
	glGenTextures(1, &nutsEdit23Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit23Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit23TexWidth, nutsEdit23TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit23Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit23Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[22] = nutsEdit23Texture;

	GLuint nutsEdit24Texture;
	glGenTextures(1, &nutsEdit24Texture);
	glBindTexture(GL_TEXTURE_2D, nutsEdit24Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nutsEdit24TexWidth, nutsEdit24TexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nutsEdit24Image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(nutsEdit24Image);
	glBindTexture(GL_TEXTURE_2D, 0);
	nutTexList[23] = nutsEdit24Texture;

	GLuint woodTexture;
	glm::vec3 woodColor;
	glGenTextures(1, &woodTexture);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, woodTexWidth, woodTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, woodImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	woodColor = glm::vec3(0.27f, 0.21f, 0.13f);
	SOIL_free_image_data(woodImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Vertex shader source code
	string vertexShaderSource =
		"#version 430 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aColor;\n"
		"layout(location = 2) in vec2 texCoord;\n"
		"layout(location = 3) in vec3 normal;\n"
		"out vec3 oColor;\n"
		"out vec2 oTexCoord;\n"
		"out vec3 oNormal;\n"
		"out vec3 fragPos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main() {\n"
		"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"oColor = aColor;\n"
		"oTexCoord = texCoord;\n"
		"oNormal = mat3(transpose(inverse(model))) * normal;\n"
		"fragPos = vec3(model * vec4(aPos, 1.0));\n"
		"}";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 430 core\n"
		"in vec3 oColor;\n"
		"in vec2 oTexCoord;\n"
		"in vec3 oNormal;\n"
		"in vec3 fragPos;\n"
		"out vec4 fragColor;\n"
		"uniform sampler2D myTexture;\n"
		"uniform vec3 objectColor;\n"
		"uniform vec3 lightColor1;\n"
		"uniform vec3 lightPos1;\n"
		"uniform vec3 lightColor2;\n"
		"uniform vec3 lightPos2;\n"
		"uniform vec3 lightColor3;\n"
		"uniform vec3 lightPos3;\n"
		"uniform vec3 viewPos;\n"
		"void main() {\n"
		"// Ambient\n"
		"float ambientStrength = 1.0f;\n"
		"vec3 ambient1 = ambientStrength * lightColor1;\n"
		"vec3 ambient2 = 0.3 * ambientStrength * lightColor2;\n"
		"vec3 ambient3 = 0.5 * ambientStrength * lightColor3;\n"
		"vec3 ambient = ambient1 + ambient2 + ambient3;\n"
		"// Diffuse\n"
		"vec3 norm = normalize(oNormal);\n"
		"vec3 lightDir1 = normalize(lightPos1 - fragPos);\n"
		"vec3 lightDir2 = normalize(lightPos2 - fragPos);\n"
		"vec3 lightDir3 = normalize(lightPos3 - fragPos);\n"
		"float diff1 = max(dot(norm, lightDir1), 0.0);\n"
		"float diff2 = max(dot(norm, lightDir2), 0.0);\n"
		"float diff3 = max(dot(norm, lightDir3), 0.0);\n"
		"vec3 diffuse1 = diff1 * 4.0 * lightColor1;\n"
		"vec3 diffuse2 = diff2 * lightColor2;\n"
		"vec3 diffuse3 = diff3 * 2.0 * lightColor3;\n"
		"vec3 diffuse = diffuse1 + diffuse2 + diffuse3;\n"
		"// Specular\n"
		"float specularStrength = 1.5f;\n"
		"vec3 viewDir = normalize(viewPos - fragPos);\n"
		"vec3 reflectDir1 = reflect(-lightDir1, norm);\n"
		"vec3 reflectDir2 = reflect(-lightDir2, norm);\n"
		"vec3 reflectDir3 = reflect(-lightDir3, norm);\n"
		"float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 16);\n"
		"float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 8);\n"
		"float spec3 = pow(max(dot(viewDir, reflectDir3), 0.0), 16);\n"
		"vec3 specular1 = specularStrength * spec1 * lightColor1;\n"
		"vec3 specular2 = specularStrength  * 0.5 * spec2 * lightColor2;\n"
		"vec3 specular3 = specularStrength * spec3 * lightColor3;\n"
		"vec3 specular = specular1 + specular2 + specular3;\n"
		"vec3 result = (ambient + diffuse + specular) * objectColor;\n"
		"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0);\n"
		"}";

	// Lamp Vertex shader source code
	string lampVertexShaderSource =
		"#version 430 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main() {\n"
		"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}";

	// Lamp Fragment shader source code
	string lampFragmentShaderSource =
		"#version 430 core\n"
		"out vec4 fragColor;\n"
		"uniform vec3 lampColor;\n"
		"void main() {\n"
		"fragColor = vec4(lampColor, 1.0);\n"
		"}";

	// Creating shader program
	GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
	GLuint lampShaderProgram = createShaderProgram(lampVertexShaderSource, lampFragmentShaderSource);

	init(window);

	while (!glfwWindowShouldClose(window)) {
		// Set deltaTime
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Process input each frame
		processInput(window);

		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use shader program executable
		glUseProgram(shaderProgram);

		/*
			Draw Plane
		*/

		// Declare identity matrices
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);

		// Initialize transforms
		viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		projectionMatrix = getProjection();

		// Get object color, light color, and light position location
		GLuint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLuint lightColor1Loc = glGetUniformLocation(shaderProgram, "lightColor1");
		GLuint lightPos1Loc = glGetUniformLocation(shaderProgram, "lightPos1");
		GLuint lightColor2Loc = glGetUniformLocation(shaderProgram, "lightColor2");
		GLuint lightPos2Loc = glGetUniformLocation(shaderProgram, "lightPos2");
		GLuint lightColor3Loc = glGetUniformLocation(shaderProgram, "lightColor3");
		GLuint lightPos3Loc = glGetUniformLocation(shaderProgram, "lightPos3");
		GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		// Assign light color
		glUniform3f(lightColor1Loc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightColor2Loc, 1.0f, 0.0f, 0.0f);
		glUniform3f(lightColor3Loc, 0.0f, 0.0f, 1.0f);

		// Set Light Position
		glUniform3f(lightPos1Loc, lightPosition1.x, lightPosition1.y, lightPosition1.z);
		glUniform3f(lightPos2Loc, lightPosition2.x, lightPosition2.y, lightPosition2.z);
		glUniform3f(lightPos3Loc, lightPosition3.x, lightPosition3.y, lightPosition3.z);

		// Specify View Position
		glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

		// Select shader and uniform variable
		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		// Pass transform to shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindVertexArray(squareVAO); // Bind VAO

		modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f, 1.0f, 20.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glUniform3f(objectColorLoc, woodColor.x, woodColor.y, woodColor.z); // Set object color
		glBindTexture(GL_TEXTURE_2D, woodTexture); // Bind Texture

		// Draw plane
		draw(6);

		glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture

		/*
			Draw Laptop Base
		*/

		for (GLuint i = 0; i < 6; i++) {
			modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, basePositions[i]);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(baseRotationsX[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(baseRotationsZ[i]), glm::vec3(0.0f, 0.0f, 1.0f));
			modelMatrix = glm::scale(modelMatrix, baseScaling[i]);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			switch (i) {
			case 2:
				glUniform3f(objectColorLoc, keyboardColor.x, keyboardColor.y, keyboardColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, keyboardTexture); // Bind Texture
				break;
			default:
				glUniform3f(objectColorLoc, laptop_rimColor.x, laptop_rimColor.y, laptop_rimColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, laptop_rimTexture); // Bind Texture
				break;
			}

			draw(6);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		/*
			Draw Laptop Monitor
		*/

		for (GLuint i = 0; i < 6; i++) {
			modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, monitorPositions[i]);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(monitorRotationsX[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(monitorRotationsZ[i]), glm::vec3(0.0f, 0.0f, 1.0f));
			modelMatrix = glm::scale(modelMatrix, monitorScaling[i]);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			switch (i) {
			case 1:
				glUniform3f(objectColorLoc, laptop_lidColor.x, laptop_lidColor.y, laptop_lidColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, laptop_lidTexture); // Bind Texture
				break;
			case 3:
				glUniform3f(objectColorLoc, monitorColor.x, monitorColor.y, monitorColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, monitorTexture); // Bind Texture
				break;
			default:
				glUniform3f(objectColorLoc, laptop_rimColor.x, laptop_rimColor.y, laptop_rimColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, laptop_rimTexture); // Bind Texture
				break;
			}

			draw(6);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		/*
			Draw Teabox
		*/

		for (GLuint i = 0; i < 6; i++) {
			modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::rotate(modelMatrix, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::translate(modelMatrix, teaboxPositions[i]);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(teaboxRotationsX[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			if (i == 1) {
				modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			modelMatrix = glm::rotate(modelMatrix, glm::radians(teaboxRotationsZ[i]), glm::vec3(0.0f, 0.0f, 1.0f));
			modelMatrix = glm::scale(modelMatrix, teaboxScaling[i]);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			switch (i) {
			case 0:
				glUniform3f(objectColorLoc, teabox_bottomColor.x, teabox_bottomColor.y, teabox_bottomColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabox_bottomTexture); // Bind Texture
				break;
			case 1:
				glUniform3f(objectColorLoc, teabox_backColor.x, teabox_backColor.y, teabox_backColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabox_backTexture); // Bind Texture
				break;
			case 2:
				glUniform3f(objectColorLoc, teabox_topColor.x, teabox_topColor.y, teabox_topColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabox_topTexture); // Bind Texture
				break;
			case 3:
				glUniform3f(objectColorLoc, teabox_frontColor.x, teabox_frontColor.y, teabox_frontColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabox_frontTexture); // Bind Texture
				break;
			case 4:
				glUniform3f(objectColorLoc, teabox_leftColor.x, teabox_leftColor.y, teabox_leftColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabox_leftTexture); // Bind Texture
				break;
			case 5:
				glUniform3f(objectColorLoc, teabox_rightColor.x, teabox_rightColor.y, teabox_rightColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabox_rightTexture); // Bind Texture
				break;
			}

			draw(6);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		/*
			Draw Tea Bottle
		*/

		for (GLuint i = 0; i < 6; i++) {
			modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, teaBottlePositions[i]);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(teaBottleRotationsX[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(teaBottleRotationsZ[i]), glm::vec3(0.0f, 0.0f, 1.0f));
			if (i == 1)
				modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, teaBottleScaling[i]);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			switch (i) {
			case 0:
				glUniform3f(objectColorLoc, teaColor.x, teaColor.y, teaColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teaTexture); // Bind Texture
				break;
			case 1:
				glUniform3f(objectColorLoc, teabottle_labelColor.x, teabottle_labelColor.y, teabottle_labelColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabottle_labelTexture); // Bind Texture
				break;
			case 2:
				glUniform3f(objectColorLoc, teaColor.x, teaColor.y, teaColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teaTexture); // Bind Texture
				break;
			case 3:
				glUniform3f(objectColorLoc, teabottle_labelColor.x, teabottle_labelColor.y, teabottle_labelColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabottle_labelTexture); // Bind Texture
				break;
			case 4:
				glUniform3f(objectColorLoc, teabottle_nutrColor.x, teabottle_nutrColor.y, teabottle_nutrColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabottle_nutrTexture); // Bind Texture
				break;
			case 5:
				glUniform3f(objectColorLoc, teabottle_descColor.x, teabottle_descColor.y, teabottle_descColor.z); // Set object color
				glBindTexture(GL_TEXTURE_2D, teabottle_descTexture); // Bind Texture
				break;
			}

			draw(6);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		glBindVertexArray(0);

		glBindVertexArray(pyramidVAO);

		for (GLuint i = 0; i < 4; i++) {
			modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, glm::vec3(-6.0f, 1.5f, -2.5f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(pyramidRotationsY[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.85f, 1.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			glUniform3f(objectColorLoc, teaColor.x, teaColor.y, teaColor.z); // Set object color
			glBindTexture(GL_TEXTURE_2D, teaTexture); // Bind Texture

			draw(3);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		glBindVertexArray(0);

		glBindVertexArray(cylinderVAO); // Bind VAO

		for (GLuint i = 0; i < 24; i++) {
			glUniform3f(objectColorLoc, teaColor.x, teaColor.y, teaColor.z); // Set object color
			glBindTexture(GL_TEXTURE_2D, teaTexture); // Bind Texture

			modelMatrix = glm::mat4(1.0f);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-6.0f, 1.5f, -2.5f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(i * 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 1.25f, 0.3f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw plane
			draw(12);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		for (GLuint i = 0; i < 24; i++) {
			glUniform3f(objectColorLoc, lidColor.x, lidColor.y, lidColor.z); // Set object color
			glBindTexture(GL_TEXTURE_2D, lidTexture); // Bind Texture

			modelMatrix = glm::mat4(1.0f);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-6.0f, 2.75f, -2.5f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(i * 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.4f, 0.2f, 0.4f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw plane
			draw(12);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		/*
			Draw Nut Tin
		*/

		for (GLuint i = 0; i < 24; i++) {
			glUniform3f(objectColorLoc, nutsEditColor.x, nutsEditColor.y, nutsEditColor.z); // Set object color
			glBindTexture(GL_TEXTURE_2D, nutTexList[i]); // Bind Texture

			modelMatrix = glm::mat4(1.0f);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-7.0f, 0.0f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(i * 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw plane
			draw(12);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		for (GLuint i = 0; i < 24; i++) {
			glUniform3f(objectColorLoc, lidColor.x, lidColor.y, lidColor.z); // Set object color
			glBindTexture(GL_TEXTURE_2D, lidTexture); // Bind Texture

			modelMatrix = glm::mat4(1.0f);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-7.0f, 1.0f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, glm::radians(i * 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1.05f, 0.2f, 1.05f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw plane
			draw(12);

			glBindTexture(GL_TEXTURE_2D, 0); // Unbind Texture
		}

		glBindVertexArray(0);

		// Unbind shader program
		glUseProgram(0);

		/*
			Draw Light Sources
		*/

		glUseProgram(lampShaderProgram);

		GLuint lampModelLoc = glGetUniformLocation(lampShaderProgram, "model");
		GLuint lampViewLoc = glGetUniformLocation(lampShaderProgram, "view");
		GLuint lampProjectionLoc = glGetUniformLocation(lampShaderProgram, "projection");

		glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(lampProjectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		GLuint lampColorLoc = glGetUniformLocation(lampShaderProgram, "lampColor");

		glBindVertexArray(lampVAO); // Bind VAO

		glUniform3f(lampColorLoc, 1.0f, 1.0f, 1.0f); // Set Lamp Color

		for (GLuint i = 0; i < 6; i++) {
			glm::mat4 modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, lampPositions[i] / glm::vec3(8.0f, 8.0f, 8.0f) + lightPosition1);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(lampRotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, glm::radians(lampRotations[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));

			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(6);
		}

		glUniform3f(lampColorLoc, 1.0f, 0.0f, 0.0f); // Set Lamp Color

		for (GLuint i = 0; i < 6; i++) {
			glm::mat4 modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, lampPositions[i] / glm::vec3(8.0f, 8.0f, 8.0f) + lightPosition2);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(lampRotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, glm::radians(lampRotations[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));

			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(6);
		}

		glUniform3f(lampColorLoc, 0.0f, 0.0f, 1.0f); // Set Lamp Color

		for (GLuint i = 0; i < 6; i++) {
			glm::mat4 modelMatrix = glm::mat4(1.0f);

			modelMatrix = glm::translate(modelMatrix, lampPositions[i] / glm::vec3(8.0f, 8.0f, 8.0f) + lightPosition3);
			modelMatrix = glm::rotate(modelMatrix, glm::radians(lampRotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, glm::radians(lampRotations[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));

			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(6);
		}

		glBindVertexArray(0); // Unbind VAO

		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &squareVAO);
	glDeleteBuffers(1, &squareVBO);
	glDeleteBuffers(1, &squareEBO);
	glDeleteVertexArrays(1, &pyramidVAO);
	glDeleteBuffers(1, &pyramidVBO);
	glDeleteBuffers(1, &pyramidEBO);
	glDeleteVertexArrays(1, &cylinderVAO);
	glDeleteBuffers(1, &cylinderVBO);
	glDeleteBuffers(1, &cylinderEBO);
	glDeleteVertexArrays(1, &lampVAO);
	glDeleteBuffers(1, &lampVBO);
	glDeleteBuffers(1, &lampEBO);

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

// Define processInput function
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	GLfloat cameraSpeed = speedModifier * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos += cameraRight * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos -= cameraRight * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;

	// Reset camera if F is pressed
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		resetCamera();
}

// Define Input Callback Functions
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// Set Speed Modifier
	speedModifier += yoffset;

	// Clamp Speed Modifier
	if (speedModifier < 1.0f)
		speedModifier = 1.0f;
	if (speedModifier > 20.0f)
		speedModifier = 20.0f;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouseMove) {
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	// Calculate cursor offset
	xOffset = xpos - lastX;
	yOffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	// Sensitivity
	GLfloat sensitivity = 0.1f;

	rawYaw += xOffset * sensitivity;
	rawPitch += yOffset * sensitivity;

	degYaw = glm::radians(rawYaw);
	degPitch = glm::clamp(glm::radians(rawPitch), glm::radians(-89.0f), glm::radians(89.0f));

	// Define and set camera direction
	glm::vec3 direction;
	direction.x = cosf(degYaw) * cosf(degPitch);
	direction.y = sinf(degPitch);
	direction.z = sinf(degYaw) * cosf(degPitch);
	cameraRight = glm::normalize(glm::cross(worldUp, direction));
	cameraUp = glm::normalize(glm::cross(direction, cameraRight));
	cameraFront = glm::normalize(direction);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//Flip the view
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		is3D = !is3D;
}

// Define Reset Camera Function
void resetCamera() {
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
}

// Define 2d / 3d view swap prototype
glm::mat4 getProjection() {
	if (is3D)
		return glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	else
		return glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
}