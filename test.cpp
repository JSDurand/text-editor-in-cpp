#include "piece.h"
#include <iostream>


int main(int argc, char **argv)
{
  handleData origB(new PieceData("hi", RO)); 
  PieceTable *table = new PieceTable(origB);

  std::cout << "first:\n" << table->to_string() << std::endl;
  
  table->insert(1, "wowow ");
  
  std::cout << "second:\n" << table->to_string() << std::endl;
  
  table->insert(2, "very cool!");

  std::cout << "third:\n" << table->to_string() << std::endl;
  
  table->remove(6, 1);

  std::cout << "fourth:\n" << table->to_string() << std::endl;

  table->insert(4, "new");

  std::cout << "new:\n" << table->to_string() << std::endl;
  
  PieceIterator it(0, 0, std::make_shared<PieceTable>(*table));

  it++;
  it++;
  it++;
  it++;

  std::cout << it.value() << std::endl;

  return 0;
}
