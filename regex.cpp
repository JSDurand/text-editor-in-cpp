#include "regex.h"
#include <iostream>
#include <algorithm> // std::reverse
#include <stdexcept>
// #define DEBUG


void print_map(const mint m)
{
   std::cout << "{\n";
   for(auto p : m)
     std::cout << p.first << ':' << p.second << ',' << std::endl;
   std::cout << "}\n";
}

std::string Inst::to_string() const
{
  std::string result("");

  std::string code("");

  switch (this->op) {
  case ANY:
    code = "ANY";
    break;
  case CHAR:
    code = "CHAR";
    break;
  case MATCH:
    code = "MATCH";
    break;
  case SPLIT:
    code = "SPLIT";
    break;
  case JUMP:
    code = "JUMP";
    break;
  case OP_LEFT:
    code = "LEFT";
    break;
  case OP_RIGHT:
    code = "RIGHT";
    break;
  }
  
  result.append("CODE: " + code + ", ");
  result.append("CHAR: " + std::string(1, this->c) + ", ");
  result.append("x: " + std::to_string(this->x) + ", ");
  result.append("y: " + std::to_string(this->y));

  return result;
}

int find_open_bracket(vInst ins, int start, int end)
{
  if (end >= ins.size())
    end = ins.size() - 1;
  if (start < 0)
    start = 0;

  int count(0);
  
  for (int i = end; i >= start; --i) {
    if (ins[i].op == OP_RIGHT)
      ++count;
    if (ins[i].op == OP_LEFT)
      --count;

    if (count == -1)
      return i + 1;
  }

  return start;
}

int find_close_bracket(std::string str, int start, int end)
{
  if (end >= str.size())
    end = str.size() - 1;
  if (start < 0)
    start = 0;

  int count(0);

  for (int i = start; i <= end; ++i) {
    if (str[i] == '(')
      ++count;
    if (str[i] == ')')
      --count;

    if (count == -1)
      return i - 1;
  }

  return end;
}

void append_regex(vInst &e1, vInst e2)
{
  int offset = e1.size();
  for (auto it = e2.begin(); it != e2.end(); ++it) {
    it->x += offset;
    it->y += offset;
  }
  
  e1.insert(e1.end(), e2.begin(), e2.end());
}

int get_last_term (vInst e)
{
  if (e.back().op == OP_RIGHT) {
    int count(0);
    for (int i = e.size() - 1; i >= 0; --i) {
      if (e[i].op == OP_RIGHT)
        ++count;
      else if (e[i].op == OP_LEFT)
        --count;

      if (count == 0)
        return i + 1;
    }
  }

  return e.size() - 1;
}

void shift_regex (vInst &e, int begin, int end, int offset)
{
  for (auto it = e.begin() + begin; it != e.begin() + end; ++it) {
    it->x += offset;
    it->y += offset;
  }
}

bool determine_greedy (std::string regex, int index)
{
  int len = regex.size();

  if (index >= len - 1) { // no more char to make it non-greedy
    return true;
  }

  char current = *(regex.c_str() + index);

  char next = *(regex.c_str() + index + 1);

  std::vector<char> special = { '*', '?', '+' }; // for sanity check

  bool non_greedy = (std::find(special.begin(), special.end(), current) != special.end()) &&
    next == '?';

  return !non_greedy;
}

vInst compile_regex (std::string regex)
{
  vInst result{Inst(SPLIT, '0', 3, 1), Inst(ANY), Inst(JUMP, '0', 0), Inst(OP_LEFT)};
  int L1(0), L2(0), L3(0);
  Inst i1;
  vInst e1, e2;

  for (int i = 0; i < regex.length(); ++i) {
    char c = regex[i];
    switch (c) {
    case '.':
      result.push_back(Inst(ANY));
      break;
    case '(':
      i1 = Inst(OP_LEFT);
      result.push_back(i1);
      break;
    case ')':
      i1 = Inst(OP_RIGHT);
      result.push_back(i1);
      break;
    case '+':
      L1 = get_last_term(result);
      if (result.back().op == OP_RIGHT) {
        L2 = result.size();
        if (determine_greedy(regex, i))
          result.insert(result.end() - 1, Inst(SPLIT, '0', L1, L2));
        else {
          result.insert(result.end() - 1, Inst(SPLIT, '0', L2, L1));
          ++i;
        }
      } else {
        L2 = result.size() + 1;
        if (determine_greedy(regex, i))
          result.push_back(Inst(SPLIT, '0', L1, L2));
        else {
          ++i;
          result.push_back(Inst(SPLIT, '0', L2, L1));
        }
      }
      break;
    case '?':
      L1 = get_last_term(result);
      L2 = result.size() + 1;
      if (result.back().op == OP_RIGHT)
        --L1;
      shift_regex(result, L1, result.size(), 1);
      if (determine_greedy(regex, i))
        result.insert(result.begin() + L1, Inst(SPLIT, '0', L1 + 1, L2));
      else {
        result.insert(result.begin() + L1, Inst(SPLIT, '0', L2, L1 + 1));
        ++i;
      }
      break;
    case '*':
      L1 = get_last_term(result);
      L2 = result.size() + 2;
      if (result.back().op == OP_RIGHT)
        --L1;
      shift_regex(result, L1, result.size(), 1);
      if (determine_greedy(regex, i))
        result.insert(result.begin() + L1, Inst(SPLIT, '0', L1 + 1, L2));
      else {
        result.insert(result.begin() + L1, Inst(SPLIT, '0', L2, L1 + 1));
        ++i;
      }
      result.push_back(Inst(JUMP, '0', L1));
      break;
    case '|':
      L1 = find_open_bracket(result, 0, result.size());
      e1 = compile_regex(regex.substr(i + 1, find_close_bracket(regex, i + 1, regex.size() - 1) - i));
      // get rid of extra stuff
      e1.pop_back();
      e1.pop_back();
      e1.pop_front();
      e1.pop_front();
      e1.pop_front();
      e1.pop_front();
      shift_regex(e1, 0, e1.size(), -4);
      L2 = e1.size();
      L3 = result.size() + 2;
      i1 = Inst(SPLIT, '0', L1 + 1, L3);
      shift_regex(result, L1, result.size(), 1);
      result.insert(result.begin() + L1, i1);
      L1 = result.size() + 1 + L2;
      result.push_back(Inst(JUMP, '0', L1)); // jump over e2
      append_regex(result, e1);
      i = find_close_bracket(regex, i + 1, regex.size() - 1);
      break;
    default:
      result.push_back(Inst(CHAR, c));
      break;
    }
  }

  result.push_back(Inst(OP_RIGHT));
  result.push_back(Inst(MATCH));

  return result;
}

void add_thread(vThread &list, int pc, mint saved, const vInst prog, const vpint parens, const int input_index)
{
  for (vThread::iterator it = list.begin(); it != list.end(); ++it) {
    if (it->pc == pc)
      return;
  }

  Inst in = prog.at(pc);

  int jump_index = 0;

  switch (in.op) {
  case JUMP:
    add_thread(list, in.x, saved, prog, parens, input_index);
    break;
  case SPLIT:
    add_thread(list, in.x, saved, prog, parens, input_index);
    add_thread(list, in.y, saved, prog, parens, input_index);
    break;
  case OP_LEFT:
    jump_index = find_left_paren(parens, pc);
    if (saved.find(2 * jump_index) == saved.end() ||
        (saved.find(2 * jump_index + 1) != saved.end() &&
         input_index > saved[2 * jump_index + 1])) {
      saved[2 * jump_index] = input_index;
    }
    add_thread(list, pc + 1, saved, prog, parens, input_index);
    break;
  case OP_RIGHT:
    jump_index = find_right_paren(parens, pc);
    saved[2 * jump_index + 1] = input_index;
    add_thread(list, pc + 1, saved, prog, parens, input_index);
    break;
  default:
    list.push_back(Thread(pc, saved));
    break;
  }
}

int regex_vm (vInst prog, std::string input, mint &match_data)
{
  vThread clist, nlist;

  int pc, jump_index, input_index(0);

  Inst inst;

  vpint parens;

  mint save_loc;

  bool matched(false);

  parse_parens(prog, parens);

  add_thread(clist, 0, mint(), prog, parens, 0); // start from 0

  for (std::string::iterator it = input.begin(); it != input.end(); ++it, ++input_index) {
#ifdef DEBUG
      std::cout << "current char: " + std::string(1, *it) << std::endl;
#endif
    for (int i = 0; i < clist.size(); ++i) {
      pc       = clist[i].pc;
      inst     = prog.at(pc);
      save_loc = clist[i].saved;

#ifdef DEBUG
      std::cout << "current pc: " + std::to_string(pc) << std::endl;
#endif

      switch (inst.op) {
      case ANY:
#ifdef DEBUG
        std::cout << "pc " + std::to_string(pc) + " matched." << std::endl;
#endif
        add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
        break;
      case CHAR:
        if (*it != inst.c) {
#ifdef DEBUG
          std::cout << "pc " + std::to_string(pc) + " died." << std::endl;
#endif
          break;
        }
        add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
#ifdef DEBUG
        std::cout << "pc " + std::to_string(pc) + " matched." << std::endl;
#endif
        break;
      case MATCH:
        match_data = save_loc;
        clist.clear();
        matched = true;
        break;
      case JUMP:
        add_thread(clist, inst.x, save_loc, prog, parens, input_index);
        break;
      case SPLIT:
        add_thread(clist, inst.x, save_loc, prog, parens, input_index);
        add_thread(clist, inst.y, save_loc, prog, parens, input_index);
        break;
      case OP_LEFT:
        jump_index = find_left_paren(parens, pc);
        if (save_loc.find(2 * jump_index) == save_loc.end() ||
            (save_loc.find(2 * jump_index + 1) != save_loc.end() &&
             input_index > save_loc[2 * jump_index + 1])) {
          save_loc[2 * jump_index] = input_index;
        }
        add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
        break;
      case OP_RIGHT:
        jump_index = find_right_paren(parens, pc);
        save_loc[2 * jump_index + 1] = input_index;
        add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
        break;
      default:
        break;
      }
    }

    std::swap(clist, nlist);
    nlist.clear();
  }

  for (vThread::iterator it = clist.begin(); it != clist.end(); ++it) {
    if (prog[it->pc].op == MATCH) {
      match_data = it->saved;
      clist.clear();
      matched = true;
      break;
    }
  }

  if (matched)
    return 1;

  return 0;
}

void parse_parens(const vInst list, vpint &vp)
{
  std::vector<int> left_list, right_list;

  for (int i = list.size() - 1; i >= 0; --i) {
    OPCODE op = list.at(i).op;
    if (op == OP_RIGHT) {
      right_list.push_back(i);
    } else if (op == OP_LEFT) {
      if (right_list.size() == 0) {
        std::cerr << "Unbalanced parens!" << std::endl;
        return;
      }

      vp.push_back(std::make_pair(i, right_list.back()));
      right_list.pop_back();
    }
  }

  std::reverse(vp.begin(), vp.end());
}

int find_left_paren(const vpint vp, const int pc)
{
  for (int i = 0; i < vp.size(); ++i) {
    if (std::get<0>(vp.at(i)) == pc)
      return i;
  }

  return -1;
}

int find_right_paren(const vpint vp, const int pc)
{
  for (int i = 0; i < vp.size(); ++i) {
    if (std::get<1>(vp.at(i)) == pc)
      return i;
  }

  return -1;
}

int regex_vm_piece_table (vInst prog, PieceTable table, mint &match_data,
                          int start_index, bool forward)
{
  if (start_index < 0 || table.length() <= start_index)
    std::cerr << "Index out of bounds" << std::endl;
  
  vThread clist, nlist;

  int pc, jump_index, input_index(0);

  Inst inst;

  vpint parens;

  mint save_loc;

  bool matched(false);

  parse_parens(prog, parens);

  add_thread(clist, 0, mint(), prog, parens, 0); // start from 0

  if (forward) {
    for (PieceIterator it = table.begin() + start_index; it != table.end() + 1; ++it, ++input_index) {
#ifdef DEBUG
      std::cout << "current char: " + it.value() << std::endl;
#endif
      for (int i = 0; i < clist.size(); ++i) {
        pc       = clist[i].pc;
        inst     = prog.at(pc);
        save_loc = clist[i].saved;

#ifdef DEBUG
        std::cout << "current pc: " + std::to_string(pc) << std::endl;
#endif

        switch (inst.op) {
        case ANY:
          add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
          break;
        case CHAR:
          if (it.value()[0] != inst.c) {
#ifdef DEBUG
            std::cout << "pc " + std::to_string(pc) + " died." << std::endl;
#endif
            break;
          }
          add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
          break;
        case MATCH:
          match_data = save_loc;
          clist.clear();
          matched = true;
          break;
        case JUMP:
          add_thread(clist, inst.x, save_loc, prog, parens, input_index);
          break;
        case SPLIT:
          add_thread(clist, inst.x, save_loc, prog, parens, input_index);
          add_thread(clist, inst.y, save_loc, prog, parens, input_index);
          break;
        case OP_LEFT:
          jump_index = find_left_paren(parens, pc);
          if (save_loc.find(2 * jump_index) == save_loc.end() ||
              (save_loc.find(2 * jump_index + 1) != save_loc.end() &&
               input_index > save_loc[2 * jump_index + 1])) {
            save_loc[2 * jump_index] = input_index;
          }
          add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
          break;
        case OP_RIGHT:
          jump_index = find_right_paren(parens, pc);
          save_loc[2 * jump_index + 1] = input_index;
          add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
          break;
        default:
          break;
        }
      }

      std::swap(clist, nlist);
      nlist.clear();
    }
  } else {
    for (PieceIterator it = table.begin() + start_index; it != table.begin() + -1;
         it = it + -1, ++input_index) {
#ifdef DEBUG
      std::cout << "current char: " + it.value() << std::endl;
#endif
      for (int i = 0; i < clist.size(); ++i) {
        pc       = clist[i].pc;
        inst     = prog.at(pc);
        save_loc = clist[i].saved;

#ifdef DEBUG
        std::cout << "current pc: " + std::to_string(pc) << std::endl;
#endif

        switch (inst.op) {
        case ANY:
          add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
          break;
        case CHAR:
          if (it.value()[0] != inst.c) {
#ifdef DEBUG
            std::cout << "pc " + std::to_string(pc) + " died." << std::endl;
#endif
            break;
          }
          add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
          break;
        case MATCH:
          match_data = save_loc;
          clist.clear();
          matched = true;
          break;
        case JUMP:
          add_thread(clist, inst.x, save_loc, prog, parens, input_index);
          break;
        case SPLIT:
          add_thread(clist, inst.x, save_loc, prog, parens, input_index);
          add_thread(clist, inst.y, save_loc, prog, parens, input_index);
          break;
        case OP_LEFT:
          jump_index = find_left_paren(parens, pc);
          if (save_loc.find(2 * jump_index) == save_loc.end() ||
              (save_loc.find(2 * jump_index + 1) != save_loc.end() &&
               input_index > save_loc[2 * jump_index + 1])) {
            save_loc[2 * jump_index] = input_index;
          }
          add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
          break;
        case OP_RIGHT:
          jump_index = find_right_paren(parens, pc);
          save_loc[2 * jump_index + 1] = input_index;
          add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
          break;
        default:
          break;
        }
      }

      std::swap(clist, nlist);
      nlist.clear();
    }
  }

  for (vThread::iterator it = clist.begin(); it != clist.end(); ++it) {
    if (prog[it->pc].op == MATCH) {
      match_data = it->saved;
      clist.clear();
      matched = true;
      break;
    }
  }

  if (matched)
    return 1;

  return 0;
}

int regex_vm_piece_table (vInst prog, PieceTable table, mint &match_data,
                          int start_index, int end_index, int times,
                          bool forward)

{
  if (start_index < 0) {
    start_index = 0;
  } else if (table.length() <= start_index) {
    start_index = table.length() - 1;
  }

  if (end_index == -1)
    end_index = forward ? table.length() - 1 : 0;
  
  if (end_index < 0) {
    end_index = 0;
  } else if (table.length() <= end_index) {
    end_index = table.length() - 1;
  }

  if (start_index > end_index && forward)
    end_index = start_index;
  if (start_index < end_index && !forward)
    end_index = start_index;
  
  vThread clist, nlist;

  int pc, jump_index, input_index(start_index), old_start(start_index);

  Inst inst;

  vpint parens;

  mint save_loc;

  bool matched(false);

  parse_parens(prog, parens);

  add_thread(clist, 0, mint(), prog, parens, start_index);

  for (int t = 0; t < times; ++t) {
#ifdef DEBUG
    std::cout << std::to_string(start_index) << std::endl;
#endif
    if (forward) {
      for (PieceIterator it = table.begin() + start_index;
           input_index <= end_index + 1; ++it, ++input_index) {
#ifdef DEBUG
        std::cout << "current char: " + it.value() << std::endl;
        std::cout << "size: " << clist.size() << std::endl;
#endif

        if (clist.size() == 0) break;

        for (int i = 0; i < clist.size(); ++i) {
          pc       = clist[i].pc;
          inst     = prog.at(pc);
          save_loc = clist[i].saved;

#ifdef DEBUG
          std::cout << "current pc: " + std::to_string(pc) << std::endl;
#endif

          switch (inst.op) {
          case ANY:
            add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
            break;
          case CHAR:
            if (it.value()[0] != inst.c) {
#ifdef DEBUG
              std::cout << "pc " + std::to_string(pc) + " died." << std::endl;
#endif
              break;
            }
            add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
            break;
          case MATCH:
            match_data = save_loc;
            clist.clear();
            matched = true;
            break;
          case JUMP:
            add_thread(clist, inst.x, save_loc, prog, parens, input_index);
            break;
          case SPLIT:
            add_thread(clist, inst.x, save_loc, prog, parens, input_index);
            add_thread(clist, inst.y, save_loc, prog, parens, input_index);
            break;
          case OP_LEFT:
            jump_index = find_left_paren(parens, pc);
            if (save_loc.find(2 * jump_index) == save_loc.end() ||
                (save_loc.find(2 * jump_index + 1) != save_loc.end() &&
                 input_index > save_loc[2 * jump_index + 1])) {
              save_loc[2 * jump_index] = input_index;
            }
            add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
            break;
          case OP_RIGHT:
            jump_index = find_right_paren(parens, pc);
            save_loc[2 * jump_index + 1] = input_index;
            add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
            break;
          default:
            break;
          }
        }

        std::swap(clist, nlist);
        nlist.clear();
      }
    } else {
      for (PieceIterator it = table.begin() + start_index;
           it != table.begin() + (end_index - 1);
           it = it + -1, ++input_index) {
#ifdef DEBUG
        std::cout << "current char: " + it.value() << std::endl;
        std::cout << "size: " << clist.size() << std::endl;
#endif
        
        if (clist.size() == 0) break;

        for (int i = 0; i < clist.size(); ++i) {
          pc       = clist[i].pc;
          inst     = prog.at(pc);
          save_loc = clist[i].saved;

#ifdef DEBUG
          std::cout << "current pc: " + std::to_string(pc) << std::endl;
#endif

          switch (inst.op) {
          case ANY:
            add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
            break;
          case CHAR:
            if (it.value()[0] != inst.c) {
#ifdef DEBUG
              std::cout << "pc " + std::to_string(pc) + " died." << std::endl;
#endif
              break;
            }
            add_thread(nlist, pc + 1, save_loc, prog, parens, input_index + 1);
            break;
          case MATCH:
            match_data = save_loc;
            clist.clear();
            matched = true;
            break;
          case JUMP:
            add_thread(clist, inst.x, save_loc, prog, parens, input_index);
            break;
          case SPLIT:
            add_thread(clist, inst.x, save_loc, prog, parens, input_index);
            add_thread(clist, inst.y, save_loc, prog, parens, input_index);
            break;
          case OP_LEFT:
            jump_index = find_left_paren(parens, pc);
            if (save_loc.find(2 * jump_index) == save_loc.end() ||
                (save_loc.find(2 * jump_index + 1) != save_loc.end() &&
                 input_index > save_loc[2 * jump_index + 1])) {
              save_loc[2 * jump_index] = input_index;
            }
            add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
            break;
          case OP_RIGHT:
            jump_index = find_right_paren(parens, pc);
            save_loc[2 * jump_index + 1] = input_index;
            add_thread(clist, pc + 1, save_loc, prog, parens, input_index);
            break;
          default:
            break;
          }
        }

        std::swap(clist, nlist);
        nlist.clear();
      }
    }

    for (vThread::iterator it = clist.begin(); it != clist.end(); ++it) {
      if (prog[it->pc].op == MATCH) {
        match_data = it->saved;
        clist.clear();
        matched = true;
        break;
      }
    }

    if (t < times - 1) {
#ifdef DEBUG
      print_map(match_data);
      std::string sub = table.SubString(match_data[0], match_data[1] - match_data[0]);
      std::cout << (table.begin() + match_data[0]).value() << std::endl;
      std::cout << (table.begin() + match_data[1]).value() << std::endl;
      std::cout << sub << std::endl;
#endif
      if (matched) {
        if (forward)
          start_index = match_data[1];
        else
          start_index += -match_data[1];
        input_index = start_index;
        matched     = false;
        match_data  = mint();
        clist.clear();
        nlist.clear();
        add_thread(clist, 0, mint(), prog, parens, start_index);
      } else {
        break;
      }
    }
  }

  for (int i = 0; i < match_data.size(); ++i)
    match_data[i] -= old_start;

  if (matched)
    return 1;
 
  return 0;
}

// int main (int argc, char **argv)
// {

//   vInst reg = compile_regex("c(a|g)+(eb.k+|ec.f+)*e");
//   // vInst reg = compile_regex("b*c*b");
//   int i = 0;
//   mint match_data;
//   std::string str = "caggaebhkkecgfffebjkkegvgvbbn";
//   // std::string str = "bcccb";

//   for (auto it = reg.begin(); it != reg.end(); ++it, ++i)
//     std::cout << "INDEX: " << std::to_string(i) << " " << it->to_string() << std::endl;

//   std::cout << std::to_string(regex_vm(reg, str, match_data)) << std::endl;

//   print_map(match_data);

//   for (i = 0; i < match_data.size() / 2; ++i) {
//     std::string sub = str.substr(match_data[2 * i], match_data[2 * i + 1] - match_data[2 * i]);
//     std::cout << "The " + std::to_string(i) << "-th match: "
//               << sub << std::endl;
//   }
  
//   return 0;
// }

#ifdef DEBUG
int main (int argc, char **argv)
{

  // vInst reg = compile_regex("c(a|g)+(eb.k+|e(c.f+))*e");
  // vInst reg = compile_regex("\n");
  vInst reg = vInst{Inst(SPLIT, '0', 3, 1), Inst(ANY), Inst(JUMP, '0', 0),
                    Inst(OP_LEFT), Inst(CHAR, '\n'), Inst(OP_RIGHT), Inst(MATCH)};
  // vInst reg = compile_regex("b*c*b");
  int i = 0;
  mint match_data{};
  // std::string str = "gvgzvcaggaebhkkehhcaggaebhkketcgfffechfffebjkkegvgvbbn";
  std::string str = "qhbdhbs\njskdn\nrqsdsbd\nhjdksnfkdsf\nqhbdhbs\njskdn\n qsdsbd\n jdksnfkdsf\nqhbdhbs\njskdn\n qsdsbd\n jdksnfkdsf\n";
  // std::string str = "bcccb";

  for (auto it = reg.begin(); it != reg.end(); ++it, ++i)
    std::cout << "INDEX: " << std::to_string(i) << " " << it->to_string() << std::endl;

  handleData orig = std::make_shared<PieceData>(PieceData(str, RO));
  PieceTable table = PieceTable(orig);

  std::cout << std::to_string(regex_vm_piece_table(reg, table, match_data, 15, 0, 1, false)) << std::endl;
  // std::cout << table.SubString(4, 11) << std::endl;
  // std::cout << std::to_string(regex_vm(reg, str, match_data)) << std::endl;

  print_map(match_data);

  for (i = 0; i < match_data.size() / 2; ++i) {
    std::string sub = str.substr(-match_data[2 * i] + 15, match_data[2 * i + 1] - match_data[2 * i]);
    std::cout << "The " + std::to_string(i) << "-th match: "
              << sub << std::endl;
  }
  
  return 0;
}
#endif
