#pragma once
#include <imgui.h>

class ImGuiButton 
{
public:
	ImGuiButton();

	void drawButton(ImVec2 l_windowPos, const char* l_name, const char* l_label);
	bool isPressed();
	void setPressedOff();
private:
	bool m_pressed{false};
};