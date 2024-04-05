/*
CS 3388 Assignment 4

This assignment creates a 3D scene using OpenGL and GLFW. 
The scene is created using a PLY file and a BMP file. 
The PLY file contains the vertex data and the BMP file contains the texture data. 
The scene is created using textured meshes. 
The camera can be moved using the arrow keys and the WASD/QE keys. 
The scene is drawn using the Model View Projection matrix.

Date: March 8, 2024
Written by: Nathan Orgera
*/


// Include headers and libraries
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "LoadBitmap.hpp"

using namespace glm;
GLFWwindow* window;

// Vertex struct
struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 textureCoords;

    VertexData(glm::vec3 pos, glm::vec3 norm, glm::vec3 col, glm::vec2 tex) :
        position(pos),
        normal(norm),
        color(col),
        textureCoords(tex)
    {}
};

struct TriData {
    GLuint vertex_indices[3];
};

void processInput(GLFWwindow* window, glm::vec3& Position, glm::vec3& Front, glm::vec3& Up) {

    // Camera movement speed
    float Speed = 0.005;
    float Rotation = 0.2f;

    // Processes arrow keys to move the camera
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        Position += Speed * Front; // forward

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        Position -= Speed * Front; // backward

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        Front = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), Up) * glm::vec4(Front, 1.0f))); // rotate left

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        Front = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(-Rotation), Up) * glm::vec4(Front, 1.0f))); // rotate right


    // Processes WASD/QE keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += Speed * Up; // upwards

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= Speed * Up; // downwards

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::normalize(glm::cross(Front, Up)) * Speed; // left

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::normalize(glm::cross(Front, Up)) * Speed; // right

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        Front = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(Rotation * 0.5f), glm::normalize(glm::cross(Front, Up))) * glm::vec4(Front, 1.0f))); // rotate up

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        Front = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(-Rotation * 0.5f), glm::normalize(glm::cross(Front, Up))) * glm::vec4(Front, 1.0f))); // rotate down
        
    // Normalizes the cameraFront vector
    Up = glm::normalize(glm::cross(glm::cross(Front, Up), Front));
}
\

// Function to read PLY file
void readPLYFile(std::string fname, std::vector<VertexData>& vertices, std::vector<TriData>& faces) {
    // Open file
    std::ifstream file(fname);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fname << std::endl;
        return;
    }

    // Read header
    std::string line;
    std::istringstream stream; 
    std::string token; 
    int Verts = 0; 
    int Faces = 0;
    bool normalVector = false;
    bool colour = false;
    bool coordinates = false;
    int Length = 0;
    std::map <int, const char*> Order = {}; // Map to store the order of properties

    // Parses header
    while (getline(file, line)) {
        stream.clear();
        stream.str(line);
        stream >> token;

        // Checks for normal vector, colour and texture coordinates
        if (token == "element") {
            stream >> token;

            if (token == "vertex") {
                stream >> Verts;
            }
            if (token == "face") {
                stream >> Faces;
            }
        }
        else if (token == "property") { 
            stream >> token;
            if (token == "float") {
                stream >> token;
                if (token == "x") {
                    Order.insert(std::make_pair(Length, "x")); 
                }
                else if (token == "y") {
                    Order.insert(std::make_pair(Length, "y")); 
                }
                else if (token == "z") {
                    Order.insert(std::make_pair(Length, "z"));
                }
                if (token == "nx") {
                    Order.insert(std::make_pair(Length, "nx"));
                }
                else if (token == "ny") {
                    Order.insert(std::make_pair(Length, "ny"));
                }
                else if (token == "nz") {
                    Order.insert(std::make_pair(Length, "nz"));
                }
                if (token == "red") {
                    Order.insert(std::make_pair(Length, "red"));
                }
                else if (token == "green") {
                    Order.insert(std::make_pair(Length, "green"));
                }
                else if (token == "blue") {
                    Order.insert(std::make_pair(Length, "blue"));
                }
                else if (token == "u") {
                    Order.insert(std::make_pair(Length, "u"));
                }
                else if (token == "v") {
                    Order.insert(std::make_pair(Length, "v"));
                }
            }
            else if (token == "uchar") {
                stream >> token;
                if (token == "red") {
                    Order.insert(std::make_pair(Length, "red"));
                }
                else if (token == "green") {
                    Order.insert(std::make_pair(Length, "green"));
                }
                else if (token == "blue") {
                    Order.insert(std::make_pair(Length, "blue"));
                }
            } Length++;
        }

        // Checks for end of header
        else if (token == "end_header") {
            break;
        }
    }

    // Parses vertex data
    for (int i = 0; i < Verts; i++) {
        glm::vec3 position(0.0f);
        glm::vec3 normal(0.0f);
        glm::vec3 color(0.0f);
        glm::vec2 texturePos(0.0f);
        getline(file, line);
        stream.clear();
        stream.str(line);
        int counter = 0;

        do {
            auto property = Order.find(counter); // Finds property
            if (property->second == "x") { 
                stream >> position.x; // Reads x coordinate
            }
            else if (property->second == "y") { 
                stream >> position.y; // Reads y coordinate
            }
            else if (property->second == "z") {
                stream >> position.z; // Reads z coordinate
            }
            else if (property->second == "nx") {
                stream >> normal.x; // Reads x normal
            }
            else if (property->second == "ny") {
                stream >> normal.y; // Reads y normal
            }
            else if (property->second == "nz") {
                stream >> normal.z; // Reads z normal
            }
            else if (property->second == "red") {
                stream >> color.r; // Reads red color
            }
            else if (property->second == "green") {
                stream >> color.g; // Reads green color
            }
            else if (property->second == "blue") {
                stream >> color.b;  // Reads blue color
            }
            else if (property->second == "u") {
                stream >> texturePos.x; // Reads u texture coordinate
            }
            else if (property->second == "v") {
                stream >> texturePos.y; // Reads v texture coordinate
            }
            counter++; // Increments counter
        } while (counter < Order.size()); // Loops through properties until all are read
        
        // Pushes back vertex data
        vertices.push_back(VertexData(position, normal, color, texturePos));
    }

    // Parses face data
    for (int i = 0; i < Faces; i++) {
        GLuint v0, v1, v2;
        getline(file, line);
        stream.clear();
        stream.str(line);
        stream >> token >> v0 >> v1 >> v2;
        faces.push_back(TriData{ {v0, v1, v2} });
    }

    file.close();
}

// Class to create textured mesh
class TexturedMesh {
private:
    std::vector <VertexData> vertices;
    std::vector <TriData> faces;
    GLuint vao; 
    GLuint vboVertexPosition; 
    GLuint vboTextureCoordinates; 
    GLuint eboFacesIndices; 
    GLuint texture; 
    GLuint shader;

    // Function to load ARGB BMP file
public:
    TexturedMesh(const char* plyFilePath, const char* bmpFilePath) {

        // Reads PLY file
        readPLYFile(plyFilePath, vertices, faces);
        std::vector<GLfloat> vertexTextureCoordinates;
        std::vector<GLfloat> vertexPositions;

        // Pushes back vertex positions and texture coordinates
        vertexPositions.reserve(vertices.size() * 3);
        for (const auto& vertex : vertices) {
            vertexPositions.push_back(vertex.position.x);
            vertexPositions.push_back(vertex.position.y);
            vertexPositions.push_back(vertex.position.z);
        }
        vertexTextureCoordinates.reserve(vertices.size() * 2);
        for (const auto& vertex : vertices) {
            vertexTextureCoordinates.push_back(vertex.textureCoords.x);
            vertexTextureCoordinates.push_back(vertex.textureCoords.y);
        }
        
        unsigned char* data = nullptr;
        GLuint width, height;
        loadARGB_BMP(bmpFilePath, &data, &width, &height);

        // Generates texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        delete[] data;

        // Creates shader
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        std::string VertexShaderCode = "\
            #version 120\n\
            attribute vec3 vertexPosition;\n\
            attribute vec2 uv;\n\
            varying vec2 uv_out;\n\
            uniform mat4 MVP;\n\
            void main(){ \n\
                gl_Position =  MVP * vec4(vertexPosition,1);\n\
                uv_out = uv;\n\
            }\n";

        std::string FragmentShaderCode = "\
            #version 120\n\
            varying vec2 uv_out; \n\
            uniform sampler2D tex;\n\
            void main() {\n\
                gl_FragColor = texture2D(tex, uv_out);\n\
            }\n";

        // Compiles and links shader
        char const* VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
        glCompileShader(VertexShaderID);

        char const* FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
        glCompileShader(FragmentShaderID);
        
        shader = glCreateProgram();
        glAttachShader(shader, VertexShaderID);
        glAttachShader(shader, FragmentShaderID);
        glLinkProgram(shader);

        glDetachShader(shader, VertexShaderID);
        glDetachShader(shader, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vboVertexPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboVertexPosition);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexPositions.size(), vertexPositions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      
        glGenBuffers(1, &vboTextureCoordinates);
        glBindBuffer(GL_ARRAY_BUFFER, vboTextureCoordinates);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexTextureCoordinates.size(), vertexTextureCoordinates.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glGenBuffers(1, &eboFacesIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFacesIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * faces.size(), faces.data(), GL_STATIC_DRAW);   
        glBindVertexArray(0);
    }

    // Function to draw textured mesh
    void draw(glm::mat4 MVP) {

        // Enables blending using alpha channel
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Uses shader
        glUseProgram(shader);
        GLuint Matrix = glGetUniformLocation(shader, "MVP");
        glUniformMatrix4fv(Matrix, 1, GL_FALSE, &MVP[0][0]);

        // Binds texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);

        // Draws mesh
        glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);

        // Disables blending
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        }

};

int main() {
    
    // Initializes GLFW
	if( !glfwInit() )
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return 1;
	}

	float screenWidth = 1400;
	float screenHeight = 800;

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwSwapInterval(1);

    // Creates window
	window = glfwCreateWindow( screenWidth, screenHeight, "Assignment 4", NULL, NULL);
	if( window == NULL ){
		fprintf(stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

    // Initializes GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return 1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); // Ensures we can capture the escape key being pressed below

    // Sets background color and enables depth test
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    // Creates textured mesh objects
    TexturedMesh bottlesMesh("Bottles.ply", "bottles.bmp");
    TexturedMesh floorMesh("Floor.ply", "floor.bmp");
    TexturedMesh windowBackgroundMesh("WindowBG.ply", "windowbg.bmp");
    TexturedMesh woodObjectsMesh("WoodObjects.ply", "woodobjects.bmp");
    TexturedMesh patioMesh("Patio.ply", "patio.bmp"); 
    TexturedMesh tableMesh("Table.ply", "table.bmp");
    TexturedMesh wallsMesh("Walls.ply", "walls.bmp");
    TexturedMesh doorBackgroundMesh("DoorBG.ply", "doorbg.bmp");
    TexturedMesh metalObjectsMesh("MetalObjects.ply", "metalobjects.bmp");
    TexturedMesh curtainsMesh("Curtains.ply", "curtains.bmp");;
    
    // Camera position, front and up vectors
    glm::vec3 Position = glm::vec3(0.5f, 0.4f, 0.5f);
    glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 MVP;

	do{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        processInput(window, Position, Front, Up);
        
        glm::mat4 V = glm::lookAt(Position, Position + Front, Up); // View matrix
        glm::mat4 P = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f); // Projection matrix

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(glm::value_ptr(P));

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(glm::value_ptr(V));
        
        glm::mat4 M = glm::mat4(1.0f); // Identity matrix
        MVP = P * V * M; // Model View Projection matrix

        // Draws textured mesh objects
        bottlesMesh.draw(MVP);
        floorMesh.draw(MVP);
        patioMesh.draw(MVP);
        tableMesh.draw(MVP);
        wallsMesh.draw(MVP);
        woodObjectsMesh.draw(MVP);
        windowBackgroundMesh.draw(MVP);
        metalObjectsMesh.draw(MVP);
        doorBackgroundMesh.draw(MVP);
        curtainsMesh.draw(MVP);

        // Swaps buffers
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glfwSwapBuffers(window);
        glfwPollEvents();
	}

    // ESC key to close window
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glfwTerminate();
	return 0;
}

