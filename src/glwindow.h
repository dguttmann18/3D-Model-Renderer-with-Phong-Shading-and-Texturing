#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    bool handleEvent(SDL_Event e);
    void cleanup();
    void shaderSetUniformValue(std::string var, glm::vec3 values);
    void shaderSetUniformValue(std::string var, glm::mat4& values);
    void shaderSetUniformValue(std::string var, float values);

    unsigned int loadTexture(const char* filename);
    void applyTexture(const char* textureFilePath);

private:
    SDL_Window* sdlWin;

    GLuint vao;
    GLuint shader;
    GLuint vertexBuffer;
    GLuint normalBuffer;

    GLuint projID;
    GLuint viewID;
    GLuint modID;
    GLuint totID;
    GLuint viewPOSID;

    GLuint ambient;
    GLuint diffuse;
    GLuint specular;
    GLuint shininess;

    GeometryData geometryData;
    int colorLoc;
    int colourIdx;
    int vertexLoc;
    int normalLoc;
    
    glm::mat4 modelMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 model;
    glm::mat4 MVP;
    glm::mat4 totalTransformations;
    glm::mat4 light1Transformations;
    glm::mat4 light2Transformations;

    glm::vec3 light1Pos;
    glm::vec3 light1Col;
    glm::vec3 light2Pos;
    glm::vec3 light2Col;

    glm::vec3 lightCenter;
    float lightRadius = 1.5f;
    float lightRotationSpeed = glm::pi<float>() / 2.0f;  // Rotate 90 degrees per second
    float lightAngle = 0.0f;

    char transformationMode;
    char axis;
    bool isLightSource1;
};

#endif
