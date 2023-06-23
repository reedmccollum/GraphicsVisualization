#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Tutorial 6.3"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1024;
    const int WINDOW_HEIGHT = 756;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao, vao2, vao3, vao4, vao5;         // Handle for the vertex array object
        GLuint vbo, vbo2, vbo3, vbo4, vbo5;         // Handle for the vertex buffer object
        GLuint nVertices, nVertices2, nVertices3, nVertices4, nVertices5;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture
    GLuint gTextureId;
    GLuint gTexture2Id;
    GLuint gTexture3Id;
    GLuint gTexture4Id;
    GLuint gTexture5Id;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs
    GLuint tableProgramId;
    GLuint tableClothProgramId;
    GLuint diceProgramId;
    GLuint boxProgramId;
    GLuint candleProgramId;
    GLuint lightProgramId;

    GLUquadricObj* quad;
    

    // camera
    Camera gCamera(glm::vec3(0.0f, 1.0f, 3.5f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f, gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 tablePos(0.0f, 0.0f, 0.0f);
    glm::vec3 tableScale(0.4f);

    // Cube and light color
    glm::vec3 gObjectColor(1.f, 1.f, 1.f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

    // Light position and scale
    glm::vec3 gLightPosition(0.0f, 0.9f, 0.0f);
    glm::vec3 gLightScale(0.001f);

    // Lamp animation
    bool gIsLampOrbiting = true;

    // perspective changing
    bool changePersp = false;
}


bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* tableVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* tableFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.2f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);

/* Cube Vertex Shader Source Code*/
const GLchar* tableClothVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* tableClothFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture2; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.2f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.1f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 texture2Color = texture(uTexture2, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * texture2Color.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


//////////////////////////////


const GLchar* diceVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* diceFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture3; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.2f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 texture2Color = texture(uTexture3, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * texture2Color.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);

//////////////////////////////

//*******************************

const GLchar* boxVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* boxFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture4; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.8f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 texture2Color = texture(uTexture4, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * texture2Color.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);

//*******************************
const GLchar* candleVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Candle Fragment Shader Source Code*/
const GLchar* candleFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture5; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.8f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 texture2Color = texture(uTexture5, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * texture2Color.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}



int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(tableVertexShaderSource, tableFragmentShaderSource, tableProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(tableClothVertexShaderSource, tableClothFragmentShaderSource, tableClothProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(diceVertexShaderSource, diceFragmentShaderSource, diceProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(boxVertexShaderSource, boxFragmentShaderSource, boxProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(candleVertexShaderSource, candleFragmentShaderSource, candleProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, lightProgramId))
        return EXIT_FAILURE;


    // Load texture
    const char* texFilename = "wood.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    glUseProgram(tableProgramId);
    glUniform1i(glGetUniformLocation(tableProgramId, "uTexture"), 0);

    const char* tex2Filename = "fabric.jpg";
    if (!UCreateTexture(tex2Filename, gTexture2Id))
    {
        cout << "Failed to load texture " << tex2Filename << endl;
        return EXIT_FAILURE;
    }
    glUseProgram(tableClothProgramId);
    glUniform1i(glGetUniformLocation(tableClothProgramId, "uTexture2"), 0);

    const char* tex3Filename = "dice.jpg";
    if (!UCreateTexture(tex3Filename, gTexture3Id))
    {
        cout << "Failed to load texture " << tex3Filename << endl;
        return EXIT_FAILURE;
    }
    glUseProgram(diceProgramId);
    glUniform1i(glGetUniformLocation(diceProgramId, "uTexture3"), 0);
    
    const char* tex4Filename = "box.jpg";
    if (!UCreateTexture(tex4Filename, gTexture4Id))
    {
        cout << "Failed to load texture " << tex4Filename << endl;
        return EXIT_FAILURE;
    }
    glUseProgram(boxProgramId);
    glUniform1i(glGetUniformLocation(boxProgramId, "uTexture4"), 0);
    
    
    const char* tex5Filename = "candle.jpg";
    if (!UCreateTexture(tex5Filename, gTexture5Id))
    {
        cout << "Failed to load texture " << tex5Filename << endl;
        return EXIT_FAILURE;
    }
    glUseProgram(candleProgramId);
    glUniform1i(glGetUniformLocation(candleProgramId, "uTexture5"), 0);
    


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {

        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        UProcessInput(gWindow);
        URender();
        glfwPollEvents();
    }


    UDestroyMesh(gMesh);
    UDestroyTexture(gTextureId);
    UDestroyTexture(gTexture2Id);
    UDestroyShaderProgram(tableProgramId);
    UDestroyShaderProgram(tableClothProgramId);
    UDestroyShaderProgram(lightProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}



bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// Proccess inputs
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.Position.y += cameraSpeed * 0.01;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.Position.y -= cameraSpeed * 0.01;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (changePersp == true) {
            changePersp = false;
        }
        else {
            changePersp = true;
        }
    }

}


// Window Resizing
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
// Mouse postion actions
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}
// Mouse scroll actions
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// Mouse button actions
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.95f, 0.82f, 0.46f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pyramid VOA
    glBindVertexArray(gMesh.vao);


    // Set the shader to pyramid
    glUseProgram(tableProgramId);

    // Model matrix
    glm::mat4 model = glm::translate(tablePos) * glm::scale(tableScale);

    // Camera view Matrix
    glm::mat4 view = gCamera.GetViewMatrix();

    // Projection set as perspective
    glm::mat4 projection;
    if (changePersp == false) {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 5.0f);
    }
    // Pass model, view, and projection to pyramid 
    GLint modelLoc = glGetUniformLocation(tableProgramId, "model");
    GLint viewLoc = glGetUniformLocation(tableProgramId, "view");
    GLint projLoc = glGetUniformLocation(tableProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference Shader for object and light color, and light and view posistion, then pass to Pyramid shader uniforms
    GLint objectColorLoc = glGetUniformLocation(tableProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(tableProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(tableProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(tableProgramId, "viewPosition");
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(tableProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draw Pyramid
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glBindVertexArray(gMesh.vao2);


    // Set the shader to pyramid
    glUseProgram(tableClothProgramId);

    // Pass model, view, and projection to pyramid 
    modelLoc = glGetUniformLocation(tableClothProgramId, "model");
    viewLoc = glGetUniformLocation(tableClothProgramId, "view");
    projLoc = glGetUniformLocation(tableClothProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference Shader for object and light color, and light and view posistion, then pass to Pyramid shader uniforms
    objectColorLoc = glGetUniformLocation(tableClothProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(tableClothProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(tableClothProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(tableClothProgramId, "viewPosition");
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(tableClothProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture2Id);

    // Draw Pyramid
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices2);

    /////////////////////////
    glBindVertexArray(gMesh.vao3);


    // Set the shader to pyramid
    glUseProgram(diceProgramId);

    // Pass model, view, and projection to pyramid 
    modelLoc = glGetUniformLocation(diceProgramId, "model");
    viewLoc = glGetUniformLocation(diceProgramId, "view");
    projLoc = glGetUniformLocation(diceProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference Shader for object and light color, and light and view posistion, then pass to Pyramid shader uniforms
    objectColorLoc = glGetUniformLocation(diceProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(diceProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(diceProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(diceProgramId, "viewPosition");
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(diceProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture3Id);

    // Draw Pyramid
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices3);
    /////////////////////////
    //********************
    
    glBindVertexArray(gMesh.vao4);


    // Set the shader to pyramid
    glUseProgram(boxProgramId);

    // Pass model, view, and projection to pyramid 
    modelLoc = glGetUniformLocation(boxProgramId, "model");
    viewLoc = glGetUniformLocation(boxProgramId, "view");
    projLoc = glGetUniformLocation(boxProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference Shader for object and light color, and light and view posistion, then pass to Pyramid shader uniforms
    objectColorLoc = glGetUniformLocation(boxProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(boxProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(boxProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(boxProgramId, "viewPosition");
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(boxProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture4Id);

    // Draw Pyramid
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices4);
    
    //********************




    glBindVertexArray(gMesh.vao5);


    // Set the shader to pyramid
    glUseProgram(candleProgramId);

    // Pass model, view, and projection to pyramid 
    modelLoc = glGetUniformLocation(candleProgramId, "model");
    viewLoc = glGetUniformLocation(candleProgramId, "view");
    projLoc = glGetUniformLocation(candleProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference Shader for object and light color, and light and view posistion, then pass to Pyramid shader uniforms
    objectColorLoc = glGetUniformLocation(candleProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(candleProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(candleProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(candleProgramId, "viewPosition");
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(candleProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture5Id);

    // Draw Pyramid
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices5);


    

    


    glBindVertexArray(gMesh.vao2);

    // Light shader
    glUseProgram(lightProgramId);

    //Translate light to be based on prefered size / location
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Ref light shader for matrix info
    modelLoc = glGetUniformLocation(lightProgramId, "model");
    viewLoc = glGetUniformLocation(lightProgramId, "view");
    projLoc = glGetUniformLocation(lightProgramId, "projection");

    // Pass data to Light matrix
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices2);



    // Deactivate VAO and Shader
    glBindVertexArray(0);
    glUseProgram(0);


    glfwSwapBuffers(gWindow);
}



void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    const float repeat = 1.0f;

    //static_meshes_3d::Cylinder C(1, 2, 3, true, true, true);

    GLfloat verts[] = {
        //Positions          //Normals           // Texture Coords
        // --------------------------------------

        //// UNDER TOP LEFT            VERT 0
        //-2.0f,  0.0f, -1.0f, -1.0f, 0.0f, -1.0f,  repeat, 0.0f,
        //// UNDER TOP RIGHT           VERT 1
        //2.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  repeat, repeat,
        //// UNDER BOTTOM LEFT         VERT 2
        //-2.0f,  0.0f,  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        //// UNDER BOTTOM RIGHT        VERT 3
        //2.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f, repeat,
        //// OVER TOP LEFT             VERT 4
        //-2.0f,  1.0f, -1.0f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        //// OVER TOP RIGHT            VERT 5
        //2.0f,  1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        //// OVER BOTTOM LEFT          VERT 6
        //-2.0f,  1.0f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        //// OVER TOP RIGHT            VERT 7
        //2.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,


        //**************TABLE****************


        // SQUARE 0 verts 046620
        -2.0f,  0.0f, -1.0f, -1.0f, 0.0f, -1.0f,  repeat, 0.0f,
        -2.0f,  0.1f, -1.0f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        -2.0f,  0.1f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -2.0f,  0.1f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -2.0f,  0.0f,  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -2.0f,  0.0f, -1.0f, -1.0f, 0.0f, -1.0f,  repeat, 0.0f,

        // SQUARE 1 verts 154401

        2.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  repeat, repeat,
        2.0f,  0.1f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        -2.0f,  0.1f, -1.0f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        -2.0f,  0.1f, -1.0f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        -2.0f,  0.0f, -1.0f, -1.0f, 0.0f, -1.0f,  repeat, 0.0f,
        2.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  repeat, repeat,

        // SQUARE 2 verts 375513
        2.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f, repeat,
        2.0f,  0.1f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,
        2.0f,  0.1f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        2.0f,  0.1f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        2.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  repeat, repeat,
        2.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f, repeat,

        // SQUARE 3 verts 267732
        -2.0f,  0.0f,  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -2.0f,  0.1f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        2.0f,  0.1f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,
        2.0f,  0.1f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,
        2.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f, repeat,
        -2.0f,  0.0f,  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        // SQUARE 4 verts 645576
        -2.0f,  0.1f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -2.0f,  0.1f, -1.0f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        2.0f,  0.1f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        2.0f,  0.1f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        2.0f,  0.1f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,
        -2.0f,  0.1f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        // SQUARE 5 verts 023310
        -2.0f,  0.0f, -1.0f, -1.0f, 0.0f, -1.0f,  repeat, 0.0f,
        -2.0f,  0.0f,  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        2.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f, repeat,
        2.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f, repeat,
        2.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  repeat, repeat,
        -2.0f,  0.0f, -1.0f, -1.0f, 0.0f, -1.0f,  repeat, 0.0f,

        ////  ***********TABLE CLOTH*************

        //-2.0f,  1.0f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        //-2.0f,  1.0f, -1.0f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        //2.0f,  1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        //2.0f,  1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        //2.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,
        //-2.0f,  1.0f,  1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,



    };
    GLfloat verts2[] = {
        //Positions          //Normals           // Texture Coords
        // --------------------------------------


        //  ***********TABLE CLOTH*************

        -2.0f,  0.1001f,  0.4f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -2.0f,  0.1001f, -0.4f,   -1.0f, 1.0f, -1.0f, repeat, 0.0f,
        2.0f,  0.1001f, -0.4f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        2.0f,  0.1001f, -0.4f,  1.0f, 1.0f, -1.0f,  repeat, repeat,
        2.0f,  0.1001f,  0.4f,  1.0f, 1.0f, 1.0f,  0.0f, repeat,
        -2.0f,  0.1001f,  0.4f,  -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,



    };
    GLfloat verts3[] = {
        //Positions          //Normals           // Texture Coords
        // --------------------------------------

        //  ****************DICE(D4)************************

        //  Points
        // 
        // Top Point    Point 3
        //1.6f, 0.2f, 0.2f,    1.0f, 1.0f, 1.0f, 0.5f, 0.5f,
        // SW Point     Point 2
        //1.5f, 0.10001f, 0.1f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f,
        // SE Point     Point 1
        //1.7f, 0.10001f, 0.1f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f,
        // N Point      Point 0
        //1.6f, 0.10001f, 0.2f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f,

        // BOTTOM 012
        1.6f, 0.10001f, -0.25f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.65f, 0.10001f, -0.15f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.55f, 0.10001f, -0.15f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,

        // SOUTH FACE 123
        1.65f, 0.10001f, -0.15f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.55f, 0.10001f, -0.15f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.6f, 0.2f, -0.2f,     1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // NW FACE 013
        1.6f, 0.10001f, -0.25f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.65f, 0.10001f, -0.15f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.6f, 0.2f, -0.2f,     1.0f, 1.0f, 0.0f, 0.5f, 0.5f,

        // NE FACE 203
        1.55f, 0.10001f, -0.15f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.6f, 0.10001f, -0.25f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.6f, 0.2f, -0.2f,     1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    
    };


    const float upY = 0.10001f;
    GLfloat verts4[] = {
        
        //Positions          //Normals           // Texture Coords
        // --------------------------------------

        //  ****************BOX************************

        //  Points
        // Point 0
        //1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Point 1
        //1.2f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        // Point 2
        //1.7f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        // Point 3
        //1.7f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        // Point 4
        //1.2f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Point 5
        //1.2f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        // Point 6
        //1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        // Point 7
        //1.7f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,

        // Side 0 012230
        1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.7f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Side 1 015540
        1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.2f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.2f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.2f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Side 2 456674
        1.2f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.2f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Side 3 376623
        1.7f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.7f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.7f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Side 4 047730
        1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.7f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, 0.60001f, -0.5f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.2f, upY, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // Side 5 156621
        1.2f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, 0.60001f, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.7f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.2f, upY, -0.8f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,


    
    };
    GLfloat verts5[]{
        //Positions          //Normals           // Texture Coords
        // --------------------------------------

        //  ****************CANDLE************************

        // SIDE 0 01 8890
        1.0f, upY, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, upY, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.5f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.5f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.5f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, upY, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

        // SIDE 1 12 99 10 1
        1.0f, upY, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, upY, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.5f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.5f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.5f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, upY, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

        // SIDE 2 23 10 10 11 3
        1.2f, upY, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, upY, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.5f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.5f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.5f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, upY, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

        // SIDE 3 34 11 11 12 3
        1.2f, upY, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, upY, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.5f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.5f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.5f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, upY, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // SIDE 4 45 12 12 13 4
        1.3f, upY, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, upY, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.5f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.5f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.5f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, upY, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // SIDE 5 56 13 13 14 5
        1.3f, upY, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, upY, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.5f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.5f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.5f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, upY, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // SIDE 6 67 14 14 15 6
        1.1f, upY, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, upY, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.5f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.5f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.5f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, upY, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        // SIDE 7 70 15 15 8 7
        1.1f, upY, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, upY, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.5f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.5f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.5f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, upY, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        //BASE
        1.0f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.102f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.102f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.102f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.102f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.3f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.102f, -0.3f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.4f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.1f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 0.102f, -0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.2f, 0.102f, -0.1f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    };
    
    

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;
    // TABLE
    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a)
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // TABLE RUNNER
    mesh.nVertices2 = sizeof(verts2) / (sizeof(verts2[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao2);
    glBindVertexArray(mesh.vao2);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo2); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts2), verts2, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // DICE
    mesh.nVertices3 = sizeof(verts3) / (sizeof(verts3[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao3);
    glBindVertexArray(mesh.vao3);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo3);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo3); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts3), verts3, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // BOX
    mesh.nVertices4 = sizeof(verts4) / (sizeof(verts4[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao4);
    glBindVertexArray(mesh.vao4);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo4);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo4); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts4), verts4, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    // CANDLE
    mesh.nVertices5 = sizeof(verts5) / (sizeof(verts5[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao5);
    glBindVertexArray(mesh.vao5);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo5);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo5); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts5), verts5, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
