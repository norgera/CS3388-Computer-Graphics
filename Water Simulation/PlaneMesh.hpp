#ifndef PLANEMESH_HPP
#define PLANEMESH_HPP

// Header files
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>


// Camera controls taken from misc files on OWL
void cameraControlsGlobe(glm::mat4& V, float start) {

    glm::vec3 eye = {start * 6, start * 2, start * 6};
    glm::vec3 targ = {0.0f, 0.0f, 0.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    static float radiusFromOrigin = glm::length(eye);

    static glm::vec3 position = eye;
    //exactly the angles to look at origin from (x,y,z) = (+v, +v/2, +v);
    static GLfloat theta = 0.0f;//0.25f*3.14159f;
    static GLfloat phi = 0.392f*3.14159f;

    static double mouseDownX;
    static double mouseDownY;
    static bool firstPress = true;
    static double lastTime = glfwGetTime();

    double dx = 0.0, dy = 0.0;
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
    {
        if (firstPress) {
            glfwGetCursorPos(window, &mouseDownX, &mouseDownY);
        }
        firstPress = false;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        dx = xpos - mouseDownX;
        dy = ypos - mouseDownY;

        mouseDownX = xpos;
        mouseDownY = ypos;
    }
    if (state == GLFW_RELEASE) {
        firstPress = true;
    }

        theta += 0.002f * dx;
        phi   += -0.002f * dy;
        if (theta > 2*(2.0*asin(1))) {
            theta -= 2*(2.0*asin(1));
        }
        if (phi >= 2.0*asin(1)) {
            phi = 0.9999999 * 2.0*asin(1);
        }
        if (phi <= 0) {
            phi = 0.0000001;
        }
    

    glm::vec3 direction(
        sin(phi) * cos(theta),
        cos(phi),
        sin(phi) * sin(theta)
    );
    glm::normalize(direction);

    float speed = 0.25f * radiusFromOrigin; //move faster further away
    double currentTime = glfwGetTime();
    float deltaTime = (currentTime - lastTime);
    lastTime = currentTime;

    // Move forward
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS) {
        radiusFromOrigin -= deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS) {
        radiusFromOrigin += deltaTime * speed;
    }

    position = direction * radiusFromOrigin;
    V = glm::lookAt(position, targ, up);

}


// PlaneMesh class
class PlaneMesh {
	
	// Private variables
	std::vector<float> verts;
	std::vector<int> indices;
	float min, max;
	GLuint VAO;
	GLuint vertexbuffer, elementbuffer;
	GLuint TexID, DispID;
	GLuint ProgramID;
	GLuint MatrixID, ViewMatrixID, ModelMatrixID, LightID, timeID;


	void planeMeshQuads(float min, float max, float stepsize) {
        // Generate vertices
        for (float x = min; x <= max; x += stepsize) {
            for (float z = min; z <= max; z += stepsize) {
                verts.emplace_back(x);
                verts.emplace_back(0.0f); // Flat plane
                verts.emplace_back(z);
            }
        }

        int nCols = static_cast<int>((max - min) / stepsize + 1);

        // Generate indices
        for (int i = 0; i < nCols - 1; ++i) {
            for (int j = 0; j < nCols - 1; ++j) {
                int index = i * nCols + j;
                indices.push_back(index); // Top-left
                indices.push_back(index + nCols); // Bottom-left
                indices.push_back(index + nCols + 1); // Bottom-right
                indices.push_back(index + 1); // Top-right
            }
        }
    }

public:
	PlaneMesh(float min, float max, float stepsize) {
		this->min = min;
		this->max = max;

		planeMeshQuads(min, max, stepsize);
		
		// Load shaders
		ProgramID = LoadShaders("WaterShader.vertexshader", 
			"WaterShader.tcs", 
			"WaterShader.tes", 
			"WaterShader.geoshader", 
			"WaterShader.fragmentshader");


		// Load textures
		unsigned char* texture;
		unsigned int tWidth, tHeight;
		loadBMP("water.bmp", &texture, &tWidth, &tHeight);
		glGenTextures(1, &TexID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);


		// Load displacement map
		unsigned char* dispData;
		unsigned int dispWidth, dispHeight;
		loadBMP("displacement-map1.bmp", &dispData, &dispWidth, &dispHeight);
		glGenTextures(1, &DispID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, DispID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dispWidth, dispHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, dispData);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// Set up uniforms
		glUseProgram(ProgramID);
		glUniform1i(glGetUniformLocation(ProgramID, "tex"), 0);
		glUniform1i(glGetUniformLocation(ProgramID, "heighTex"), 1);

        // Set uniform for texOffset
		GLuint uniform = glGetUniformLocation(ProgramID, "texOffset");
		glUniform1f(uniform, 0.0);

		// Set uniform for texScale
		uniform = glGetUniformLocation(ProgramID, "texScale");
		glUniform1f(uniform, 6.0);

		// Set uniform for displacementHeight
		uniform = glGetUniformLocation(ProgramID, "displacementHeight");
		glUniform1f(uniform, 0.3);

		// Set uniform for displacementCloseness
		uniform = glGetUniformLocation(ProgramID, "displacementCloseness");
		glUniform1f(uniform, 0.06);

		// Set uniform for outerTess
		uniform = glGetUniformLocation(ProgramID, "outerTess");
		glUniform1f(uniform, 12.0);

		// Set uniform for innerTess
		uniform = glGetUniformLocation(ProgramID, "innerTess");
		glUniform1f(uniform, 12.0);

		// Set uniform for alpha
		uniform = glGetUniformLocation(ProgramID, "alpha");
		glUniform1f(uniform, 32.0);

		glUseProgram(0);

		// Set up handles for uniforms
		MatrixID = glGetUniformLocation(ProgramID, "MVP"); 
		ViewMatrixID = glGetUniformLocation(ProgramID, "V");
		ModelMatrixID = glGetUniformLocation(ProgramID, "M");
		LightID = glGetUniformLocation(ProgramID, "LightPosition_worldspace");
		timeID = glGetUniformLocation(ProgramID, "time");

		glGenBuffers(1, &vertexbuffer);
		glGenBuffers(1, &elementbuffer);

		glGenVertexArrays(1, &VAO); // Create Vertex Array Object
		glBindVertexArray(VAO); // Bind Vertex Array Object

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); // Bind Vertex Buffer
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), &verts[0], GL_STATIC_DRAW); // Send data to GPU
 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer); // Bind Element Buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW); // Send data to GPU

		// Set up vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0);

		glBindVertexArray(0);
	}

	
	// Draw function
	void draw(glm::vec3 lightPos, glm::mat4 V, glm::mat4 P) {
		
		glUseProgram(ProgramID);
		glm::mat4 M = glm::mat4(1.0f); // Model matrix
		glm::mat4 MVP = P * V * M; // ModelViewProjection matrix

		// Set up uniforms
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &M[0][0]);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform1f(timeID, glfwGetTime());

		// Bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, DispID);
		glBindVertexArray(VAO);

		// Draw elements
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, (void*)0);

		// Unbind
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisableVertexAttribArray(0);
		glUseProgram(0);

	}

	// Load shaders function
	GLuint LoadShaders(const char * vertex_file_path, const char * tessellation_control_file_path, const char * tessellation_evaluation_file_path, const char * geo_file_path, const char * fragment_file_path){

    // Reading Vertex shader code
	std::string VertexShaderCode, TessellationControlShaderCode, TessellationEvaluationShaderCode, GeometryShaderCode, FragmentShaderCode; // Strings to store shader code
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in); // Open the file

	// Check if file is open
	if(VertexShaderStream.is_open()) {
    	std::stringstream sstr; // Create a string stream
    	sstr << VertexShaderStream.rdbuf(); // Read the file
    	VertexShaderCode = sstr.str(); // Store the file in the string
    	VertexShaderStream.close(); // Close the file
	} else {
    	std::cerr << "Could not open " << vertex_file_path << ".\n"; // Error message
	}

	// Reading tessellation control shader code
    std::ifstream TessellationControlShaderStream(tessellation_control_file_path, std::ios::in);
	if(TessellationControlShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << TessellationControlShaderStream.rdbuf();
		TessellationControlShaderCode = sstr.str();
		TessellationControlShaderStream.close();
	} else {
		std::cerr << "Could not open " << tessellation_control_file_path << ".\n";
	}

	// Reading tessellation evaluation shader code
	std::ifstream TessellationEvaluationShaderStream(tessellation_evaluation_file_path, std::ios::in);
	if(TessellationEvaluationShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << TessellationEvaluationShaderStream.rdbuf();
		TessellationEvaluationShaderCode = sstr.str();
		TessellationEvaluationShaderStream.close();
	} else {
		std::cerr << "Could not open " << tessellation_evaluation_file_path << ".\n";
	}

	// Reading geometry shader code
	std::ifstream GeometryShaderStream(geo_file_path, std::ios::in);
	if(GeometryShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << GeometryShaderStream.rdbuf();
		GeometryShaderCode = sstr.str();
		GeometryShaderStream.close();
	} else {
		std::cerr << "Could not open " << geo_file_path << ".\n";
	}

	// Reading fragment shader code
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	} else {
		std::cerr << "Could not open " << fragment_file_path << ".\n";
	}



	// Passing Vertex Shader
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER); // Create a shader ID
	const GLchar* vertexSource = VertexShaderCode.c_str(); // Get the shader code
	glShaderSource(VertexShaderID, 1, &vertexSource, NULL); // Pass the shader code to the shader
	glCompileShader(VertexShaderID); // Compile the shader

	// Passing Tessellation Control Shader
	GLuint TessellationControlShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
	const GLchar* tessControlSource = TessellationControlShaderCode.c_str();
	glShaderSource(TessellationControlShaderID, 1, &tessControlSource, NULL);
	glCompileShader(TessellationControlShaderID);

	// Passing Tessellation Evaluation Shader
	GLuint TessellationEvaluationShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
	const GLchar* tessEvalSource = TessellationEvaluationShaderCode.c_str();
	glShaderSource(TessellationEvaluationShaderID, 1, &tessEvalSource, NULL);
	glCompileShader(TessellationEvaluationShaderID);

	// Passing Geometry Shader
    GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	const GLchar* geometrySource = GeometryShaderCode.c_str();
	glShaderSource(GeometryShaderID, 1, &geometrySource, NULL);
	glCompileShader(GeometryShaderID);

	// Passing Fragment Shader
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* fragmentSource = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &fragmentSource, NULL);
	glCompileShader(FragmentShaderID);

    
	// Attach shaders to the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, TessellationControlShaderID);
    glAttachShader(ProgramID, TessellationEvaluationShaderID);
    glAttachShader(ProgramID, GeometryShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Detach shaders
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, TessellationControlShaderID);
    glDetachShader(ProgramID, TessellationEvaluationShaderID);
    glDetachShader(ProgramID, GeometryShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

	// Delete shaders
    glDeleteShader(VertexShaderID);
    glDeleteShader(TessellationControlShaderID);
    glDeleteShader(TessellationEvaluationShaderID);
    glDeleteShader(GeometryShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;

}

	// LoadBMP function taken from misc files on OWL
	void loadBMP(const char* imagepath, unsigned char** data, unsigned int* width, unsigned int* height) {

    // printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    // Actual RGB data

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file){
        printf("%s could not be opened. Are you in the right directory?\n", imagepath);
        getchar();
        return;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 ){
        printf("Not a correct BMP file\n");
        fclose(file);
        return;
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file\n");
        fclose(file);
        return;
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  ) {
    	printf("Not a correct BMP file\n");
	    fclose(file);
	    return;
	}
    if ( *(int*)&(header[0x1C])!=24 ) {
    	printf("Not a correct BMP file\n");
	    fclose(file);
	    return;
	}

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    *width      = *(int*)&(header[0x12]);
    *height     = *(int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=(*width)* (*height)*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    *data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(*data,1,imageSize,file);

    fprintf(stderr, "Done reading!\n");

    // Everything is in memory now, the file can be closed.
    fclose (file);

}
};

#endif