#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "CS-330 Module 7 Project - Zion Johnson"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vao2;
        GLuint vbo[2];         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
        GLuint nPyramidVertices;
        GLuint pyramidVao;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture
    GLuint gTextureId;
    GLuint gTextureDeskId;
    GLuint gConcreteId;
    GLuint gRailsId;
    GLuint gRailSidesId;
    GLuint gCratesId;
    GLuint gTrainWhiteId;
    GLuint gSunId;
    GLuint gTrunkId;
    GLuint gRedPaintId;
    GLuint gMoonId;
    GLuint gWheelId;
    GLuint gGreyId;
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;



    // Shader programs
    GLuint gCubeProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    bool isDark = false;

    float adjustSpeed;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gDeskPosition(0.0f, -3.1f, 0.0f);
    glm::vec3 gPyramidPosition(0.0f, -2.0f, 0.0f);
    glm::vec3 gCubeScale(2.0f);
    glm::vec3 gDeskScale(1.0f);

    // Cube and light color
    //m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    glm::vec3 gObjectColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightColor(0.8f, 0.8f, 0.8f);
    //glm::vec3 gLightColor(1.0f, 0.99f, 0.95f);
    glm::vec3 gDarkLightColor(0.2f, 0.1f, 0.2f);
    /*
    glm::vec3 gLightColor(0.0f, 0.1f, 0.0f);
    glm::vec3 gKeyLightColor(1.0f, 1.0f, 1.0f);
    */

    // Light position and scale
    glm::vec3 gLightPosition(-5.0f, 5.0f, 8.0f);
    glm::vec3 gDarkLightPosition(0.0f, -10.0f, 0.0f);
    glm::vec3 gLightScale(0.5f);
    glm::vec3 gDarkLightScale(0.5f);

    glm::mat4 modelDesk;
    glm::mat4 modelPyramid;
    glm::mat4 modelBody;
    glm::mat4 modelCab;
    glm::mat4 modelTracks;

    // Lamp animation
    bool gIsLampOrbiting = false;
    bool isOrtho = false;


}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
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
const GLchar* cubeVertexShaderSource = GLSL(440,

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
uniform int orthographic;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;

}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 darkLightColor;
uniform vec3 keyLightPos;
uniform vec3 lightPos;
uniform vec3 darkLightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform sampler2D uTextureDesk;
uniform vec2 uvScale;


void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/


    //Calculate Ambient lighting*/
    float ambientStrength = 0.8f; // Set ambient or global lighting strength
        float darkAmbientStrength = 1.0f;
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color
    vec3 darkAmbient = darkAmbientStrength * darkLightColor;


    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube

        vec3 darkLightDirection = normalize(darkLightPos - vertexFragmentPos);

    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light

        float darkImpact = max(dot(norm, darkLightDirection), 0.0);

    vec3 diffuse = impact * lightColor; // Generate diffuse light color

        vec3 darkDiffuse = darkImpact * darkLightColor;

    //Calculate Specular lighting*/
    float specularIntensity = 10.0f; // Set specular light strength
    float highlightSize = 50.0f; // Set specular highlight size

    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector

    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);

    vec3 specular = specularIntensity * specularComponent * lightColor;

        vec3 darkSpecular = specularIntensity * specularComponent * darkLightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    //vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    vec3 phong = (ambient + darkAmbient + diffuse + darkDiffuse + specular + darkSpecular) * textureColor.xyz;

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


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

    uniform vec3 lightColor;

    uniform vec3 darkLightColor;

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    /*
    if (isDark = false) {
        fragmentColor = vec4(1.0f, 1.0f, 0.8f, 1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
    else
    {
        fragmentColor = vec4(0.2f, 0.1f, 0.5f, 1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
    */
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
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load textures
    //Tree Texture
    const char* texFilename = "leaves.png";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    //Desk Texture
    texFilename = "desk.png";
    if (!UCreateTexture(texFilename, gTextureDeskId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    //Desk Texture
    texFilename = "concrete.png";
    if (!UCreateTexture(texFilename, gConcreteId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    //Rails Texture
    texFilename = "rails.png";
    if (!UCreateTexture(texFilename, gRailsId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Rail Sides Texture
    texFilename = "railsides.png";
    if (!UCreateTexture(texFilename, gRailSidesId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    //Crates Texture
    texFilename = "crates.png";
    if (!UCreateTexture(texFilename, gCratesId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    //White Train Texture
    texFilename = "trainwhite.png";
    if (!UCreateTexture(texFilename, gTrainWhiteId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Sun Texture
    texFilename = "sun.png";
    if (!UCreateTexture(texFilename, gSunId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Wood Texture
    texFilename = "trunk.png";
    if (!UCreateTexture(texFilename, gTrunkId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Red Texture
    texFilename = "red.png";
    if (!UCreateTexture(texFilename, gRedPaintId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Moon Texture
    texFilename = "wheel.png";
    if (!UCreateTexture(texFilename, gWheelId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "grey.png";
    if (!UCreateTexture(texFilename, gGreyId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCubeProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);
    // Now we set uTextureDesk as texture unit 1.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTextureDesk"), 1);

    // Now we set uConcrete as texture unit 2.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uConcrete"), 2);

    // Now we set uRails as texture unit 3.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uRails"), 3);

    // Now we set uRailSides as texture unit 4.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uRailSides"), 4);

    // Now we set uCrates as texture unit 5.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uCrates"), 5);

    // Now we set as texture unit 6.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTrainWhite"), 6);

    // Now we set as texture unit 7.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uSun"), 7);

    // Now we set as texture unit 8.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTrunk"), 8);

    // Now we set as texture unit 9.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uRedPaint"), 9);

    // Now we set as texture unit 10.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uWheel"), 10);

    // Now we set as texture unit 10.
    glUseProgram(gCubeProgramId);
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uGrey"), 11);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureId);

    // Release shader programs
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
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

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    float cameraSpeed = 1.0f;
    cameraSpeed = cameraSpeed * adjustSpeed * gDeltaTime;
    float yDirSpeed = cameraSpeed / 3;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    //WASD functionality
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, cameraSpeed);


    //Up and down functionality

        //Press E to go up on y-axis
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, yDirSpeed);
    //Press Q to go down on y-axis
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, yDirSpeed);


    //Press Space Bar to go up on y-axis
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, yDirSpeed);
    //Press Left Control to go down on y-axis
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, yDirSpeed);

    //Press P to switch between perspective and orthographic
        //Press P to activate Perspective Mode (On by default)
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        isOrtho = false;
    }
    //Press O to activate Orthographic Mode
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        isOrtho = true;
    }


    //Light Controls

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            gLightPosition.z -= 0.03f;
        }
        else {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                gLightPosition.y += 0.03f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            gLightPosition.z += 0.03f;
        }
        else {
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                gLightPosition.y -= 0.03f;
            }
        }
    }



    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        gLightPosition.x -= 0.03f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        gLightPosition.x += 0.03f;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    //If the 'z' key is being pressed then it allows for zooming to occur. Otherwise the scroll wheel will adjust the camera speed.
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        gCamera.ProcessMouseScroll(yoffset);
    else
        adjustSpeed += 0.1f * yoffset;
    if (adjustSpeed < 0.1f)
        adjustSpeed = 0.1f;
    if (adjustSpeed > 10.0f)
        adjustSpeed = 10.0f;

}

// glfw: handle mouse button events
// --------------------------------
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
    // Lamp orbits around the origin
    const float angularVelocity = glm::radians(15.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * (gDeltaTime), glm::vec3(-1.0f, 0.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        glm::vec4 newDarkPosition = glm::rotate(angularVelocity * (gDeltaTime), glm::vec3(-1.0f, 0.0f, 0.0f)) * glm::vec4(gDarkLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
        gDarkLightPosition.x = newDarkPosition.x;
        gDarkLightPosition.y = newDarkPosition.y;
        gDarkLightPosition.z = newDarkPosition.z;
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gMesh.vao);

    // CUBE: draw cube
    //----------------
    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);


    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();
    GLint orthoLoc = glGetUniformLocation(gCubeProgramId, "isOrtho");
    glUniform1i(orthoLoc, isOrtho);

    // Creates a perspective projection
    glm::mat4 projection;

    if (isOrtho)
    {
        float orthoSize = 10.0f;
        projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    };


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");


    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint darkLightColorLoc = glGetUniformLocation(gCubeProgramId, "darkLightColor");
    GLint darkLightPositionLoc = glGetUniformLocation(gCubeProgramId, "darkLightPos");


    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);

    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);

    glUniform3f(darkLightColorLoc, gDarkLightColor.r, gDarkLightColor.g, gDarkLightColor.b);
    glUniform3f(darkLightPositionLoc, gDarkLightPosition.x, gDarkLightPosition.y, gDarkLightPosition.z);


    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


    //DRAW: Table
    //----------------------------
    modelDesk = glm::translate(gDeskPosition) * glm::scale(glm::vec3(40.0f, 0.01f, 50.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDesk));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureDeskId);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    //DRAW: Track Sides
    glm::vec3 gTrackPosition(0.0f, -3.05f, 1.0f);
    modelTracks = glm::translate(gTrackPosition) * glm::scale(glm::vec3(10.0f, 0.1f, 0.2f));
    modelTracks = modelTracks * glm::translate(glm::vec3(0.0f, 2.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTracks));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRailSidesId);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrack2Position(0.0f, -3.05f, -1.0f);
    modelTracks = glm::translate(gTrack2Position) * glm::scale(glm::vec3(10.0f, 0.1f, 0.2f));
    modelTracks = modelTracks * glm::translate(glm::vec3(0.0f, 2.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTracks));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRailSidesId);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);



    //DRAW: Tracks
    const int nrows = 20;


   // glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    
    for (int i = 0; i < nrows; ++i)
    {
        glm::vec3 gInnerTrackPosition(9.5f, -5.75f, 0.0f);
        glm::vec3 gNewTrackPosition = gInnerTrackPosition;
        glm::mat4 modelInnerTracks = glm::translate(gInnerTrackPosition);

        //modelInnerTracks = modelInnerTracks * glm::translate(gNewTrackPosition);
        modelInnerTracks = modelInnerTracks + glm::translate(glm::vec3(-i, 0.0f, 0.0f));

        modelInnerTracks = modelInnerTracks * glm::scale(glm::vec3(0.2f, 0.05f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelInnerTracks));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gRailsId);
                // Draws the triangles
        glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    }

    //Background
    glm::vec3 gRadioPos(5.0f, 0.0f, -20.0f);
    glm::vec3 gAntennaPos(6.0f, 5.0f, -20.f);
    glm::mat4 modelRadio = glm::translate(gRadioPos);
    glm::mat4 modelAntenna = glm::translate(gAntennaPos);
    modelRadio = modelRadio * glm::scale(glm::vec3(3.0f, 7.0f, 2.0f));
    modelAntenna = modelAntenna * glm::scale(glm::vec3(0.5f, 7.0f, 0.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRadio));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gGreyId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAntenna));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);


    const float rotationAnglez = glm::radians(5.0f);
    glm::mat4 rotationz = glm::rotate(rotationAnglez, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 gKeyboard(-8.0f, -3.0f, -20.0f);
    glm::mat4 modelKeyboard = glm::translate(gKeyboard) * rotationz;
    modelKeyboard = modelKeyboard * glm::scale(glm::vec3(20.0f, 0.5f, 6.0f));
    //modelKeyboard = modelKeyboard * rotationz;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelKeyboard));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);




    //DRAW TRAIN
    //----------------------------------------------------------------------------------------------------
    glm::vec3 gTrainPosition(-1.0f, -2.6f, 0.0f);
    glm::mat4 modelTrain = glm::translate(gTrainPosition);
    glm::mat4 modelTrain1 = modelTrain; //Bottom
    modelTrain1 = modelTrain1 * glm::scale(glm::vec3(5.0f, 0.2f, 1.2f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain1));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);



    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain1));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    /*
    glm::mat4 modelTrain2 = modelTrain;
    modelTrain2 = modelTrain2 * glm::scale(glm::vec3(0.7f, 2.0f, 0.9f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain2));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    */
    glm::vec3 gTrainPosition2(-1.0f, -2.0f, 0.0f);
    glm::mat4 modelTrain3 = glm::translate(gTrainPosition2);
    modelTrain3 = modelTrain3 * glm::scale(glm::vec3(1.1f, 1.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain3));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrainPosition3(-1.0f, -0.5f, 0.0f);
    glm::mat4 modelTrain4 = glm::translate(gTrainPosition3);
    modelTrain4 = modelTrain4 * glm::scale(glm::vec3(1.2f, 0.3f, 1.2f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain4));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRedPaintId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    gTrainPosition3 = glm::vec3(-1.0f, -0.8f, 0.0f);
    glm::mat4 modelTrain5 = glm::translate(gTrainPosition3);
    modelTrain5 = modelTrain5 * glm::scale(glm::vec3(1.0f, 0.3f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain5));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrainPolePosition(-1.4f, -1.0f, 0.0f);
    glm::mat4 modelTrainCab = glm::translate(gTrainPolePosition);
    modelTrainCab = modelTrainCab * glm::scale(glm::vec3(0.3f, 1.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainCab));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrainPolePosition2(-0.6f, -1.0f, 0.0f);
    glm::mat4 modelTrainCab2 = glm::translate(gTrainPolePosition2);
    modelTrainCab2 = modelTrainCab2 * glm::scale(glm::vec3(0.3f, 1.0f, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainCab2));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrainEnginePos(-2.2f, -2.2f, 0.0f);
    glm::mat4 modelTrainEngine = glm::translate(gTrainEnginePos);
    glm::mat4 modelTrainEngine2 = modelTrainEngine * glm::scale(glm::vec3(1.7f, 0.6f, 0.8f));
    glm::vec3 gTrainEngineTipPos(-3.1f, -2.2f, 0.0f);
    glm::mat4 modelTrainEngineTip = glm::translate(gTrainEngineTipPos);
    modelTrainEngineTip = modelTrainEngineTip * glm::scale(glm::vec3(0.2f, 0.5f, 0.6f));
    modelTrainEngine = modelTrainEngine * glm::scale(glm::vec3(1.7f, 0.7f, 0.7f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainEngine));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRedPaintId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainEngine2));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainEngineTip));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrainEngineMidPos(-2.5f, -2.2f, 0.0f);
    glm::mat4 modelTrainEngineMid = glm::translate(gTrainEngineMidPos);
    modelTrainEngineMid = modelTrainEngineMid * glm::scale(glm::vec3(0.2f, 0.75f, 0.85f));
    glm::vec3 gTrainEngineMidPos2(-1.7f, -2.2f, 0.0f);
    glm::mat4 modelTrainEngineMid2 = glm::translate(gTrainEngineMidPos2);
    modelTrainEngineMid2 = modelTrainEngineMid2 * glm::scale(glm::vec3(0.2f, 0.75f, 0.85f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainEngineMid));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainEngineMid2));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gTrainEngineStackPos(-2.8f, -1.6f, 0.0f);
    glm::vec3 gTrainEngineStackPos2(-2.1f, -1.6f, 0.0f);
    glm::vec3 gTrainEngineStackPos3(-2.1f, -1.3f, 0.0f);
    glm::vec3 gTrainStackCapPos(-2.1f, -1.15f, 0.0f);
    glm::vec3 gTrainStackCapPos2(-2.8f, -1.3f, 0.0f);
    glm::mat4 modelEngineStack = glm::translate(gTrainEngineStackPos);
    glm::mat4 modelEngineStack2 = glm::translate(gTrainEngineStackPos2);
    glm::mat4 modelEngineStack3 = glm::translate(gTrainEngineStackPos3);
    glm::mat4 modelTrainStackCap = glm::translate(gTrainStackCapPos);
    glm::mat4 modelTrainStackCap2 = glm::translate(gTrainStackCapPos2);

    modelEngineStack = modelEngineStack * glm::scale(glm::vec3(0.1f, 0.4f, 0.1f));
    modelEngineStack2 = modelEngineStack2 * glm::scale(glm::vec3(0.2f, 0.4f, 0.2f));
    modelEngineStack3 = modelEngineStack3 * glm::scale(glm::vec3(0.25f, 0.3f, 0.25f));
    modelTrainStackCap = modelTrainStackCap * glm::scale(glm::vec3(0.35f, 0.1f, 0.35f));
    modelTrainStackCap2 = modelTrainStackCap2 * glm::scale(glm::vec3(0.15f, 0.15f, 0.15f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelEngineStack));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRedPaintId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelEngineStack2));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelEngineStack3));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainStackCap));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTrainWhiteId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrainStackCap2));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    glm::vec3 gInitWheelPos(-3.0f, -3.6f, -0.35f);
    glm::vec3 gInitRWheelPos = glm::vec3(-3.0f, -3.6f, -1.65f);

    //Wheels
    const int lWheel = 7;
    for (int n = 0; n < lWheel; ++n) 
    {

        int k = n * 0.8;
        glm::vec3 gNewWheelPos = gInitWheelPos;

        glm::mat4 modelLWheel = glm::translate(gNewWheelPos + glm::vec3(k, 1.0f, 1.0f));
        modelLWheel = modelLWheel * glm::scale(glm::vec3(0.45f, 0.45f, 0.1f));

        glm::mat4 modelRWheel = glm::translate(gInitRWheelPos + glm::vec3(k, 1.0f, 1.0f));
        modelRWheel = modelRWheel * glm::scale(glm::vec3(0.45f, 0.45f, 0.1f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelLWheel));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gWheelId);
        glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRWheel));
        glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    }

    //------------------------------------------------------------------------------------------------------

        //DRAW CRATES

    const float rotationAngle = glm::radians(90.0f);
    glm::mat4 rotation = glm::rotate(rotationAngle, glm::vec3(-1.0f, 0.0f, 0.0f));
    glm::vec3 gCratePos(0.0f, -2.15f, 0.0f);
    glm::vec3 gCrate2Pos(1.0f, -2.15f, 0.0f);
    glm::mat4 modelCrate = glm::translate(gCratePos) * glm::scale(glm::vec3(0.7f, 0.7f, 0.7f)) * rotation;
    glm::mat4 modelCrate2 = glm::translate(gCrate2Pos) * glm::scale(glm::vec3(0.7f, 0.7f, 0.7f)) * rotation;
    //modelCrate = modelCrate - glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCrate));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRedPaintId);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCrate2));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    //DRAW: Trees
    //----------------------------

    const bool coolCube = true;
    if (coolCube == true)
    {
        glm::vec3 treePos(0.0f,  -2.3f, 0.0f);
        glm::vec3 trunkPos(0.0f, -2.3f, 0.0f);
        glm::mat4 scaleFactor = glm::scale(glm::vec3(0.1f, 0.2f, 0.1f));
        glm::mat4 modelTree = glm::translate(treePos) * glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
        modelTree = modelTree * scaleFactor;


        const int jrows = 2;
        for (int j = 0; j < jrows; ++j)
        {

            glm::mat4 modelTree1 = modelTree + glm::translate(treePos) + glm::scale(glm::vec3(1.0f, -0.1f, 1.0f)); //Bottom
            glm::mat4 modelTree2 = modelTree + glm::translate(treePos + glm::vec3(0.0f, 0.8f, 0.0f)) + glm::scale(glm::vec3(0.8f, -0.2f, 0.8f));
            glm::mat4 modelTree3 = modelTree + glm::translate(treePos + glm::vec3(0.0f, 1.4f, 0.0f)) + glm::scale(glm::vec3(0.4f, -0.3f, 0.4f));
            glm::mat4 modelTree4 = modelTree + glm::translate(treePos + glm::vec3(0.0f, 2.0f, 0.0f)) + glm::scale(glm::vec3(0.1f, -0.5f, 0.1f)); //Top
            glm::mat4 modelTree5 = modelTree + glm::translate(treePos + glm::vec3(0.0f, 2.4f, 0.0f)) + glm::scale(glm::vec3(-0.5f, -0.3f, -0.5f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTree1));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gTextureId);
            // Draws the triangles
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTree2));
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTree3));
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTree4));
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTree5));
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            glm::mat4 modelTrunk = glm::translate(trunkPos + glm::vec3(0.0f, 0.55f, 0.0f)) * glm::scale(glm::vec3(0.1f, 0.2f, 0.1f)); //Trunk

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrunk));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gTrunkId);
            glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

            treePos = treePos + glm::vec3(3.0f, 0.0f, 0.0f);
            trunkPos = trunkPos + glm::vec3(1.0f, 0.0f, 0.0f);
        }

    };

    //Lamp Hats-------------------------------------
    glm::mat4 modelSun = glm::translate(gLightPosition) * glm::scale(glm::vec3(gLightScale + glm::vec3(0.2f)));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelSun));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gSunId);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    /*
    glm::mat4 modelMoon = glm::translate(gDarkLightPosition) * glm::scale(glm::vec3(gDarkLightScale + glm::vec3(0.2f)));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMoon));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gMoonId);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    */

    // LAMP: draw lamp
    //----------------
   
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    
    // Dark Lamp: draw  dark lamp
    //------------------------------------------------------------
     /*
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gDarkLightPosition) * glm::scale(gDarkLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    */
    

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        // 
        //Cube
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // 1
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // 
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // 
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // 
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // 
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // 

       //Front Face         //Positive Z Normal
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // 
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 

      //Left Face          //Negative X Normal
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // 
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // 
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // 
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // 
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // 
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // 

     //Right Face         //Positive X Normal
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // 
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // 
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // 
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // 
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // 
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // 

     //Bottom Face        //Negative Y Normal
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // 
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // 
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f

    };



    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);


    glBindVertexArray(0);


    GLfloat pyramidVerts[] = {
                //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.

        /*
       -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // 3  0
        1.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // 0  1
        0.0f,  2.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // 4  2

       //Front Face         //Positive Z Normal
      -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 2  3
       1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 1  4
       0.0f,  2.0f,  0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 4  5

      //Left Face          //Negative X Normal
     -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 2  6
     -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // 3  7
      0.0f,  2.0f,  0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // 4  8

     //Right Face         //Positive X Normal
      1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 1
      1.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // 0
      0.0f,  2.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // 4
*/
     //Bottom Face        //Negative Y Normal
     -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // 0
     -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // 3
      1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 4
      1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 4
      1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 1
     -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // 0

      0.0f, -2.0f,  0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // 2
     -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // 3
      1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // 4



    };

 /*   GLushort indices[] =
    {
        0, 3, 4,
        4, 1, 0,

        0, 1, 2,
        4, 1, 2,
        3, 4, 2,
        0, 3, 2,
    }; *//*
    glGenVertexArrays(1, &mesh.vao2); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao2);
    // Create 2 buffers: first one for the vertex data; second one for the indices
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[1]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVerts), pyramidVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU
    
    mesh.nPyramidVertices = sizeof(pyramidVerts) / (sizeof(pyramidVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    
    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);


    glBindVertexArray(1); */
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbo);
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
