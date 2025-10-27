#include "ImGUITextBox.h"

ImGuiTextBox::ImGuiTextBox(){}

ImGuiTextBox::ImGuiTextBox(ImVec2 l_windowPos, const char* l_name, const char* l_label, char* l_textIn, ImFont* l_font)
{
	ImGui::SetNextWindowPos(l_windowPos);
	ImGui::SetNextWindowSize(m_windowSize); 
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(155.f / 255.f, 173.f / 255.f, 183.f / 255.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImGui::PushFont(l_font);
	ImGui::Begin(l_name, nullptr, ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar);
	ImGui::InputText(l_label, l_textIn, 128);
	ImGui::PopStyleColor(2);
	ImGui::PopFont();
	ImGui::End();
}

ImGuiTextBox::~ImGuiTextBox(){}

