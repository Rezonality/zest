#pragma once

#include <map> 

#include <glm/glm.hpp>
#include <tomlplusplus/toml.hpp>

#include <zest/string/string_utils.h>

using namespace std::string_view_literals;

template<typename VecT, class T>
VecT toml_read_vec2(const T& node, const VecT& def = VecT(0.0f))
{
    const toml::array* pArray = node.as_array();
    if (!pArray)
    {
        return def; 
    }

    VecT ret;
    ret.x = (*pArray)[0].value_or(def.x);
    ret.y = (*pArray)[1].value_or(def.y);
    return ret;
}

template<typename VecT, class T>
VecT toml_read_vec3(const T& node, const VecT& def = VecT(0.0f))
{
    const toml::array* pArray = node.as_array();
    if (!pArray)
    {
        return def; 
    }

    VecT ret;
    ret.x = (*pArray)[0].value_or(def.x);
    ret.y = (*pArray)[1].value_or(def.y);
    ret.z = (*pArray)[2].value_or(def.y);
    return ret;
}

template<typename VecT, class T>
VecT toml_read_vec4(const T& node, const VecT& def = VecT(0.0f))
{
    const toml::array* pArray = node.as_array();
    if (!pArray)
    {
        return def; 
    }

    VecT ret;
    ret.x = (*pArray)[0].value_or(def.x);
    ret.y = (*pArray)[1].value_or(def.y);
    ret.z = (*pArray)[2].value_or(def.y);
    ret.w = (*pArray)[3].value_or(def.y);
    return ret;
}

template<class VecT>
void toml_write_vec2(toml::table& table, const std::string& strEntry, const VecT& value)
{
    table.insert_or_assign(strEntry, toml::array{ value.x, value.y });
}

template<class VecT>
void toml_write_vec3(toml::table& table, const std::string& strEntry, const VecT& value)
{
    table.insert_or_assign(strEntry, toml::array{ value.x, value.y, value.z });
}

template<class VecT>
void toml_write_vec4(toml::table& table, const std::string& strEntry, const VecT& value)
{
    table.insert_or_assign(strEntry, toml::array{ value.x, value.y, value.z, value.w });
}

inline toml::table* toml_sub_table(toml::table& tbl, const std::string& search)
{
    // Warning, this nukes the subtable; needs repair
    auto path = Zest::string_split(search, ".");

    toml::table* pParent = &tbl;
    for (auto& sub : path)
    {
        auto itr = pParent->insert_or_assign(sub, toml::table{});
        pParent = itr.first->second.as_table();
    }
    return pParent;
}
