#pragma once

#include "property_tree.hpp"
#include "error.hpp"

namespace base
{

template<typename R>
R PropertyTree::get(const std::string& path) const
{
    try {
        return _ptree.get<R>(std::string{ path });
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument,
                    std::string{ "Cannot get given value from configuration. Additional info = " } + e.what());
    }
}


template<typename R>
std::vector<R> PropertyTree::getVector(const std::string& path) const
{
    try {
        std::vector<R> ret;
        for (const auto& item : _ptree.get_child(path)) {
            ret.push_back(item.second.get_value<R>());
        }
        return ret;
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument,
                    std::string{ "Cannot get given value from configuration. Additional info = " } + e.what());
    }
}

} // namespace base
