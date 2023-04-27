#include <app_utils/reflexio_utils.hpp>
#include <cstring>
#include <sstream>
#include <unordered_map>

#include <app_utils/cond_check.hpp>
#include <app_utils/container_utils.hpp>

namespace reflexio::details {

std::string get_docstring(
        std::span<member_descriptor_t const* const> descriptors) {
  std::ostringstream oss;
  for (auto& descriptor: descriptors) {
    oss << descriptor->get_name() << ": "
        << descriptor->get_description() << "\n";
  }
  return oss.str();
}

std::string differences(
        void const* instance1,
        void const* instance2,
        std::span<member_descriptor_t const* const> descriptors,
        std::function<bool(size_t)> const& should_skip) {
  std::ostringstream out;
  size_t descriptor_counter = 0;
  for (auto& descriptor: descriptors) {
    size_t const descriptor_index = descriptor_counter++;
    if (should_skip and should_skip(descriptor_index)) {
      continue;
    }
    if (descriptor->values_differ(instance1, instance2)) {
      out << descriptor->get_name() << ": ";
      descriptor->value_to_yaml(instance1, out);
      out << " vs ";
      descriptor->value_to_yaml(instance2, out);
      out << '\n';
    }
  }
  return out.str();
}

std::ostream& to_yaml(
        void const* instance,
        std::span<member_descriptor_t const* const> descriptors,
        std::function<bool(size_t)> const& should_skip,
        std::ostream& os)
{
  yaml_utils::indenter_t indenter;
  bool const am_i_nested = yaml_utils::get_indent_depth() > 0;
  if (am_i_nested) {
    os << "\n";
  }

  size_t const last_index = descriptors.size() - 1;

  size_t field_index = 0;
  size_t descriptor_counter = 0;
  for (auto& descriptor: descriptors) {
    size_t const descriptor_index = descriptor_counter++;
    if (not should_skip(descriptor_index)) {
      yaml_utils::print_indent(os);
      os << descriptor->get_name() << ": ";
      descriptor->value_to_yaml(instance, os);
      if (field_index < last_index or not am_i_nested) {
        os << '\n';// avoid adding a newline
      }
      field_index++;
    }
  }
  return os;
}

std::istream& from_yaml(
        void* instance,
        std::span<member_descriptor_t const* const> descriptors,
        std::istream& is,
        std::function<bool(size_t)> const& should_skip,
        int const line_offset) {

  using descriptor_map_t = std::unordered_map<std::string_view,
                                              std::pair<size_t, member_descriptor_t const*>>;
  descriptor_map_t const descriptor_map =
          [&]{
            descriptor_map_t map;
            size_t descriptor_index = 0;
            for (auto& descriptor: descriptors) {
              map[descriptor->get_name()] = {descriptor_index++, descriptor};
            }
            return map;
          }();

  bool first_line_seen = false;
  size_t start_indent = 0;
  std::string raw_line;
  int line_num = line_offset;
  for(size_t start_of_line=is.tellg();
       std::getline(is, raw_line);
       start_of_line=is.tellg())
  {
    line_num++;
    std::string_view line = app_utils::strutils::strip(raw_line);
    size_t indent = raw_line.find_first_not_of(' ') / yaml_utils::indent_width;

    if (line.starts_with("#")) {
      // it's a comment line
      continue;
    } else if (line == "---") {
      if (first_line_seen) {
        // signals the start of another section
        break;
      } else {
        checkCond(indent == 0);
        start_indent = 0;
      }
      continue;
    } else if (line == "...") {
      // end of section
      break;
    } else if (not first_line_seen) {
      start_indent = indent;
    } else {
      if (start_indent > indent) {
        int rewind_offset = -static_cast<int>((size_t)is.tellg() - start_of_line);
        is.seekg(rewind_offset, std::ios_base::cur);
        break;
      }
    }

    first_line_seen = true;

    size_t separator_pos = line.find_first_of(':');

    if (separator_pos == std::string::npos) {
      if (line.find_first_not_of(" \t\n\r") == std::string::npos) {
        throwExc("unexpected empty line found at line", line_num);
      } else {
        throwExc("cannot parse a 'name: value' pair at line", line_num, ':', raw_line);
      }
    }

    auto const name = line.substr(0, separator_pos);
    auto const val = line.substr(separator_pos + 1);

    auto it = descriptor_map.find(name);

    checkCond(it != descriptor_map.end(), "unrecognized",
              "member name", name, "at line", line_num, ':', raw_line);

    if (should_skip(it->second.first)) {
      continue;
    }

    if (val.empty()) {
      // nested reflexio struct
      try {
        it->second.second->set_value_from_yaml(instance, is);
      } catch (std::exception const& exc) {
        throwExc("error found:", exc.what(),
                 "\nwhen parsing", it->second.second->get_name(), "at line", line_num);
      }
    } else {
      // value fits on just one line

      size_t sep_pos = raw_line.find_first_of(':');
      size_t rewind_offset;
      if (is.eof()) {
        is.clear();
        rewind_offset = raw_line.size() - sep_pos - 1;
      } else {
        size_t cur_pos = is.tellg();
        rewind_offset = cur_pos - start_of_line - sep_pos - 1;
      }
      is.seekg(-(int)rewind_offset, std::ios_base::cur);
      try {
        it->second.second->set_value_from_yaml(instance, is);
      } catch (std::exception const& exc) {
        throwExc("Failed parsing line", line_num, ":", raw_line, exc.what());
      }
    }
  }

  return is;
}

}// namespace reflexio::details