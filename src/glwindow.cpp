#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "glwindow.h"
#include "geometry.h"
                                                                                                                                                                            
using namespace std;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}

void OpenGLWindow::shaderSetUniformValue(string var, glm::vec3 values)
{
    const GLchar* var_gl = var.c_str();
    GLuint pos = glGetUniformLocation(shader, var_gl);
    glUniform3fv(pos, 1, &values[0]);
}

void OpenGLWindow::shaderSetUniformValue(string var, glm::mat4& values)
{
    const GLchar* var_gl = var.c_str();
    GLuint pos = glGetUniformLocation(shader, var_gl);
    glUniformMatrix4fv(pos, 1, GL_FALSE, &values[0][0]);
}

void OpenGLWindow::shaderSetUniformValue(string var, float values)
{
    const GLchar* var_gl = var.c_str();
    GLuint pos = glGetUniformLocation(shader, var_gl);
    glUniform1f(pos, values);
}

// Function to load and create a texture from a JPEG file
unsigned int OpenGLWindow::loadTexture(const char* filename)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip texture vertically since OpenGL expects the top-left corner as the origin
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters (optional)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

void OpenGLWindow::applyTexture(const char* textureFilePath)
{
    std::vector<float> textureCoordData = geometryData.getTextureData();

    // Create and bind a VBO for the texture coordinates
    unsigned int textureVBO;
    glGenBuffers(1, &textureVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, textureCoordData.size() * sizeof(float), textureCoordData.data(), GL_STATIC_DRAW);

    // Bind and activate a texture from a JPEG file
    unsigned int textureID = loadTexture(textureFilePath);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configure texture coordinate attribute in the vertex shader
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 2",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);   

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    shaderSetUniformValue("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shaderSetUniformValue("lightPos", glm::vec3(-2.0f, 5.0f, 0.0f));
    shaderSetUniformValue("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));

    colourIdx = 0;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);  
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
    
    totalTransformations = glm::mat4(1.0f);
    projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    viewMatrix = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0,1,0));
    model = glm::mat4(1.0f);
    MVP = projectionMatrix * viewMatrix * model;
    
    shaderSetUniformValue("projection", projectionMatrix);
    shaderSetUniformValue("view", viewMatrix);
    shaderSetUniformValue("model", model);
    shaderSetUniformValue("totalTransformations", totalTransformations);
    
    light1Pos = glm::vec3(-3.0f, 2.0f, 0.0f);
    light1Col = glm::vec3(1.0f, 1.0f, 1.0f);

    light2Pos = glm::vec3(4.0f, -2.0f, 1.0f);
    light2Col = glm::vec3(1.0f, 1.0f, 0.0f);
    
    shaderSetUniformValue("light1.position", light1Pos);
    shaderSetUniformValue("light1.color", light1Col);

    shaderSetUniformValue("light2.position", light2Pos);
    shaderSetUniformValue("light2.color", light2Col);

   shaderSetUniformValue("viewPos", viewMatrix);

    glm::vec3 pointLightPositions[] = {
	    glm::vec3( 0.7f,  0.2f,  2.0f),
	    glm::vec3( 2.3f, -3.3f, -4.0f),
    };

    // Load the model that we want to use and buffer the vertex attributes
    geometryData = GeometryData();
    geometryData.loadFromOBJFile("suzanne.obj");

    //applyTexture("spikes.jpeg");
    applyTexture("wall-norm.jpg");
    
    vertexLoc = glGetAttribLocation(shader, "aPos");
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * geometryData.vertexCount() * 3, geometryData.vertexData(), GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    normalLoc = glGetAttribLocation(shader, "aNormal");
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * geometryData.vertexCount() * 3, geometryData.normalData(), GL_STATIC_DRAW);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    transformationMode = 's';
    axis = 'y';
    isLightSource1 = true;
    light1Transformations = glm::mat4(1.0f);
    light2Transformations = glm::mat4(1.0f);

    glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, geometryData.vertexCount());
   
    lightAngle += lightRotationSpeed * 0.01f;
    float lightPosX = lightCenter.x + lightRadius * glm::cos(lightAngle);
    float lightPosZ = lightCenter.z + lightRadius * glm::sin(lightAngle);
    light1Pos = glm::vec3(lightPosX, light1Pos.y, lightPosZ);
    light2Pos = glm::vec3(lightPosX, light2Pos.y, lightPosZ);

    // Update uniform values for light positions in the shader
    shaderSetUniformValue("light1.position", light1Pos);
    shaderSetUniformValue("light2.position", light2Pos);
    
    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        
        if(e.key.keysym.sym == SDLK_c)
        {
            colourIdx++;

            if (colourIdx == 0)
                glUniform3f(colorLoc, 0.5f, 0.6f, 0.0f); // yellow
            else if (colourIdx == 1)
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // red
            else if (colourIdx == 2)
                glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f); // green
            else if (colourIdx == 3)
                glUniform3f(colorLoc, 0.0f, 0.0f, 1.0f); // blue 
            else if (colourIdx == 4)
                glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f); // turquoise
            else if (colourIdx == 5)            
            {
                glUniform3f(colorLoc, 1.0f, 0.0f, 1.0f); // purple
            }
            else if (colourIdx == 6)
            {
                glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // white
                colourIdx = -1;
            }
            
            return false;
        }

        if(e.key.keysym.sym == SDLK_x)
        {
            axis = 'x';
            cout << "The transformation axis has been set to the x-axis." << endl;
        }

        if(e.key.keysym.sym == SDLK_y)
        {
            axis = 'y';
            cout << "The transformation axis has been set to the y-axis." << endl;
        }

        if(e.key.keysym.sym == SDLK_z)
        {
            axis = 'z';
            cout << "The transformation axis has been set to the z-axis." << endl;
        }

        if(e.key.keysym.sym == SDLK_KP_PLUS || e.key.keysym.sym == SDLK_PLUS)
        {
            float changeMatrix[3] = {1.0f, 1.0f, 1.0f};

            if (axis == 'x')
                changeMatrix[0] = 1.02f;
            else if (axis == 'y')
                changeMatrix[1] = 1.02f;
            else if (axis == 'z')
                changeMatrix[2] = 1.02f;
            
            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::scale(trans, glm::vec3(changeMatrix[0], changeMatrix[1], changeMatrix[2]));
            
            totalTransformations = trans * totalTransformations;
            MVP = projectionMatrix * viewMatrix * totalTransformations * model;
            
            shaderSetUniformValue("totalTransformations", totalTransformations);
        }

        if(e.key.keysym.sym == SDLK_KP_MINUS || e.key.keysym.sym == SDLK_MINUS)
        {
            float changeMatrix[3] = {1.0f, 1.0f, 1.0f};

            if (axis == 'x')
                changeMatrix[0] = 0.98f;
            else if (axis == 'y')
                changeMatrix[1] = 0.98f;
            else if (axis == 'z')
                changeMatrix[2] = 0.98f;
            
            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::scale(trans, glm::vec3(changeMatrix[0], changeMatrix[1], changeMatrix[2]));
            totalTransformations = trans * totalTransformations;
            shaderSetUniformValue("totalTransformations", totalTransformations);
        }
        else if (e.key.keysym.sym == SDLK_BACKSPACE)
        {
            MVP = projectionMatrix * viewMatrix * model;
            totalTransformations = glm::mat4(1.0f);
            viewMatrix = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0) );
            
            shaderSetUniformValue("totalTransformations", totalTransformations);
            shaderSetUniformValue("view", viewMatrix);
        }
        else if (e.key.keysym.sym == SDLK_RIGHT)
        {
            float changeMatrix[3] = {0.0f, 0.0f, 0.0f};

            if (axis == 'x')
                changeMatrix[0] = 0.5f;
            else if (axis == 'y')
                changeMatrix[1] = 0.5f;
            else if (axis == 'z')
                changeMatrix[2] = 0.5f;

            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::rotate(trans, glm::radians(10.0f), glm::vec3(changeMatrix[0], changeMatrix[1], changeMatrix[2]));
            
            viewMatrix = viewMatrix * trans;
            
            shaderSetUniformValue("view", viewMatrix);
        }
        else if (e.key.keysym.sym == SDLK_LEFT)
        {        
            float changeMatrix[3] = {0.0f, 0.0f, 0.0f};

            if (axis == 'x')
                changeMatrix[0] = -0.5f;
            else if (axis == 'y')
                changeMatrix[1] = -0.5f;
            else if (axis == 'z')
                changeMatrix[2] = -0.5f;

            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::rotate(trans, glm::radians(10.0f), glm::vec3(changeMatrix[0], changeMatrix[1], changeMatrix[2]));
            
            viewMatrix = viewMatrix * trans;

            shaderSetUniformValue("view", viewMatrix);
            shaderSetUniformValue("viewPos", viewMatrix);
        }
    }
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
