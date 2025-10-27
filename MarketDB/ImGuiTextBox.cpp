#include "ImGUITextBox.h"

ImGuiTextBox::ImGuiTextBox(){}

void ImGuiTextBox::draw(ImVec2 l_windowPos, const char* l_name, const char* l_label)
{
	ImGui::SetNextWindowPos(l_windowPos);
	ImGui::SetNextWindowSize(m_windowSize);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(155.f / 255.f, 173.f / 255.f, 183.f / 255.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImGui::Begin(l_name, nullptr, ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar);
	ImGui::InputText(l_label, m_input, size_t(m_input));
	ImGui::PopStyleColor(2);
	ImGui::End();
}

char* ImGuiTextBox::getInput()
{
	return m_input;
}

