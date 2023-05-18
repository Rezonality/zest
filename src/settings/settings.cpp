#include <zest/file/toml_utils.h>
#include <toml++/toml.h>

#pragma warning(disable: 4005)
#include <imgui.h>
#pragma warning(default: 4005)

#define DECLARE_SETTINGS
#include <zest/settings/settings.h>

namespace Zest
{

void SettingManager::DrawGUI(const std::string& name)
{
    std::vector<Zest::StringId> themeNames;

    if (ImGui::Begin(name.c_str()))
    {

        for (auto& [mstr, val] : m_themes[m_currentSetting])
        {
            themeNames.push_back(mstr);
        }

        std::sort(themeNames.begin(), themeNames.end(), [](auto& lhs, auto& rhs) {
            return lhs.ToString().substr(2) < rhs.ToString().substr(2);
        });

        std::string last;
        auto& themeMap = m_themes[m_currentSetting];
        for (auto& id : themeNames)
        {
            auto name = id.ToString();
            auto& val = themeMap[id];
            auto prefix = name.substr(0, 2);

            if (!last.empty() && (last != name.substr(2, 4)))
            {
                ImGui::NewLine();
            }
            last = name.substr(2, 4);

            if (prefix == "c_")
            {
                glm::vec4 v = val.ToVec4f();
                if (ImGui::ColorEdit4(name.c_str(), &v[0]))
                {
                    val.f4 = v;
                }
            }
            else if (prefix == "b_")
            {
                bool v = val.ToBool();
                if (ImGui::Checkbox(name.c_str(), &v))
                {
                    val.b = v;
                }
            }
            else if (prefix == "s_")
            {
                float f = 0.0f;
                switch (val.type)
                {
                case SettingType::Float:
                    ImGui::DragFloat(name.c_str(), &val.f);
                    break;
                case SettingType::Vec2f:
                    ImGui::DragFloat2(name.c_str(), &val.f);
                    break;
                case SettingType::Vec3f:
                    ImGui::DragFloat3(name.c_str(), &val.f);
                    break;
                case SettingType::Vec4f:
                    ImGui::DragFloat4(name.c_str(), &val.f);
                    break;
                }
            }
        }
    }
    ImGui::End();
}

bool SettingManager::Save(const std::filesystem::path& path)
{
    toml::table tbl;

    for (const auto& [theme_name, values] : m_themes)
    {
        toml::table values_table;
        for (const auto& [value_name, value] : values)
        {
            switch (value.type)
            {
            case SettingType::Float:
                values_table.insert(value_name.ToString(), value.ToFloat());
                break;
            case SettingType::Bool:
                values_table.insert(value_name.ToString(), value.ToBool());
                break;
            case SettingType::Vec2f:
            {
                toml_write_vec2(values_table, value_name.ToString(), value.ToVec2f());
            }
            break;
            case SettingType::Vec3f:
            {
                toml_write_vec3(values_table, value_name.ToString(), value.ToVec3f());
            }
            break;
            case SettingType::Vec4f:
            {
                toml_write_vec4(values_table, value_name.ToString(), value.ToVec4f());
            }
            break;
            case SettingType::Vec2i:
            {
                toml_write_vec2(values_table, value_name.ToString(), value.ToVec2i());
            }
            break;
            }
        }
        tbl.insert(theme_name, values_table);
    }

    std::ofstream fs(path, std::ios_base::trunc);
    fs << tbl;
    return true;
}

bool SettingManager::Load(const std::filesystem::path& path)
{
    toml::table tbl;
    try
    {
        tbl = toml::parse_file(path.string());
        for (auto& [theme, value] : tbl)
        {
            if (value.is_table())
            {
                auto theme_table = value.as_table();
                if (theme_table)
                {
                    for (auto& [themeName, themeValue] : *theme_table)
                    {
                        if (themeValue.is_array())
                        {
                            auto name = std::string(themeName.str());
                            auto arr = themeValue.as_array();
                            switch (arr->size())
                            {
                            case 2:
                                Set(StringId(std::string(themeName.str())), toml_read_vec2(themeValue, glm::vec2(0.0f)));
                                break;
                            case 3:
                                Set(StringId(std::string(themeName.str())), toml_read_vec3(themeValue, glm::vec3(0.0f)));
                                break;
                            case 4:
                                Set(StringId(std::string(themeName.str())), toml_read_vec4(themeValue, glm::vec4(0.0f)));
                                break;
                            }
                        }
                        else if (themeValue.is_floating_point())
                        {
                            Set(StringId(std::string(themeName.str())), (float)themeValue.as_floating_point()->get());
                        }
                        else if (themeValue.is_boolean())
                        {
                            Set(StringId(std::string(themeName.str())), (bool)themeValue.as_boolean()->get());
                        }
                    }
                }
            }
        }
    }
    catch (const toml::parse_error&)
    {
        return false;
    }
    return true;
}

} // Zest