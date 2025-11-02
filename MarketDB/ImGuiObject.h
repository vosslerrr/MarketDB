#pragma once
#include <imgui.h>

class ImGuiObject
{
public:
	ImGuiObject(const char* l_name, const char* l_label);
	ImGuiObject(const char* l_name, const char* l_label, ImFont* l_font);
 
	void drawTextBox(ImVec2 l_windowPos);
	void drawButton(ImVec2 l_windowPos);
	void drawHeaderButton(ImVec2 l_windowPos);

	void setInputEmpty();
	char* getInput();

	bool isPressed();
	void setPressedOff();

	void setInvalid();
	void setValid();
private:
	ImVec2 m_windowSize{ 231,45 };
	float m_borderSize{ 0.f };
	const char* m_name{};
	const char* m_label{};
	char m_input[128] = "";
	ImFont* m_font{};

	bool m_pressed{ false };
};