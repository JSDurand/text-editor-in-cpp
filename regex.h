#pragma once

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <utility> // for pair
#include "piece.h"

enum OPCODE
  {
   CHAR,
   MATCH,
   JUMP,
   SPLIT,
   OP_LEFT,
   OP_RIGHT,
   ANY
  };

struct Inst
{
  Inst(OPCODE o, char ch = '0', int xx = 0, int yy = 0)
    : op(o), c(ch), x(xx), y(yy) {};
  Inst() = default;
  OPCODE op;
  // for matching
  char c;
  int x = 0, y = 0;

  std::string to_string() const;
};

typedef std::deque<Inst> vInst;
typedef std::map<int, int> mint;
typedef std::vector<std::pair<int, int> > vpint;

struct Thread
{
  Thread() = default;
  Thread(int x) : pc(x) {};
  Thread(int x, mint save) : pc(x), saved(save) {};
  int pc;
  mint saved = mint();
};

typedef std::deque<Thread> vThread;

int regex_vm (vInst prog, std::string input, mint &match_data);

vInst compile_regex (std::string regex);

void shift_regex (vInst &e, int begin, int end, int offset);

int get_last_term (vInst e);

void append_regex(vInst &e1, vInst e2);

int find_close_bracket(std::string str, int start, int end);

int find_open_bracket(vInst ins, int start, int end);

void add_thread(vThread &list, int pc, mint saved, const vInst prog, const vpint parens, const int input_index);

void parse_parens(const vInst list, vpint &vp);

int find_left_paren(const vpint vp, const int pc);

int find_right_paren(const vpint vp, const int pc);

void print_map(const mint m);

int regex_vm_piece_table (vInst prog, PieceTable table, mint &match_data,
                          int start_index, bool forward = true);

int regex_vm_piece_table (vInst prog, PieceTable table, mint &match_data,
                          int start_index, int end_index, int times,
                          bool forward = true);
