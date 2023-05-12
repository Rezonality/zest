#include <string>
#include <vector>

#include <zest/math/math.h>

namespace ImGui
{
bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);
bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);
bool DragIntRange4(const char* label, Zest::NVec4<int>& v, float v_speed, int v_min, int v_max, const char* format = "%d", const char* format_max = nullptr);
} // namespace ImGui
