#pragma once
#include <imgui.h>
#include <imgui-SFML.h>

class ImGuiTextBox
{
public:
	ImGuiTextBox(ImVec2 l_windowPos, const char* l_name, const char* l_label, char* l_textIn, ImFont* l_font);

private:
	ImVec2 m_windowSize{231,45};
};