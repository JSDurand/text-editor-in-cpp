#include "box.h"
#include "piece.h"
#include "text_renderer.h"
#include <string>

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

  int cursorPos;
  int mark;
  bool mark_activated;

private:
  PieceTable content;
  int window_start = 0;
  bool modified = false;
};
