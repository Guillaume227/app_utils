#include "log_utils.hpp"

namespace app_utils
{
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