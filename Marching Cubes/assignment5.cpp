#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "TriTable.hpp"
#include "cmake-build-debug/shaders.hpp"
#include "CubeAxes.hpp"

#define FRONT_TOP_LEFT     128
#define FRONT_TOP_RIGHT     64
#define BACK_TOP_RIGHT      32
#define BACK_TOP_LEFT       16
#define FRONT_BOTTOM_LEFT    8
#define FRONT_BOTTOM_RIGHT   4
#define BACK_BOTTOM_RIGHT    2
#define BACK_BOTTOM_LEFT     1

float f(float x, float y, float z){
    //return y - (sin(x) * cos(z));		        //function 1 (waves)
    return x * x - y * y - z * z - z;	        //function 2 (cone thing)
/*
    float R = 1.5; // Major radius              //function 3 donut
    float r = 0.5; // Minor radius
    float dx = x;
    float dy = y;
    float d = sqrt(dx * dx + dy * dy) - R;
    return r * r - (d * d + z * z);
*/
}

GLFWwindow* window;

//function to control camera movement
void cameraControlsGlobe(GLFWwindow* window, glm::mat4& V, float& r, float& theta, float& phi) {
    static const float move_speed = 0.05f; // adjust this for desired movement speed
    static const float rotate_speed = 0.5f; // adjust this for desired rotation speed

    glm::vec3 camera_position(r * sin(glm::radians(theta)) * cos(glm::radians(phi)),
                              r * sin(glm::radians(phi)),
                              r * cos(glm::radians(theta)) * cos(glm::radians(phi)));
    glm::vec3 camera_direction = glm::normalize(-camera_position);
    glm::vec3 camera_up(0.0f, 1.0f, 0.0f);

    // handle keyboard input
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        r = std::max(r - move_speed, 0.1f); // stop before reaching r = 0
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        r += move_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        theta += rotate_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        theta -= rotate_speed;
    }

    // handle mouse input
    static double last_xpos = -1, last_ypos = -1;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (last_xpos >= 0 && last_ypos >= 0) {
            theta -= rotate_speed * float(xpos - last_xpos);
            phi += rotate_speed * float(ypos - last_ypos);
            // clamp phi to avoid flipping over
            phi = glm::clamp(phi, -89.0f, 89.0f);
        }
    }
    last_xpos = xpos;
    last_ypos = ypos;

    // update camera position and direction
    camera_position.x = r * sin(glm::radians(theta)) * cos(glm::radians(phi));
    camera_position.y = r * sin(glm::radians(phi));
    camera_position.z = r * cos(glm::radians(theta)) * cos(glm::radians(phi));
    camera_direction = glm::normalize(-camera_position);

    // update view matrix
    V = glm::lookAt(camera_position, camera_position + camera_direction, camera_up);
}

// Handles marching cubes mesh generation
class MarchingCubes {
    std::function<float(float, float, float)> generationFunction;
    float isoValue = 0;
    float minCoord = 0;
    float maxCoord = 1;
    float stepSize = 0.1;
    float currentIteration = 0;
    std::vector<float> vertices;

    void add_triangles(int* verts, float x, float y, float z) {
        for (int i = 0; verts[i] >= 0; i += 3) {
            for (int j = 0; j < 3; ++j) {
                vertices.emplace_back(x + stepSize * vertTable[verts[i + j]][0]);
                vertices.emplace_back(y + stepSize * vertTable[verts[i + j]][1]);
                vertices.emplace_back(z + stepSize * vertTable[verts[i + j]][2]);
            }
        }
    }
    void generate_iterative_new() {
        float ftl, ftr, fbr, fbl, btl, btr, bbr, bbl;
        int which = 0;
        int *verts;

        for (float a = minCoord; a < maxCoord; a += stepSize) {
            for (float b = minCoord; b < maxCoord; b += stepSize) {


                //test the square
                bbl = generationFunction(a, b, currentIteration);
                btl = generationFunction(a, b + stepSize, currentIteration);
                btr = generationFunction(a + stepSize, b + stepSize, currentIteration);
                bbr = generationFunction(a + stepSize, b, currentIteration);
                fbl = generationFunction(a, b, currentIteration + stepSize);
                ftl = generationFunction(a, b + stepSize, currentIteration + stepSize);
                ftr = generationFunction(a + stepSize, b + stepSize, currentIteration + stepSize);
                fbr = generationFunction(a + stepSize, b, currentIteration + stepSize);

                which = 0;
                if (ftl < isoValue) {
                    which |= FRONT_TOP_LEFT;
                }
                if (ftr < isoValue) {
                    which |= FRONT_TOP_RIGHT;
                }
                if (btr < isoValue) {
                    which |= BACK_TOP_RIGHT;
                }
                if (btl < isoValue) {
                    which |= BACK_TOP_LEFT;
                }
                if (fbl < isoValue) {
                    which |= FRONT_BOTTOM_LEFT;
                }
                if (fbr < isoValue) {
                    which |= FRONT_BOTTOM_RIGHT;
                }
                if (bbr < isoValue) {
                    which |= BACK_BOTTOM_RIGHT;
                }
                if (bbl < isoValue) {
                    which |= BACK_BOTTOM_LEFT;
                }
                verts = marching_cubes_lut[which];
                add_triangles(verts, a, b, currentIteration);
            }
        }
        currentIteration += stepSize;
        if (currentIteration > maxCoord){
            finished = true;
        }
    }

public:
    bool finished = false;

    MarchingCubes(std::function<float(float, float, float)> f, float isoval, float min, float max, float step)
            : generationFunction(f), isoValue(isoval), minCoord(min), maxCoord(max), stepSize(step), currentIteration(min) {}

    void generate() {
        generate_iterative_new();
    }

    std::vector<float> getVertices() {
        return vertices;
    }
};

std::vector<float> compute_normals(std::vector<float> vertices){
    std::vector<float> normals;
    glm::vec3 p1, p2, p3, p12, p13, n;
    glm::vec3 cross;
    for(int i = 0;i < vertices.size();i += 9){
        p1 = {vertices[i], vertices[i+1], vertices[i+2]};
        p2 = {vertices[i+3], vertices[i+4], vertices[i+5]};
        p3 = {vertices[i+6], vertices[i+7], vertices[i+8]};

        float p12x = p2.x - p1.x;
        float p12y = p2.y - p1.y;
        float p12z = p2.z - p1.z;
        float p13x = p3.x - p1.x;
        float p13y = p3.y - p1.y;
        float p13z = p3.z - p1.z;

        float crossX = (p12y*p13z)-(p12z*p13y);
        float crossY = (p12z*p13x)-(p12x*p13z);
        float crossZ = (p12x*p13y)-(p12y*p13x);

        cross = glm::vec3(crossX, crossY, crossZ);
        n = glm::normalize(cross);

        for(int j = 0;j < 3;j++){
            normals.emplace_back(n.z);
            normals.emplace_back(n.y);
            normals.emplace_back(n.x);
        }
    }
    return normals;
}

void writePLY(std::string outFile, std::vector<float> vertexData, std::vector<float> normalData){
    // Create output file
    std::ofstream plyFile(outFile);
    if (plyFile.fail()){
        return;
    }

    int vertexCount = vertexData.size() / 3;
    int faceCount = vertexCount / 3;

    // Write PLY header
    plyFile << "ply" << std::endl;
    plyFile << "format ascii 1.0" << std::endl;
    plyFile << "element vertex " << vertexCount << std::endl;
    plyFile << "property float x" << std::endl;
    plyFile << "property float y" << std::endl;
    plyFile << "property float z" << std::endl;
    plyFile << "property float nx" << std::endl;
    plyFile << "property float ny" << std::endl;
    plyFile << "property float nz" << std::endl;
    plyFile << "element face " << faceCount << std::endl;
    plyFile << "property list uchar uint vertex_indices" << std::endl;
    plyFile << "end_header" << std::endl;

    // Write vertex positions and normals
    for (size_t idx = 2; idx < vertexData.size(); idx += 3){
        plyFile << vertexData[idx - 2] << " " << vertexData[idx - 1] << " " << vertexData[idx] << " " << normalData[idx - 2] << " " << normalData[idx - 1] << " " << normalData[idx] << std::endl;
    }

    // Write face data
    for (size_t idx = 2; idx < vertexCount; idx += 3){
        plyFile << "3 " << idx - 2 << " " << idx - 1 << " " << idx << std::endl;
    }

    plyFile.close();
}


int main(int argc, char* argv[]) {
    std::vector<float> normals;
    float step = 0.05;
    float min = -5.0f;
    float max = 5.0f;
    float isoval = -1.5;
    std::string filename = "rendering.ply";
    bool generateFile = true;

    // Initialize window
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(1000, 1000, "Assignment 5", NULL, NULL);
    if (window == NULL) {
        printf("Failed to open window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    glClearColor(0.2, 0.2, 0.3, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set up initial MVP matrix
    glm::mat4 mvp;
    glm::vec3 eyePos(5, 5, 5);
    glm::vec3 zero(0, 0, 0);
    glm::vec3 up(0, 1, 0);
    glm::mat4 view = glm::lookAt(eyePos, zero, up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.001f, 1000.0f);
    glm::mat4 model = glm::mat4(1.0f);
    mvp = projection * view * model;

    MarchingCubes cubes(f, isoval, min, max, step);
    Cube drawCube(min,max);

    // Set up the VAO and buffers
    GLuint vao, vertexVBO, normalVBO, programID;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Vertex VBO
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, &cubes.getVertices()[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
    );
    // Normal VBO
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, &normals, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
    );
    glBindVertexArray(0);

    // Shaders
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShaderID, 1, &vertexShader, NULL);
    glCompileShader(vertexShaderID);
    glShaderSource(fragmentShaderID, 1, &fragmentShader, NULL);
    glCompileShader(fragmentShaderID);
    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // Variables for camera and input
    float r = 30.0f;
    float theta = 45.0f;
    float phi = 45.0f;
    //color and light models
    GLfloat MODEL_COLOR[4] = {0.0f, 1.0f, 1.0f, 1.0f};
    GLfloat LIGHT_DIRECTION[3] = {5.0f, 5.0f, 5.0f};
    double prevTime = glfwGetTime();
    bool wroteFile = false;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get delta time
        double currentTime = glfwGetTime();
        float deltaTime = currentTime - prevTime;
        prevTime = currentTime;

        // Call camera controls function here
        cameraControlsGlobe(window, view, r, theta, phi);
        // mvp
        mvp = projection * view * model;

        // Generate more of the mesh if it's not done yet (also update vertex and normal buffers)
        if (!cubes.finished) {
            cubes.generate();
            std::vector<float> vertices = cubes.getVertices();
            normals = compute_normals(vertices);

            // Update buffers
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), &normals[0], GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), &vertices[0], GL_DYNAMIC_DRAW);
            glBindVertexArray(0);

        } else if (!wroteFile && generateFile) {
            // Mesh is done - generate file if enabled
            std::vector<float> vertices = cubes.getVertices();
            normals = compute_normals(vertices);
            writePLY("rendering.ply", vertices, normals);
            wroteFile = true;
        }

        // Draw the axes and box
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(glm::value_ptr(projection));
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(glm::value_ptr(view));
        drawCube.draw();

        // Draw the mesh
        glUseProgram(programID);

        // Set up uniforms
        GLuint matrixID = glGetUniformLocation(programID, "MVP");
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);

        GLuint viewID = glGetUniformLocation(programID, "V");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);

        GLuint colorID = glGetUniformLocation(programID, "modelColor");
        glUniform4fv(colorID, 1, MODEL_COLOR);

        GLuint lightDirID = glGetUniformLocation(programID, "lightDir");
        glUniform3fv(lightDirID, 1, LIGHT_DIRECTION);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, normals.size());
        glBindVertexArray(0);
        glUseProgram(0);


        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}
