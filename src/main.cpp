#include "Token.h"

#include "Lexer.h"
#include "Parser.h"

#include <iostream>
#include <cstring>
#include <fstream>


int main(int argc, char** argv)
{
  if(argc < 2)
    std::cout << "No input file" << std::endl;
  std::ifstream source(argv[1]);
  std::cout << "Compiling: " << argv[1] << std::endl;
  std::vector<TokenPos> tokens = Lexer::Read(source);
  if(argc >= 3)
  {
    if(strcmp(argv[2], "-t") == 0)
    {
      int i = 0;
      for(auto token : tokens)
      {
        std::cout << i << ": " << Tokens::GetName(token.token) << std::endl;
        i++;
      }
    }

    std::cout << std::endl;
  }

  if(Parser::Parse(tokens))
  {
    std::cout << "Succesfully Parsed file!" << std::endl;
  }

  return 0;
}
