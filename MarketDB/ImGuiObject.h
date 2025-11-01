#pragma once
#include <imgui.h>

class ImGuiObject
{
public:
	ImGuiObject();
 
	void drawTextBox(ImVec2 l_windowPos, const char* l_name, const char* l_label);
	void drawButton(ImVec2 l_windowPos, const char* l_name, const char* l_label);

	char* getInput();

	bool isPressed();
	void setPressedOff();

	void setInvalid();
	void setValid();
private:
	char m_input[128] = "";
	ImVec2 m_windowSize{231,45};
	bool m_pressed{ false };
	float m_borderSize{ 0.f };
};