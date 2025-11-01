#include "ImGuiButton.h"

ImGuiButton::ImGuiButton(){}

void ImGuiButton::drawButton(ImVec2 l_windowPos, const char* l_name, const char* l_label)
{
	ImGui::SetNextWindowPos(l_windowPos);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(100.f / 255.f, 200.f / 255.f, 183.f / 255.f, 1.f));
	ImGui::Begin(l_name, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);

	if (ImGui::Button(l_label))
	{
		m_pressed = true;
	}

	ImGui::PopStyleColor(2);
	ImGui::End();
}

bool ImGuiButton::isPressed()
{
	return m_pressed;
}

void ImGuiButton::setPressedOff()
{
	m_pressed = false;
}
