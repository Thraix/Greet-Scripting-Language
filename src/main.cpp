#include "Token.h"

#include "Lexer.h"

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
  if(argc < 2)
    std::cout << "No input file" << std::endl;
  std::ifstream source(argv[1]);
  std::cout << "Lexing: " << argv[1] << std::endl;
  std::vector<Token> tokens = Lexer::Read(source);
  for(auto token : tokens)
  {
    std::cout << Tokens::tokenName.find(token)->second << " ";
  }
  std::cout << std::endl;

  return 0;
}
