#include "ImGuiObject.h"
#include <iostream>

ImGuiObject::ImGuiObject() {}

ImGuiObject::ImGuiObject(ImFont* l_font)
{
	m_font = l_font;
}

void ImGuiObject::drawTextBox(ImVec2 l_windowPos, const char* l_name, const char* l_label)
{
	ImGui::SetNextWindowPos(l_windowPos);
	ImGui::SetNextWindowSize(m_windowSize);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, m_borderSize);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(155.f / 255.f, 173.f / 255.f, 183.f / 255.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 0.f, 0.f, 1.f));
	ImGui::Begin(l_name, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground| ImGuiWindowFlags_NoTitleBar);
	ImGui::InputText(l_label, m_input, size_t(m_input));
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar();
	ImGui::End();
}

void ImGuiObject::drawButton(ImVec2 l_windowPos, const char* l_name, const char* l_label)
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

void ImGuiObject::drawHeaderButton(ImVec2 l_windowPos, const char* l_name, const char* l_label)
{
	ImGui::SetNextWindowPos(l_windowPos);
	ImGui::PushFont(m_font);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(62.f / 255.f, 127.f / 255.f, 8.f / 255.f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(56.f / 255.f, 114.f / 255.f, 8.f / 255.f, 1.f));
	ImGui::Begin(l_name, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);

	if (ImGui::Button(l_label))
	{
		m_pressed = true;
	}

	ImGui::PopStyleColor(3);
	ImGui::PopFont();
	ImGui::End();
}

char* ImGuiObject::getInput()
{
	return m_input;
}

bool ImGuiObject::isPressed()
{
	return m_pressed;
}

void ImGuiObject::setPressedOff()
{
	m_pressed = false;
}

void ImGuiObject::setInvalid()
{
	m_borderSize = 1.f;
}

void ImGuiObject::setValid()
{
	m_borderSize = 0.f;
}


