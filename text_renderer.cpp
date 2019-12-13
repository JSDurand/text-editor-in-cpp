#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "text_renderer.h"
#include "resource_manager.h"

TextRenderer::TextRenderer(GLuint width, GLuint height)
{
  this->width  = width;
  this->height = height;

  glm::mat4 matrix = glm::ortho(0.0f, static_cast<GLfloat>(width), static_cast<GLfloat>(height), 0.0f);

  // matrix = glm::translate(matrix, glm::vec3(0.0f, -70.0f, 0.0f));

  // Load and configure shader
  this->TextShader = ResourceManager::LoadShader("text.vs", "text.fs", nullptr, "text");
  this->TextShader.SetMatrix4("projection", matrix, GL_TRUE);
  this->TextShader.SetInteger("text", 0);
  // Configure VAO/VBO for texture quads
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glBindVertexArray(this->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void TextRenderer::Load(std::string font, GLuint fontSize)
{
    // First clear the previously loaded Characters
    this->Characters.clear();
    // Then initialize and load the FreeType library
    FT_Library ft;    
    if (FT_Init_FreeType(&ft)) // All functions return a value different than 0 whenever an error occurred
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, font.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
    // Then for the first 128 ASCII characters, pre-load/compile their characters and store them
    for (GLubyte c = 0; c < 128; c++) // lol see what I did there 
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
            );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale,
                              glm::vec3 color, GLuint cursor, Box box, int offset)
{
  GLfloat originX = x;
  // Activate corresponding render state	
  this->TextShader.Use();
  this->TextShader.SetVector3f("textColor", color);

  float float_offset = -offset * 20 * scale;
  // float float_offset = -offset * static_cast<float>(this->Characters['H'].Bearing.y) * scale;
  y += float_offset;

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(this->VAO);

  GLuint count = 0;

  // Iterate through all characters
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++) {
    Character ch = Characters[*c];

    GLfloat xpos = x + ch.Bearing.x * scale;
    GLfloat ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

    if (ypos >= this->height)
      break;

    GLfloat w = ch.Size.x * scale;
    GLfloat h = ch.Size.y * scale;

    if (w == 0)
      w = (ch.Advance >> 6) * scale;

    // cursor
    if (count == cursor) {
      box.Draw(xpos, y, w, 20);
      this->TextShader.Use();
      glBindVertexArray(this->VAO);
    }

    if (*c == ' ' || *c == '\t') { // don't draw anything if it is a space.
      x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
      ++count;
      continue;
    } else if (*c == '\n') {
      y += 20;
      x  = originX;
      ++count;
      continue;
    } else if (xpos + w >= this->width) {
      y += 20;
      x  = originX;
      xpos = x + ch.Bearing.x * scale;
      ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;
    }

    // Update VBO for each character
    GLfloat vertices[6][4] = {
                              { xpos,     ypos + h,   0.0, 1.0 },
                              { xpos + w, ypos,       1.0, 0.0 },
                              { xpos,     ypos,       0.0, 0.0 },

                              { xpos,     ypos + h,   0.0, 1.0 },
                              { xpos + w, ypos + h,   1.0, 1.0 },
                              { xpos + w, ypos,       1.0, 0.0 }
    };
    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Now advance cursors for next glyph
    x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)

    ++count;
  }

  if (count == cursor) {
    box.Draw(x, y, 5, 20);
    this->TextShader.Use();
    glBindVertexArray(this->VAO);
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// NOTE: A renderer should take buffer as an argument.
//
// FIXME: The two params x and y should be replaced by a window object in the
// end.
//
// FIXME: The param scale should be incorporated by the buffer argument.
//
// FIXME: The param offset is the offset of the buffer, and should be included
// by the buffer argument.
void TextRenderer::RenderText(Buffer *buffer, GLfloat x, GLfloat y, GLfloat scale,
                              glm::vec3 color, Box box, int offset)
{

  std::string text = buffer->SubString(0).c_str();
  GLfloat originX  = x;
  std::vector<cursor>::iterator cursor_iter (buffer->cursorPos.begin());
  std::vector<cursor>::iterator cursor_iter_end (buffer->cursorPos.end());
  std::pair<long, long> region (std::make_pair<long, long>(0, 0));
  std::pair<long, long> position (std::make_pair<double, double>(0, 0));
  // cursor fake_cursor_head (fakeCursor.front());
  // Activate corresponding render state
  this->TextShader.Use();
  this->TextShader.SetVector3f("textColor", color);

  float float_offset = -offset * 20 * scale;
  // float float_offset = -offset * static_cast<float>(this->Characters['H'].Bearing.y) * scale;
  y += float_offset;

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(this->VAO);

  GLuint count = 0;

  // Iterate through all characters
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++) {
    Character ch = Characters[*c];

    GLfloat xpos = x + ch.Bearing.x * scale;
    GLfloat ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

    if (ypos >= this->height)
      break;

    GLfloat w = ch.Size.x * scale;
    GLfloat h = ch.Size.y * scale;

    if (w == 0)
      w = (ch.Advance >> 6) * scale;

    // cursors
    // if (count == cursor) {
    //   box.Draw(xpos, y, w, 20);
    //   this->TextShader.Use();
    //   glBindVertexArray(this->VAO);
    // }

    if (cursor_iter != cursor_iter_end) {
      switch (cursor_iter->cursor_branch()) {
        case BUFFER_LOC:
          if (count == cursor_iter->get_buffer_location()) {
            box.Draw(xpos, y, w, 20);
            this->TextShader.Use();
            glBindVertexArray(this->VAO);
            cursor_iter++;
          }
          break;
        case BUFFER_REG:
          region = cursor_iter->get_buffer_region();
          if (count >= region.first && count <= region.second) {
            box.Draw(xpos, y, w, 20);
            this->TextShader.Use();
            glBindVertexArray(this->VAO);
            cursor_iter++;
          }
          break;
        case SCREEN_POS:
          position = cursor_iter->get_screen_position();
          if (position.first >= xpos &&
              position.first <= xpos + w &&
              position.second >= ypos &&
              position.second <= ypos + h) {
            *cursor_iter = cursor(static_cast<long>(count));
          }
          break;
      }
    }

    if (*c == ' ' || *c == '\t') { // don't draw anything if it is a space.
      x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
      ++count;
      continue;
    } else if (*c == '\n') {
      y += 20;
      x  = originX;
      ++count;
      continue;
    } else if (xpos + w >= this->width) {
      y += 20;
      x  = originX;
      xpos = x + ch.Bearing.x * scale;
      ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;
    }

    // Update VBO for each character
    GLfloat vertices[6][4] = {
                              { xpos,     ypos + h,   0.0, 1.0 },
                              { xpos + w, ypos,       1.0, 0.0 },
                              { xpos,     ypos,       0.0, 0.0 },

                              { xpos,     ypos + h,   0.0, 1.0 },
                              { xpos + w, ypos + h,   1.0, 1.0 },
                              { xpos + w, ypos,       1.0, 0.0 }
    };
    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    // NOTE: Be sure to use glBufferSubData and not glBufferData
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Now advance cursors for next glyph
    x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)

    ++count;
  }

  // if (count == cursor) {
  //   box.Draw(x, y, 5, 20);
  //   this->TextShader.Use();
  //   glBindVertexArray(this->VAO);
  // }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}
