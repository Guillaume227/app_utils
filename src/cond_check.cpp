#include <app_utils/cond_check.hpp>
#include <app_utils/string_utils.hpp>

#include <sstream>
#include <cstring> // strlen
#include <string>
#include <vector>

#if defined(__GNUG__) && !defined(__clang__)
#include <execinfo.h>
#include <cxxabi.h>
#define SUPPORTS_BACKTRACE true

#else
#define SUPPORTS_BACKTRACE false
//TODO
//#define NOMINMAX
//#include <Windows.h>
//#include <DbgHelp.h>
#endif


namespace app_utils {
  std::string Exception::formatStackInfo(char const* file, size_t line, char const* functionName) {
    std::ostringstream os;
    std::vector<std::string> backTrace; // TODO:  = getBackTrace(50);

    for (size_t i = 0; i < backTrace.size(); i++) {
      if (app_utils::strutils::contains(backTrace[i], __FUNCTION__)) {
        continue; // skip enclosing method
      }
      // skip the last entry in the backTrace which comes from the 
      // current function that is not adding any value to the error message
      os << "  " << backTrace[i] << "\n";
    }
    os << "\nthrown from " << file << ":" << line << " " << functionName << "():";
    os << "\nERROR -";

    return os.str();
  }


  std::string combineTraces(std::string str1, std::string const& str2) {
    size_t const max_prefix_size = std::min(str1.size(), str2.size());
size_t prefix_size = 0;
for (; prefix_size < max_prefix_size; prefix_size++) {
  if (str1[prefix_size] != str2[prefix_size]) {
    break;
  }
}
str1 += "\n";
str1 += "  Root cause:\n";
str1 += str2.substr(prefix_size);
return str1;
  }

  namespace {
    bool isPrefix(char const* pre, char const* str) {
      return strncmp(pre, str, strlen(pre)) == 0;
    }

    void cleanFunctionSignature(std::string& function_signature) noexcept {

      try {
        char const token[] = ", std::allocator<";
        size_t tokenSize = sizeof(token) - 1;
        while (true) {
          size_t tokenStart = function_signature.rfind(token);
          if (tokenStart == std::string::npos) {
            break;
          }
          auto closePos = strutils::findMatchingClose(function_signature, tokenStart + tokenSize - 1);
          if (closePos == std::string::npos) {
            return;
            //throw "invalid function signature;";
          }
          size_t eraseSpan = closePos - tokenStart + 1;
          function_signature.erase(tokenStart, eraseSpan);

        }
        strutils::replaceAll(function_signature, "std::", "");
      }
      catch (std::exception const& exc) {
        std::cout << exc.what();

      }
    }
  } // namespace

  std::vector<std::string> getBackTrace(unsigned int const max_stack_depth) {

    auto exclude_line = [](std::string const& line) {
      if (line.empty()) {
        return true;
      }
      static std::vector<std::string> const exclude_prefixes{ "__libc_start_main",
        "main",
      "RUN_ALL_TESTS()",
      "testing::",
      "bool testing::internal",
      "void testing::internal",
        // from python bindings:
        "_Py",
        "PyRun_",
        "PyObject_",
        "PyEval" };

      for (auto const& prefix : exclude_prefixes) {
        if (app_utils::strutils::startswith(line, prefix)) {
          return true;
        }
      }
      return false;
    };

    std::vector<void*> array(max_stack_depth);

    #if SUPPORTS_BACKTRACE
    int size = backtrace(&*std::begin(array), max_stack_depth);
    #else
    int size = 0;
    #endif
    int skippedStackLines = 0;

    std::vector<std::string> stackTrace;
    stackTrace.reserve(max_stack_depth);

    if (char** messages = 
    #if SUPPORTS_BACKTRACE
            backtrace_symbols(&*std::begin(array), size)
      #else
      nullptr
      #endif
      ) {

      bool foundPythonStackFrames = false;

      /* skip first stack frame (points here) */
      for (int i = size - 1; i > 0; i--) {
        if (isPrefix("python(", messages[i])) {
          if (not foundPythonStackFrames) {
            stackTrace.emplace_back("[skipping python stack]");
          }
          foundPythonStackFrames = true;
          continue;
        }

        char* mangled_name = nullptr;
        char* offset_begin = nullptr;
        char* offset_end = nullptr;

        // find parentheses and +address offset surrounding mangled name
        for (char* p = messages[i]; *p; ++p) {
          if (*p == '(') {
            mangled_name = p;
          }
          else if(*p == '+') {
            offset_begin = p;
          }
          else if (*p == ')') {
            offset_end = p;
            break;
          }
        }

        // if the line could be processed, attempt to demangle the symbol
        if (mangled_name and offset_begin and offset_end and mangled_name < offset_begin) {
          *mangled_name++ = '\0';
          *offset_begin++ = '\0';
          *offset_end++ = '\0';

          
          std::string function_signature;
#if defined(__GNUG__) && !defined(__clang__)
          int status;
          char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
          // if demangling is successful, output the demangled function name
          function_signature = status == 0 ? real_name : mangled_name;
          free(real_name);
#else
          function_signature = mangled_name;
          //TODO:
          //function_signature.reserve(1024);
          //__unDName(function_signature, mangled_name + 1, function_signature.size(), malloc, free, 0x2800);
#endif
          if (exclude_line(function_signature)) {
            skippedStackLines++;
            continue;
          }
          cleanFunctionSignature(function_signature);
          stackTrace.push_back(std::move(function_signature));
        } else { // otherwise, print the whole line
          stackTrace.emplace_back(messages[i]);
        }
      }
    }
    return stackTrace;
  }

  [[noreturn]] void handleUncaughtException() noexcept {
    if (auto exptr = std::current_exception()) {
      // Redirect the error message to stderr:
      // 1. For better error display in python test framework in the event of a crash;
      // 2. For better view of error messages when running SanSim using slurm.

      try {
        std::rethrow_exception(exptr);
      }
      catch (app_utils::Exception const& exc) {
        std::cerr << "Uncaught exception:\n"
          << exc << std::endl
          << std::endl;
      }
      catch (std::exception const& exc) {
        std::cerr << "Uncaught exception:\n"
          << exc.what() << std::endl << std::endl;
      }
      catch (...) {
        std::fprintf(stderr, "Terminated due to unknown exception \n");
      }
    }
    else {
      std::fprintf(stderr, "Terminated due to unknownn reason :(\n");
    }

    std::exit(1);
  }
} // namespace app_utils