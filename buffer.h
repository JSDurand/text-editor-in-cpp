#pragma once

#include "box.h"
#include "piece.h"
// #include "text_renderer.h"
#include "cursor.h"
#include <string>
#include <vector>

// NOTE: This is for storing positions of cursors.
// This should be replaced by the union cursor.
// struct Position
// {
//   Position (): xPos(0), yPos(0) {};
//   Position (double x, double y): xPos(x), yPos(y) {};
//   double xPos;
//   double yPos;
// };


class Buffer
{
public:
  Buffer();
  Buffer(std::string content);
  ~Buffer();

  friend struct Editor;

  std::string SubString(int start, int end);
  std::string SubString(int start);
  std::string to_string() const;

  void insert(int index, std::string str);
  void remove(int index, int length);
  void load_file(const std::string);

  int Length();

  int WindowStart() {return this->window_start;};

  void scroll(int offset) {
    this->window_start += offset;
    if (this->window_start < 0)
      this->window_start = 0;
  }

  // FIXME: Make cursorPos a list of cursors.
  std::vector<cursor> cursorPos;
  cursor mark;
  bool mark_activated;

private:
  PieceTable content;
  int window_start = 0;
  bool modified = false;
};
