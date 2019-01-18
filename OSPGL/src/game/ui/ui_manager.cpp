#include "ui_manager.h"

static void disabled_button(const char* label)
{
	ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_::ImGuiCol_Button);
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, col);
	ImGui::Button(label);
	ImGui::PopStyleColor();
}

// Obtains date. It starts counting from 1970 (atleast on Windows, TODO)
static std::string get_date(double secs)
{
	time_t time = secs;
	// We need to do this as gmtime returns a pointer to a static struct
	tm tmstr; gmtime_s(&tmstr, &time);

	std::string out;

	out += std::to_string(tmstr.tm_mday); out += "/";
	out += std::to_string(tmstr.tm_mon); out += "/";
	out += std::to_string(tmstr.tm_year + 1900); out += " ";
	out += std::to_string(tmstr.tm_hour); out += ":";
	out += std::to_string(tmstr.tm_min); out += ":";
	out += std::to_string(tmstr.tm_sec); out += " GMT";

	return out;
}

void UIManager::draw()
{

	if (ImGui::BeginMainMenuBar())
	{

		if (ImGui::BeginMenu("Orbit"))
		{
			ImGui::MenuItem("Time Control", NULL, &time_control);
			ImGui::MenuItem("Orbit Focus", NULL, &orbit_focus);
			ImGui::MenuItem("Orbit Plotter", NULL, &orbit_plotter);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Design"))
		{
			ImGui::MenuItem("De-Laval Nozzle", NULL, &de_laval_nozzle);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (time_control)
	{
		ImGui::Begin("Time Control", &time_control, 
			ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Date: %s", get_date(time).c_str());
		if (timewarp > 100.0f)
		{
			if (ImGui::Button("<<<")) { timewarp -= 100.0f; }
		}
		else
		{
			disabled_button("<<<");
		}
		
		ImGui::SameLine();
		if (timewarp > 10.0f)
		{
			if (ImGui::Button("<<")) { timewarp -= 10.0f; }
		}
		else
		{
			disabled_button("<<");
		}
		ImGui::SameLine();
		if (timewarp > 1.0f)
		{
			if (ImGui::Button("<")) { timewarp -= 1.0f; }
		}
		else
		{
			disabled_button("<");
		}
		ImGui::SameLine();
		if (timewarp == 0.0f)
		{
			if (ImGui::Button("|>")) { timewarp = prev_timewarp; }
		}
		else
		{
			if (ImGui::Button("||")) { prev_timewarp = timewarp; timewarp = 0.0f;}
		}
		ImGui::SameLine();
		if (ImGui::Button(">"))	{ timewarp += 1.0f;}
		ImGui::SameLine();
		if (ImGui::Button(">>")){timewarp += 10.0f;}
		ImGui::SameLine();
		if (ImGui::Button(">>>")) { timewarp += 100.0f; }

		ImGui::InputFloat("", &timewarp, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal);
		if (timewarp < 0.0f)
		{
			timewarp = 0.0f;
		}

		if (ImGui::Button("1")) { timewarp = 1.0f; }
		ImGui::SameLine();
		if (ImGui::Button("10")) { timewarp = 10.0f; }
		ImGui::SameLine();
		if (ImGui::Button("100")) { timewarp = 100.0f; }
		ImGui::SameLine();
		if (ImGui::Button("1000")) { timewarp = 1000.0f; }
		ImGui::SameLine();
		if (ImGui::Button("10000")) { timewarp = 10000.0f; }
		ImGui::SameLine();
		if (ImGui::Button("100000")) { timewarp = 100000.0f; }
		ImGui::SameLine();

		ImGui::End();
	}


}
