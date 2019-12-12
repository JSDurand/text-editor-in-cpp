#define GL_SILENCE_DEPRECATION
// #define __gl_h_
// #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "editor.h"
#include "resource_manager.h"
#include "text_renderer.h"

const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 768;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void char_callback(GLFWwindow *window, unsigned int codepoint);

void frame_buffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_button_callback(GLFWwindow *window, int button, int action, int mode);

Editor editor(WINDOW_WIDTH, WINDOW_HEIGHT);

int main(int argc, char **argv)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "DURAND", nullptr, nullptr);

  if (window == nullptr) {
    cout << "cannot create window: " << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "cannot load GLAD" << endl;
    glfwTerminate();
    return -1;
  }

  glfwSetKeyCallback(window, key_callback);
  glfwSetCharCallback(window, char_callback);
  glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  editor.Init();

  // GLfloat deltaTime = 0.0f, lastFrame = 0.0f;

  editor.State = GAME_ACTIVE;
  glfwSetWindowTitle(window, "main.cpp");

  while (!glfwWindowShouldClose(window)) {
    // while (glfwGetTime() - lastFrame < 1.0f / 50.0f) {}

    // GLfloat currentFrame = glfwGetTime();
    // deltaTime = currentFrame - lastFrame;
    // lastFrame = currentFrame;

    // editor.ProcessInput();
    editor.Update();

    glClearColor(0.0f, 0.0f, 0.0f, 0.4f);
    glClear(GL_COLOR_BUFFER_BIT);
    editor.Render();

    glfwSwapBuffers(window);
    glfwWaitEvents();
  }

  ResourceManager::Clear();

  glfwTerminate();
  return 0;
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) 
    editor.backspace();
  if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) 
    editor.move(-1);
  if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) 
    editor.move(1);
  if (key == GLFW_KEY_UP && action == GLFW_PRESS) 
    editor.forward_line(-1);
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) 
    editor.forward_line(1);
  if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) 
    editor.buffer.insert(editor.cursorPos, std::string("\n"));
  if (key == GLFW_KEY_E && action == GLFW_PRESS && mode & GLFW_MOD_CONTROL) 
    editor.end_of_line(1);
  if (key == GLFW_KEY_Q && action == GLFW_PRESS && mode & GLFW_MOD_CONTROL) 
    editor.beginning_of_line();
  if (key == GLFW_KEY_V && action == GLFW_PRESS && mode & GLFW_MOD_CONTROL) 
    editor.scroll(1);
  if (key == GLFW_KEY_V && action == GLFW_PRESS && mode & GLFW_MOD_ALT) 
    editor.scroll(-1);


  // if (key >= 0 && key < 1024) {
    // if (action == GLFW_PRESS) {
      // Breakout.Content = Breakout.Content + std::string(1, key);
      // Breakout.Keys[key] = GL_TRUE;
    // } else if (action == GLFW_RELEASE) {}
      // Breakout.Keys[key] = GL_FALSE;
  // }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
  if (codepoint <= 128 && codepoint >=0) {
    editor.buffer.insert(editor.cursorPos, std::string(1, (int)codepoint));
    editor.cursorPos += 1;
  }
}

void frame_buffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
  editor.resize(width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mode)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // TODO: how to know the corresponding position for the cursor position?
    double xp, yp;
    glfwGetCursorPos(window, &xp, &yp);
    editor.buffer.mark = Position(xp, yp);
    cout << "Pressed" << endl;
    cout << "mark, x: " + std::to_string(xp) + ", y: " + std::to_string(yp) << endl;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    cout << "Released" << endl;
  }
}
