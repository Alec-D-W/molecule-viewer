#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "shader.h"
#include "camera.h"
#include "volume.h"
#include "load_molecule.h"
#include "load_config.h"
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
    const AppConfig config = loadConfig("config/config.json");
    mol = config.rendering.showAtoms;
    vol = config.rendering.showVolume;
    camera.Zoom = config.camera.fovDegrees;
    lastX = static_cast<float>(config.window.width) * 0.5f;
    lastY = static_cast<float>(config.window.height) * 0.5f;

    if (!glfwInit()) {
      std::cerr << "Failed to initialize GLFW" << std::endl;
      return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, config.graphics.msaaSamples);
    GLFWwindow* window = glfwCreateWindow(
      config.window.width, config.window.height, "Molecule Viewer", NULL, NULL);
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
    glfwSwapInterval(config.window.vsync ? 1 : 0);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      glfwDestroyWindow(window);
      glfwTerminate();
      return -1;
  }
  glEnable(GL_MULTISAMPLE);
  int initialFramebufferWidth = 0;
  int initialFramebufferHeight = 0;
  glfwGetFramebufferSize(window, &initialFramebufferWidth, &initialFramebufferHeight);
  framebuffer_size_callback(window, initialFramebufferWidth, initialFramebufferHeight);

  Shader shader("shaders/model_vert.vs", "shaders/model_frag.fs");

  SphereMesh baseSphere = createSphereVAO(
      config.rendering.sphereSegments,
      config.rendering.sphereRings,
      config.rendering.sphereRadius);
  std::vector<Atom> molecule = loadPDB(config.molecule.atoms, config.atomStyles);
  glm::vec3 moleculeMin = molecule.front().position;
  glm::vec3 moleculeMax = molecule.front().position;
  for (const Atom& atom : molecule) {
      moleculeMin = glm::min(moleculeMin, atom.position);
      moleculeMax = glm::max(moleculeMax, atom.position);
  }
  const glm::vec3 moleculeCenter = (moleculeMin + moleculeMax) * 0.5f;
  camera.FocusOn(moleculeCenter, config.camera.positionOffset);
  setupMolecule(molecule, baseSphere.vao);

  Shader volumeShader(Path + "shaders/volume_vert.vs", Path + "shaders/volume_frag.fs");
  unsigned int vol_mol = Gen3DTex(loadDx(config.molecule.volume));
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
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::rotate(model, glm::radians(static_cast<float>(glfwGetTime())), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = camera.GetViewMatrix();
    float aspect = framebufferHeight > 0
        ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
        : static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
    glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 500.0f);
    glm::mat4 mvp = proj * view * model;


    // Prepare Depth- and Stencil-Buffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(config.graphics.backgroundColor.r, config.graphics.backgroundColor.g, config.graphics.backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (vol) {
      glCullFace(GL_FRONT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        volumeShader.Use();
        volumeShader.SetInt("volumeTex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, vol_mol);
        volumeShader.SetMat4("mvp", mvp);
        volumeShader.SetVec3("cameraPos", camera.Position);
        glm::mat4 invMVP = glm::inverse(mvp);
        volumeShader.SetMat4("invMVP", invMVP);
        volumeShader.SetVec2("screenSize", glm::vec2(
            static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight)));

        volumeShader.SetVec3("minCorner", minCorner);
        volumeShader.SetVec3("maxCorner", maxCorner);
        volumeShader.SetFloat("stepSize", config.rendering.volumeStepSize);
        volumeShader.SetInt("maxSteps", config.rendering.volumeMaxSteps);

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glCullFace(GL_BACK);
    }

    if(mol) {
      shader.Use();
      shader.SetVec3("ambientLightColor", config.graphics.ambientLightColor);
      shader.SetVec3("directionalLightColor", config.graphics.directionalLightColor);
      shader.SetVec3("directionalLightDirection", config.graphics.directionalLightDirection);
      shader.SetVec3("specularColor", config.graphics.specularColor);
      shader.SetVec3("cameraPos", camera.Position);
      shader.SetFloat("specularStrength", config.graphics.specularStrength);
      shader.SetFloat("shininess", config.graphics.shininess);
      drawMolecule(shader.getID(), baseSphere.vao, baseSphere.vertexCount, molecule.size(), mvp);
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

