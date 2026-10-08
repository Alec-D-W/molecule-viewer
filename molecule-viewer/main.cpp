#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "shader.h"
#include "camera.h"
#include "volume.h"
#include "load_molecule.h"
#include <map>

// Visual Studio Build
const std::string Path = "";
// CMake Build
//const std::string Path = "../Uebung/";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, std::vector<Shader*> shaders = {});
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 100.0f));

bool mol = true;
bool vol = false;

// mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if(window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSwapInterval(1);
  glfwWindowHint(GLFW_SAMPLES, 4);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
 }

  Shader shader("shaders/model_vert.vs", "shaders/model_frag.fs");

  SphereMesh baseSphere = createSphereVAO();
  std::vector<Atom> molecule = loadPDB("models/2RT4.pdb");
  setupMolecule(molecule, baseSphere.vao);

  Shader volumeShader(Path + "shaders/volume_vert.vs", Path + "shaders/volume_frag.fs");
  unsigned int vol_mol = Gen3DTex(loadDx("models/volmap_7.dx"));
  volumeShader.Use();
  glActiveTexture(GL_TEXTURE0); // oder GL_TEXTURE1, je nach Slot
  glBindTexture(GL_TEXTURE_3D, vol_mol);
  glUniform1i(glGetUniformLocation(volumeShader.getID(), "volumeTex"), 0); // 0 = Slot
  BoundingBox();

  while (!glfwWindowShouldClose(window)) {
    // Process inputs, prepare camera matrices
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window, {&shader});
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::rotate(model, glm::radians(static_cast<float>(glfwGetTime())), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 500.0f);
    glm::mat4 mvp = proj * view * model;


    // Prepare Depth- and Stencil-Buffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw floor molecule
    if(mol) drawMolecule(shader.getID(), baseSphere.vao, baseSphere.vertexCount, molecule.size(), mvp);
    if (vol) {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        volumeShader.Use();
        volumeShader.SetInt("volumeTex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, vol_mol);
        volumeShader.SetMat4("mvp", mvp);
        volumeShader.SetVec3("cameraPos", camera.Position);
        glm::mat4 invMVP = glm::inverse(mvp);
        volumeShader.SetMat4("invMVP", invMVP);
        volumeShader.SetVec3("cameraPos", camera.Position);
        volumeShader.SetVec2("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

        // 🟢 Neue Uniforms für die Bounding Box
// main.cpp
        volumeShader.SetVec3("minCorner", minCorner);
        volumeShader.SetVec3("maxCorner", maxCorner);

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }



    //std::cout << "test";
    std::cout << "x: " << camera.Position.x << "y: " << camera.Position.y << "z: " << camera.Position.z << std::endl;
    glfwSwapBuffers(window);
    glfwPollEvents();

  }
        
  glfwTerminate();
  return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, std::vector<Shader*> shaders) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    for(auto& shader : shaders) shader->Refresh();
  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if(firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        if (mol)mol = false;
            else mol = true;
        std::cout << "mol: " << mol << std::endl;

    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {

        if (vol)vol = false;
        else vol = true;


    }
}

