#pragma once

#include <imgui.h>

namespace Speck::UI
{
  void Circle(float radius = 10.0f, ImU32 color = IM_COL32_WHITE)
  {
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 padding = ImGui::GetStyle().CellPadding;

    pos.x += padding.x + radius;
    pos.y += padding.y + radius;
    drawList->AddCircleFilled(pos, radius, color);

    ImVec2 frameSize = {2.0f * (padding.x + radius), 2.0f * (padding.y + radius)};
    ImGui::Dummy(frameSize);
  }

  void Square(float size = 20.0f, ImU32 color = IM_COL32_WHITE)
  {
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetCursorScreenPos();
    ImVec2 padding = ImGui::GetStyle().CellPadding;

    min.x += padding.x;
    min.y += padding.y;
    ImVec2 max = { min.x + size, min.y + size };

    drawList->AddRectFilled(min, max, color);
    ImGui::Dummy({size, size});
  }
}