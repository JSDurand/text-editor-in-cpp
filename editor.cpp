#include <iostream>
#include "editor.h"
#include "resource_manager.h"
#include "text_renderer.h"
#include "regex.h"
#include <glm/gtc/matrix_transform.hpp>
// #define DEBUG

#ifdef DEBUG
#define print_debug()                                                                            \
  std::cout << "DEBUG BEGIN" << std::endl;                                                       \
  std::cout << "Column: "    << std::to_string(column)          << std::endl;                    \
  if (result) print_map(match_data);                                                             \
  std::cout << "Cursor: "    << std::to_string(this->cursorPos) << std::endl;                    \
  std::cout << "At cursor: " << this->buffer.content.SubString(this->cursorPos, 2) << std::endl; \
  std::cout << "DEBUG END"   << std::endl
#endif

// SpriteRenderer *Renderer;

// GameObject *Player;

// Ball *Boule;

// ParticleGenerator *Particles;

TextRenderer *Text;

Box *BoxRend;

Editor::Editor(GLuint width, GLuint height) 
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height),
    buffer(Buffer("Bienvenu au merveilleux\nprogram sur la terre!")) {}

Editor::~Editor() {
}

void Editor::Init()
{
  // resources
  ResourceManager::LoadShader("shader.vs", "shader.fs", nullptr, "sprite");
  ResourceManager::LoadShader("cursor.vs", "cursor.fs", nullptr, "box");

  glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
                                    static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
  ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
  ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projection);
  // ResourceManager::GetShader("cursor").Use().SetMatrix4("projection", projection);
    
  // Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

  // text
  Text = new TextRenderer(this->Width, this->Height);
  Text->Load("fonts/Droid Sans Mono for Powerline.otf", 24);
  // Text->Load("fonts/Cardo104s.ttf", 24);

  // box
  BoxRend = new Box(this->Width, this->Height);
  BoxRend->color = glm::vec4(1.0f, 0.5f, 0.3f, 1.0f);

  // buffer
  // this->buffer = Buffer("Bienvenu au merveilleux\nprogram sur la terre!");
  this->buffer.load_file("./main.cpp");

  // cursor
  this->cursorPos = 0;
}

void Editor::Update()
{
}


void Editor::ProcessInput()
{
}

void Editor::Render()
{
  // Text->RenderText(this->buffer.SubString(0).c_str(), 10, 10, 1, glm::vec3(1.0f),
  //                  this->cursorPos, *BoxRend, this->buffer.WindowStart());
  Text->RenderText(&this->buffer, 10, 10, 1, glm::vec3(1.0f),
                   *BoxRend, this->buffer.WindowStart());

  if (this->buffer.mark_activated) {
    std::cout << "marked" << std::endl;
  }
  // std::cout << this->buffer.to_string() << std::endl;
  // std::cout << this->buffer.SubString(this->buffer.WindowStart()) << std::endl;
}

void Editor::scroll(int offset)
{
  this->buffer.scroll(offset);
}

void Editor::backspace()
{
  if (this->cursorPos > 0 && this->cursorPos <= this->buffer.Length()) {
    --this->cursorPos;
    this->buffer.remove(this->cursorPos, 1);
  } else {
    // TODO: alert system
  }
}

void Editor::move(int delta)
{
  if (this->cursorPos + delta >= 0 && this->cursorPos + delta <= this->buffer.Length())
    this->cursorPos += delta;
}

void Editor::end_of_line (int num)
{
  vInst prog = {Inst(SPLIT, '0', 3, 1), Inst(ANY), Inst(JUMP, '0', 0),
                Inst(OP_LEFT), Inst(CHAR, '\n'), Inst(OP_RIGHT), Inst(MATCH)};
  mint match_data;

  int result = regex_vm_piece_table(prog, this->buffer.content, match_data, this->cursorPos,
                                    -1, num, true);

  if (result == 1)
    this->cursorPos += match_data[0] - 1;
}

void Editor::beginning_of_line(int num)
{
  vInst prog = {Inst(SPLIT, '0', 3, 1), Inst(ANY), Inst(JUMP, '0', 0),
                Inst(OP_LEFT), Inst(CHAR, '\n'), Inst(OP_RIGHT), Inst(MATCH)};
  mint match_data;

  int result = regex_vm_piece_table(prog, this->buffer.content, match_data, this->cursorPos,
                                    -1, num, false);

  if (result == 1)
    this->cursorPos += -match_data[1] + 2;
  else
    this->cursorPos  = 0;
}

void Editor::forward_line(int num)
{
  if (num == 0)
    return;
  
  vInst prog = {Inst(SPLIT, '0', 3, 1), Inst(ANY), Inst(JUMP, '0', 0),
                Inst(OP_LEFT), Inst(CHAR, '\n'), Inst(OP_RIGHT), Inst(MATCH)};
  mint match_data;

  int result(0);

  mint c_match_data;
  int c_result = regex_vm_piece_table(prog, this->buffer.content, c_match_data, this->cursorPos,
                                      -1, 1, false);

  int column(0);
  bool end_of_line_p(false);

  if (c_result == 1 && c_match_data[0] == 0) {
    // The cursor is at the end of the line.
    end_of_line_p = true;
    c_match_data = mint{};
    c_result = regex_vm_piece_table(prog, this->buffer.content, c_match_data, this->cursorPos - 1,
                                    -1, 1, false);
  }

  if (c_result == 1) {
    column = c_match_data[0];
  } else {
    column = this->cursorPos + 1;
  }

  // empty line
  bool empty_line_p = ((c_result == 1 && c_match_data[0] == 0) || this->cursorPos == 0) && end_of_line_p;

// #ifdef DEBUG
//   std::cout << "Column: " << std::to_string(column) << std::endl;
//   print_map(c_match_data);
//   std::cout << "Cursor: " << std::to_string(this->cursorPos) << std::endl;
//   std::cout << "At cursor: " << this->buffer.content.SubString(this->cursorPos, 2) << std::endl;
// #endif

  if (num > 0) {
    for (int i = 0; i < num; ++i) {
      match_data = mint{};
      result = regex_vm_piece_table(prog, this->buffer.content, match_data, this->cursorPos, -1, 1, true);

      if (result == 0) break;

#ifdef DEBUG
      print_debug();
#endif

      this->cursorPos += match_data[1];
    }

#ifdef DEBUG
      print_debug();
#endif

    match_data = mint{};
    result = regex_vm_piece_table(prog, this->buffer.content, match_data,
                                  this->cursorPos, this->cursorPos + column, 1, true);

    --(this->cursorPos);

    if (result == 1) // column becomes too large
      this->cursorPos += match_data[1];
    else
      this->cursorPos += empty_line_p ? 1 : column;

#ifdef DEBUG
      print_debug();
#endif

    if (this->cursorPos >= this->buffer.content.length())
      this->cursorPos = this->buffer.content.length() - 1;
  } else {
#ifdef DEBUG
    print_debug();
#endif
    int limit = empty_line_p ? 2 - num : 1 - num;
    for (int i = 0; i < limit; ++i) {
      match_data = mint{};
      result = regex_vm_piece_table(prog, this->buffer.content, match_data, this->cursorPos, -1, 1, false);

      if (result == 0) {
        this->cursorPos = -2;
        break;
      }

      this->cursorPos -= match_data[1];
#ifdef DEBUG
    print_debug();
#endif
    }

    this->cursorPos += 2;

#ifdef DEBUG
    print_debug();
#endif

    match_data = mint{};
    result = regex_vm_piece_table(prog, this->buffer.content, match_data,
                                  this->cursorPos, this->cursorPos + column, 1, true);

    if (result == 1 && match_data[0] <= column) { // column becomes too large
      if (match_data[0] > 0 && !empty_line_p) {
        this->cursorPos += match_data[0] - 1;
      } else if (empty_line_p) {
        this->cursorPos += match_data[0] - 1;
      }
    } else {
      this->cursorPos += column - 1;
    }

    if (empty_line_p)
      this->cursorPos += 1;

#ifdef DEBUG
    print_debug();
#endif

    if (this->cursorPos < 0)
      this->cursorPos = 0;
  }
}

int Editor::search (std::string regex, bool forward)
{
  vInst prog = compile_regex(regex);
  mint match_data;
  int result = regex_vm_piece_table(prog, this->buffer.content, match_data, this->cursorPos, forward);
#ifdef DEBUG
  std::cout << "result: " << result << std::endl;
  print_map(match_data);
#endif
  if (result == 1)
    this->cursorPos += forward ? match_data[1] : -match_data[1];
  return 0;
}

void Editor::copy()
{
  // this->buffer.content.append(this->buffer.content);
  // this->buffer.content.balance();
}

void Editor::resize(int width, int height)
{
  this->Width  = width;
  this->Height = height;
  Text->width  = width;
  Text->height = height;
  glm::mat4 matrix = glm::ortho(0.0f, static_cast<GLfloat>(width), static_cast<GLfloat>(height), 0.0f);

  // Load and configure shader
  Text->TextShader.Use();
  Text->TextShader.SetMatrix4("projection", matrix, GL_TRUE);
}

// TODO: keymap, modes.
