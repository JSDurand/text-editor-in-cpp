#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

struct Box
{
public:
  glm::vec4 color;
  Box(GLuint, GLuint);
  Box(glm::vec3 couleur);

  Shader BoxShader;
  void Draw(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
private:
  GLuint VAO, VBO;
};
