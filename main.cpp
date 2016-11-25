// Includes
#include "ShuntingYard.h"
#include <iostream>
#include <map>
#include <string>
#include <deque>

int main(int argc, char** argv)
{
  if(argc != 2) {
    std::cout << "Usage: ./program <formula in infix notation>" << std::endl;
    std::cout << "Example: ./program \"sin ( 3 + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3 )\"" << std::endl;
    return 1;
  }
  
  ShuntingYard* sy = new ShuntingYard();
  std::string infix(argv[1]);
  std::deque<Token> postfix = sy->getPostfix(infix);
  sy->printPostfix(postfix);
  std::map<std::string, double> definitions = {{"x", 1}};
  std::cout << "Result: " << sy->evaluate(infix, definitions) << std::endl;

  delete sy;

  return 0;
}
