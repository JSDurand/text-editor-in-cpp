#include "piece.h"
#include <iostream>

// iterator

PieceIterator &PieceIterator::operator++()
{
 handleTable table = this->Table;

 // TableIndAndBufferInd tab(this->TableInd, this->BufferInd);

 // int orig_index               = table.get()->tableIndexToIndex(tab);
 // TableIndAndBufferInd new_tab = table.get()->indexToTableIndex(orig_index + 1);

 // this->TableInd  = new_tab.TableInd;
 // this->BufferInd = new_tab.BufferInd;

 // if at the end?
 Pieces ps = table->pieces;
 if (ps[this->TableInd].end <= this->BufferInd) {
   if (this->TableInd == ps.size() - 1) {
     this->BufferInd = ps[ps.size() - 1].end + 1;
   } else {
     this->TableInd += 1;
     this->BufferInd = ps[this->TableInd].start;
   }
 } else {
   this->BufferInd += 1;
 }
  
 return *this;
}

PieceIterator operator+(PieceIterator lhs, int rhs)
{
  Pieces ps = lhs.Table->pieces;

  while (rhs > 0) {
    if (ps[lhs.TableInd].end <= lhs.BufferInd) {
      if (lhs.TableInd == ps.size() - 1) {
        lhs.BufferInd = ps[ps.size() - 1].end + 1;
      } else {
        lhs.TableInd += 1;
        lhs.BufferInd = ps[lhs.TableInd].start;
      }
    } else {
      lhs.BufferInd += 1;
    }

    --rhs;
  }

  while (rhs < 0) {
    if (ps[lhs.TableInd].start >= lhs.BufferInd) {
      if (lhs.TableInd == 0) {
        lhs.BufferInd = ps[0].start - 1;
      } else {
        lhs.TableInd -= 1;
        lhs.BufferInd = ps[lhs.TableInd].end;
      }
    } else {
      lhs.BufferInd -= 1;
    }

    ++rhs;
  }

  return lhs;
}

std::string PieceIterator::value() const
{
  Piece p = this->Table.get()->pieces[this->TableInd];

  return p.buffer.get()->SubString(this->BufferInd, 1);
}

PieceIterator PieceTable::begin()
{
  handleTable table = std::make_shared<PieceTable>(*this);

  TableIndAndBufferInd pair = this->indexToTableIndex(0);

  return PieceIterator(pair.TableInd, pair.BufferInd, table);
}

PieceIterator PieceTable::end()
{
  handleTable table = std::make_shared<PieceTable>(*this);

  int len = this->length();

  TableIndAndBufferInd pair = indexToTableIndex(len - 1);

  return PieceIterator(pair.TableInd, pair.BufferInd, table);
}

bool operator==(PieceIterator lhs, PieceIterator rhs)
{
  return lhs.TableInd == rhs.TableInd && lhs.BufferInd == rhs.BufferInd;
}

bool operator!=(PieceIterator lhs, PieceIterator rhs)
{
  return !(lhs == rhs);
}

PieceData::PieceData(std::string data, PieceState state)
{
  this->content = data;
  this->length  = data.length();
  this->state   = state;
}

PieceData::~PieceData()
{
}

std::string PieceData::SubString(int start, int len) const
{
  return this->content.substr(start, len);
}

void PieceData::append(std::string data)
{
  if (this->state == RO) {
    std::cout << "Cannot write to this piece data!" << std::endl;
    return;
  }

  this->content.append(data);
  this->length += data.length();
}

Piece::Piece(handleData buffer, int start, int end, int len)
{
  this->buffer = buffer;
  this->start  = start;
  this->end    = end;
  this->length = len;
}

Piece::~Piece()
{
}

PieceTable::PieceTable(handleData orig)
{
  Piece p = Piece(orig, 0, orig->length - 1, orig->length);
  this->pieces = std::vector<Piece>{p};
  this->origBuffer = orig;
  this->addBuffer = handleData(new PieceData("", BOTH));
}

PieceTable::~PieceTable()
{
}

void PieceTable::insert(int index, std::string str)
{
  if (str.length() == 0)
    return;

  int len = str.length();

  TableIndAndBufferInd location = this->indexToTableIndex(index);

  Pieces ps = this->pieces;

  Piece p = ps[location.TableInd];

  // if points to addbuffer end and we are inserting after this piece, simply increase the length
  if (location.BufferInd == p.start && location.TableInd > 0 &&
      ps[location.TableInd - 1].buffer == this->addBuffer && 
      ps[location.TableInd - 1].end == this->addBuffer.get()->length - 1) {
  // if (p.buffer != nullptr && location.BufferInd == p.start &&
  //     p.start == p.buffer.get()->length - 1) {
    this->addBuffer.get()->append(str);
    this->pieces[location.TableInd - 1].length += len;
    this->pieces[location.TableInd - 1].end += len;
    return;
  }

  Piece newPiece = Piece(this->addBuffer,
                         this->addBuffer.get()->length, this->addBuffer.get()->length + len - 1,
                         len);

  Piece left = Piece(p.buffer,
                     p.start, location.BufferInd - 1, location.BufferInd - p.start);
  Piece right = Piece(p.buffer,
                      location.BufferInd, p.end, p.end - location.BufferInd + 1);

  int control(0);

  if (location.BufferInd - p.start > 0)
    control += 1;
  if (p.end - location.BufferInd + 1 > 0)
    control += 2;

  this->pieces.erase(this->pieces.begin() + location.TableInd, this->pieces.begin() + location.TableInd + 1);

  if (control & 2) {
    this->pieces.insert(this->pieces.begin() + location.TableInd, right);
  }

  this->pieces.insert(this->pieces.begin() + location.TableInd, newPiece);

  if (control & 1) {
    this->pieces.insert(this->pieces.begin() + location.TableInd, left);
  }

  this->addBuffer.get()->append(str);
}

// transforms an index to the index in a table and its corresponding buffer's index
TableIndAndBufferInd PieceTable::indexToTableIndex(int index) const
{
  if (index < 0)
    std::cerr << "Index negative!" << std::endl;

  int remain = index;
  
  for (int i = 0; i < this->pieces.size(); ++i) {
    Piece p = this->pieces[i];
    if (remain < p.length) {
      return TableIndAndBufferInd(i, remain + p.start);
    }

    remain -= p.length;
  }

  return TableIndAndBufferInd(this->pieces.size() - 1, this->pieces[this->pieces.size() - 1].end + 1);
}

int PieceTable::tableIndexToIndex(TableIndAndBufferInd tab) const
{
  int index = 0;
  int tab_tab = tab.TableInd;
  int tab_buf = tab.BufferInd;

  for (int i = 0; i < tab_tab; ++i) {
    index += this->pieces[i].length;
  }

  index += tab_buf - this->pieces[tab_tab].start + 1;

  return index;
}

void PieceTable::remove(int index, int len)
{
  if (len == 0)
    return;

  TableIndAndBufferInd start = this->indexToTableIndex(index);
  TableIndAndBufferInd end   = this->indexToTableIndex(index + len);

  // std::cout << "start piece: " <<
  //   std::to_string(this->pieces[start.TableInd].start) <<
  //   " and " <<
  //   std::to_string(this->pieces[start.TableInd].end) <<
  //   " and " <<
  //   std::to_string(start.BufferInd) << std::endl;

  // std::cout << "end piece: " <<
  //   std::to_string(this->pieces[end.TableInd].start) <<
  //   " and " <<
  //   std::to_string(this->pieces[end.TableInd].end) <<
  //   " and " <<
  //   std::to_string(end.BufferInd) << std::endl;

  if (start.TableInd == end.TableInd) {
    Piece p = this->pieces[start.TableInd];

    if (start.BufferInd == p.start) {
      if (len < p.length) {
        this->pieces[start.TableInd].start += len;
        this->pieces[start.TableInd].length -= len;
        return;
      }
    } else if (end.BufferInd == p.end) {
      this->pieces[start.TableInd].length -= len;
      this->pieces[start.TableInd].end    -= len;
      return;
    }
  }

  int control = 0; // controls which elements to join.

  Piece right, left;
  
  // end
  if (end.BufferInd < this->pieces[end.TableInd].end) {
    control += 1;
    right = Piece(this->pieces[end.TableInd].buffer,
                  end.BufferInd,
                  this->pieces[end.TableInd].end,
                  this->pieces[end.TableInd].end - end.BufferInd);
  }

  // start
  if (start.BufferInd > this->pieces[start.TableInd].start) {
    control += 2;
    left = Piece(this->pieces[start.TableInd].buffer,
                 this->pieces[start.TableInd].start,
                 start.BufferInd - 1,
                 start.BufferInd - this->pieces[start.TableInd].start);
  }

  this->pieces.erase(this->pieces.begin() + start.TableInd, this->pieces.begin() + end.TableInd + 1);
  
  if (control & 1) {
    this->pieces.insert(this->pieces.begin() + start.TableInd, right);
  }

  // start
  if (control & 2) {
    this->pieces.insert(this->pieces.begin() + start.TableInd, left);
  }

}

std::string PieceTable::to_string() const
{
  std::string result = "";
  Pieces ps = this->pieces;

  Pieces::const_iterator it;

  for (it = ps.begin(); it != ps.end(); ++it) {
    PieceData buf = *(it->buffer.get());
    if (it->buffer == this->addBuffer)
      result.append("buffer: add\n");
    else
      result.append(":buffer: orig\n");
    result.append("st: " + std::to_string(it->start) + ", and length: " + std::to_string(it->length) + "\n");
    result.append(buf.SubString(it->start, it->length) + "\n");
    // result.append(buf.SubString(it->start, it->length));
  }

  return result;
}

std::string PieceTable::SubString(int start, int length) const
{
  std::string result = "";
  Pieces ps = this->pieces;

  TableIndAndBufferInd debut = this->indexToTableIndex(start);
  TableIndAndBufferInd fin   = this->indexToTableIndex(start + length);

  // std::cout << std::to_string(debut.TableInd) << " and "
  //           << std::to_string(debut.BufferInd) << " and "
  //           << std::to_string(fin.TableInd) << " and "
  //           << std::to_string(fin.BufferInd) << std::endl;

  for (int i = debut.TableInd; i <= fin.TableInd; ++i) {
    int startIndex  = (i == debut.TableInd) ? debut.BufferInd : ps[i].start;
    int len(0);

    if (i == fin.TableInd) {
      if (i == debut.TableInd) {
        len = fin.BufferInd - debut.BufferInd;
      } else {
        len = fin.BufferInd - ps[i].start;
      }
    } else {
      len = ps[i].length;
    }
    
    // int endIndex = (i == fin.TableInd)   ? fin.BufferInd   : ps[i].end;

    // std::cout << "start: " << std::to_string(startIndex) << " and "
    //           << "len: " << std::to_string(len) << std::endl;

    result.append(ps[i].buffer.get()->SubString(startIndex, len));
  }

  return result;
}

std::string PieceTable::SubString(int start) const
{
  std::string result = "";
  Pieces ps = this->pieces;

  TableIndAndBufferInd debut = this->indexToTableIndex(start);

  for (int i = debut.TableInd; i <= ps.size(); ++i) {
    int startIndex  = (i == debut.TableInd) ? debut.BufferInd : ps[i].start;
    int len         = ps[i].length;
    // int endIndex = ps[i].end;

    result.append(ps[i].buffer.get()->SubString(startIndex, len));
  }

  return result;
}

int PieceTable::length() const
{
  int result = 0;

  Pieces ps = this->pieces;

  for (Pieces::const_iterator it = ps.begin(); it != ps.end(); ++it) {
    result += it->length;
  }

  return result;
}

// ARCHIVE

// void PieceTable::insert(int index, std::string str)
// {
//   // find the piece to insert
//   int len = str.length();
//   int count = 0;
//   Pieces::iterator it;

//   for (it = this->pieces.begin(); it != this->pieces.end(); ++it, ++count) 
//     if (count <= index && index < count + it->length) 
//       break;

//   Piece newPiece = Piece(this->addBuffer,
//                          this->addBuffer.get()->length, this->addBuffer.get()->length + len,
//                          len);

//   if (it == this->pieces.end()) { // just append at the end then
//     this->pieces.push_back(newPiece);
//     this->addBuffer.get()->append(str);
//     return;
//   }

//   if (count == index) {
//     this->pieces.insert(it, newPiece);
//   } else {
//     Pieces::size_type ind = it - this->pieces.begin();
//     Piece leftPiece  = Piece(it->buffer, it->start, it->start + index - count - 1, index - count);
//     Piece rightPiece = Piece(it->buffer, it->start + index - count, it->end, it->length - index + count);
//     this->pieces.erase(this->pieces.begin() + ind);
//     this->pieces.insert(this->pieces.begin() + ind, rightPiece);
//     this->pieces.insert(this->pieces.begin() + ind, newPiece);
//     this->pieces.insert(this->pieces.begin() + ind, leftPiece);
//   }

//   this->addBuffer.get()->append(str);
// }
