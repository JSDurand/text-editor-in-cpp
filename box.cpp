#include "box.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "resource_manager.h"

Box::Box(GLuint width, GLuint height)
{
  this->color = glm::vec4(1.0f);
  this->BoxShader = ResourceManager::LoadShader("cursor.vs", "cursor.fs", nullptr, "box");
  this->BoxShader.SetMatrix4("projection", glm::ortho(0.0f, static_cast<GLfloat>(width), static_cast<GLfloat>(height), 0.0f), GL_TRUE);
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glBindVertexArray(this->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 2, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Box::Draw(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
  this->BoxShader.Use();
  this->BoxShader.SetVector4f("cursorColor", this->color);
  glBindVertexArray(this->VAO);

  GLfloat vertices[] =
    {
     x    , y + h,
     x + w, y    ,
     x    , y    ,
     x    , y + h,
     x + w, y + h,
     x + w, y
    };

  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}
