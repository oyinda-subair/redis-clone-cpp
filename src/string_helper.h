#include <string>
#include <sstream>
#include <vector>

// using namespace std;

std::vector<std::string> token(const std::string &input, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(input);
  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
}

std::vector<std::string> split(const std::string &input)
{
  std::vector<std::string> tokens;
  std::stringstream ss(input);
  std::string token;

  while (ss >> token)
  {
    tokens.push_back(token);
  }

  return tokens;
}

std::string error_response(const std::string &message)
{
  return "-ERR wrong number of arguments for " + message + "\r\n";
}
