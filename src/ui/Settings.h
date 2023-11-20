#pragma once

#include <imgui.h>

#include "ui/Shapes.h"

#include "simulation/ColorMatrix.h"

namespace Speck::UI
{

void DisplayColorMatrix(ColorMatrix& matrix)
{
  std::size_t colors = matrix.GetNumColors();
  if (ImGui::BeginTable("color_matrix", colors + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame))
  {
    // Matrix Headers
    {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      for (std::size_t column = 1; column < colors + 1; column++)
      {
        ImGui::TableSetColumnIndex(column);
        glm::vec4 col = matrix.GetColor(column - 1);
        UI::Circle(8.0f, ImGui::GetColorU32({col.r, col.g, col.b, col.a}));
      }
    }

    for (int row = 0; row < colors; row++)
    {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      // Row Headers
      glm::vec4 col = matrix.GetColor(row);
      UI::Circle(8.0f, ImGui::GetColorU32({col.r, col.g, col.b, col.a}));

      for (int column = 0; column < colors; column++)
      {
        ImGui::TableSetColumnIndex(column + 1);

        glm::vec4 col;
        float scale = matrix.GetAttractionScale(row, column);
        if (scale >= 0.0f) 
          col = glm::vec4(0.0f, scale, 0.0f, 1.0f);
        else 
          col = glm::vec4(-scale, 0.0f, 0.0f, 1.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
        UI::Square(ImGui::GetColumnWidth(), ImGui::GetColorU32({col.r, col.g, col.b, col.a}));
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        // Convert Scale to a Color
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) 
        {
          scale += 0.1f;
          matrix.SetAttractionScale(row, column, glm::clamp(scale, -1.0f, 1.0f));
        }
        else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
          scale -= 0.1f;
          matrix.SetAttractionScale(row, column, glm::clamp(scale, -1.0f, 1.0f));
        };
      }
    }
    ImGui::EndTable();
  }
}

}