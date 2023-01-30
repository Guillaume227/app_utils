#pragma once


#include "reflexio_field_descriptor.hpp"
#include <istream>
#include <span>
#include <functional>

namespace reflexio::details {

std::string get_docstring(
        std::span<member_descriptor_t const* const> descriptors);

std::string differences(
        void const* instance1,
        void const* instance2,
        std::span<member_descriptor_t const* const> descriptors,
        std::function<bool(size_t)> const& should_skip);

std::ostream& to_yaml(
        void const* instance,
        std::span<member_descriptor_t const* const> descriptors,
        std::function<bool(size_t)> const& should_skip,
        std::ostream& os);

std::istream& from_yaml(
        void* instance,
        std::span<member_descriptor_t const* const> descriptors,
        std::istream& is,
        std::function<bool(size_t)> const& skip_function,
        int line_offset = 0);

}// namespace reflexio::details