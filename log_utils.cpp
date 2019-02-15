#include "log_utils.hpp"

namespace app_utils
{
  BlockIndent::BlockIndent(std::string const& message, bool incremIndent)
    : m_incremIndent(incremIndent)
  {
    if(!message.empty())
      AutoIndent::printIndent(std::cout) << message << std::endl;

    if (m_incremIndent)
      AutoIndent::increment();
  }

  BlockIndent::~BlockIndent() {
    if (m_incremIndent)
      AutoIndent::decrement();
  }

  string parsePrettyFunction(string nameStr)
  {
    auto argStart = nameStr.find('(');
    auto strippos = nameStr.rfind(' ', nameStr.rfind("::", argStart));
    if (strippos != string::npos)
    {
      // case when there is a return type
      strippos += 1; // skip white space
      nameStr = nameStr.substr(strippos, argStart - strippos);
    }else{
      // no return type (e.g. Constructor::Constructor)
      nameStr = nameStr.substr(0, argStart);
    }
    return nameStr;
  }
}