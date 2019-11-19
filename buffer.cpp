#include "buffer.h"
#include <iostream>
#include <fstream>
#include <sstream>

Buffer::Buffer()
{
  handleData origB(new PieceData("", RO));
  this->content      = PieceTable(origB);
  this->window_start = 0;
  this->cursorPos    = 0;
}


Buffer::Buffer(std::string content)
{
  handleData origB(new PieceData(content, RO));
  this->content      = PieceTable(origB);
  this->window_start = 0;
  this->cursorPos    = 0;
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
  if (this->modified) { // ask to save file; ignore for now.
  }
    
  std::fstream file(str);
  std::stringstream temp;

  if (file.is_open()) {
    temp << file.rdbuf();
    handleData orig = std::make_shared<PieceData>(PieceData(temp.str(), RO));
    this->content = PieceTable(orig);
  }
}
