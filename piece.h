#pragma once
#include <string>
#include <vector>
#include <memory>

enum PieceState {RO, BOTH};

struct TableIndAndBufferInd
{
  TableIndAndBufferInd() = default;
  TableIndAndBufferInd(int a, int b) : TableInd(a), BufferInd(b) {};
  int TableInd  = 0;
  int BufferInd = 0;
};

class PieceData
{
public:
  PieceData(std::string data, PieceState state);
  ~PieceData();

  std::string SubString(int start, int len) const;
  // Only provide append here, so that it is not possible to modify the data directly.
  void append(std::string data);

  unsigned int length;
private:
  std::string  content;
  PieceState   state;
};

struct Piece
{
  Piece() = default;
  Piece(std::shared_ptr<PieceData>, int, int, int);
  ~Piece();

  std::shared_ptr<PieceData> buffer;
  int                        start, end, length;
};

typedef std::vector<Piece> Pieces;
typedef std::shared_ptr<PieceData> handleData;

struct PieceIterator;

class PieceTable
{
public:
  using iterator = PieceIterator;
  
  PieceTable() = default;
  PieceTable(handleData orig);
  ~PieceTable();

  std::string to_string() const;

  std::string SubString(int start, int length) const;
  std::string SubString(int start) const;

  void insert(int index, std::string str);
  void remove(int index, int len);

  int length() const;

  iterator begin();
  iterator end();

  Pieces pieces;
  TableIndAndBufferInd indexToTableIndex(int index) const;
  int tableIndexToIndex(TableIndAndBufferInd tab) const;
private:
  handleData addBuffer, origBuffer;
};

typedef std::shared_ptr<PieceTable> handleTable;

struct PieceIterator
{
  PieceIterator() = default;
  PieceIterator(int a, int b, handleTable p) : TableInd(a), BufferInd(b), Table(p) {};
  int TableInd = 0;
  int BufferInd = 0;
  handleTable Table = handleTable();

  std::string value() const;

  PieceIterator &operator++();
  PieceIterator operator++(int) {
    PieceIterator temp(*this);
    this->operator++();
    return temp;
  };
};

bool operator==(PieceIterator lhs, PieceIterator rhs);

bool operator!=(PieceIterator lhs, PieceIterator rhs);

PieceIterator operator+(PieceIterator lhs, int rhs);

// class PieceBuffer
// {
// public:
//   PieceBuffer() = default;
//   ~PieceBuffer();

//   std::string SubString(int index, int len);
//   void insert(int index, std::string str);
//   void remove(int index, int len);

// private:
//   std::vector<PieceData> buffers;
//   PieceTable table;
// };
