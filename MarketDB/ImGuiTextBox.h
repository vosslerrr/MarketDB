#pragma once
#include <imgui.h>
#include <imgui-SFML.h>

class ImGuiTextBox
{
public:
	ImGuiTextBox();
 
	void draw(ImVec2 l_windowPos, const char* l_name, const char* l_label);
	char* getInput();
private:
	char m_input[128] = "";
	ImVec2 m_windowSize{231,45};
};