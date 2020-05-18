#include "Token.h"

#include "Lexer.h"
#include "Parser.h"

#include <iostream>
#include <fstream>


int main(int argc, char** argv)
{
  if(argc < 2)
    std::cout << "No input file" << std::endl;
  std::ifstream source(argv[1]);
  std::cout << "Compiling: " << argv[1] << std::endl;
  std::vector<Token> tokens = Lexer::Read(source);
  int i = 0;
  for(auto token : tokens)
  {
    std::cout << i << ": " << Tokens::GetName(token) << std::endl;
    i++;
  }

  /* int j = int k = 9; */
  std::cout << std::endl;

  Parser::Parse(tokens);

  return 0;
}
