#include "ui_manager.h"

void UIManager::draw()
{

	if (ImGui::BeginMainMenuBar())
	{

		if (ImGui::BeginMenu("Orbit"))
		{
			ImGui::MenuItem("Time Control", NULL, &time_control);
			ImGui::MenuItem("Orbit Focus", NULL, &orbit_focus);
			ImGui::MenuItem("Orbit Plotter", NULL, &orbit_plotter);
			ImGui::Separator;
			ImGui::Text("Universal Time: %f days", time / (60.0 * 60.0 * 24.0));
			ImGui::Text("True Anomaly: %f", true_anom);
			ImGui::Text("Orbital Period: %f days", period / (60.0 * 60.0 * 24.0));
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Design"))
		{
			ImGui::MenuItem("De-Laval Nozzle", NULL, &de_laval_nozzle);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}


}
