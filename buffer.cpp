#include "buffer.h"
#include "cursor.h"
#include "piece.h"
#include <iostream>
#include <fstream>
#include <sstream>

Buffer::Buffer()
  : mark (cursor{0})
{
  handleData origB(new PieceData("", RO));
  this->content              = PieceTable(origB);
  this->window_start         = 0;
  this->cursorPos            = 0;
  this->mark_activated       = false;
}


Buffer::Buffer(std::string content)
  : mark(cursor{0})
{
  handleData origB(new PieceData(content, RO));
  this->content              = PieceTable(origB);
  this->window_start         = 0;
  this->cursorPos            = 0;
  this->mark_activated       = false;
}

Buffer::~Buffer()
{
}

std::string Buffer::SubString(int start)
{
  int length = this->Length() - start;
  return this->content.SubString(start, length);
}

std::string Buffer::SubString(int start, int end)
{
  int length = end - start;
  return this->content.SubString(start, length);
}

std::string Buffer::to_string() const
{
  return this->content.to_string();
}

void Buffer::insert(int index, std::string str)
{
  this->content.insert(index, str);
}


void Buffer::remove(int index, int length)
{
  this->content.remove(index, length);
}


int Buffer::Length()
{
  return this->content.length();
}

void Buffer::load_file(const std::string str)
{
  if (this->modified) {
  // TODO: ask to save file; ignore for now, since I haven't implemented the
  // mechanism to change this parameter.
  }
    
  std::fstream file(str);
  std::stringstream temp;

  if (file.is_open()) {
    temp << file.rdbuf();
    handleData orig = std::make_shared<PieceData>(PieceData(temp.str(), RO));
    this->content = PieceTable(orig);
  }
}
