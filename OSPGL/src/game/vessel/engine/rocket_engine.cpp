#include "rocket_engine.h"



void RocketEngine::imgui_draw()
{
	if (ui_manager.de_laval_nozzle)
	{
		if (ImGui::Begin("De-Laval Nozzle", &ui_manager.de_laval_nozzle))
		{


			ImGui::Text("Radius:");
			ImGui::Columns(3);
			ImGui::SliderFloat("Inlet", &nozzle.inlet_radius, 0.1f, 100.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Throat", &nozzle.throat_radius, 0.1f, 100.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Outlet", &nozzle.outlet_radius, 0.1f, 100.0f);
			ImGui::EndColumns();

			ImGui::Text("Length:");
			ImGui::Columns(2);
			ImGui::SliderFloat("Inlet##", &nozzle.inlet_length, 0.1f, 100.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Outlet##", &nozzle.outlet_length, 0.1f, 100.0f);
			ImGui::EndColumns();

			ImGui::Text("Slope:");
			ImGui::Columns(3);
			ImGui::SliderFloat("Inlet###", &nozzle.inlet_slope, -30.0f, 0.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Throat###", &nozzle.throat_slope, 0.0f, 30.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Outlet###", &nozzle.outlet_slope, -10.0f, 30.0f);
			ImGui::EndColumns();

			ImGui::Text("Propellant Properties");
			ImGui::Columns(3);
			ImGui::SliderFloat("Pressure", &inlet_p, 0.0f, 100.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Temperature", &inlet_t, 0.0f, 10000.0f);
			ImGui::NextColumn();
			ImGui::SliderFloat("Adiabatic", &propellant_k, 1.01f, 4.5f);
			ImGui::EndColumns();

			ImGui::Separator();
			const char* items[] = { "Mach Number", "Pressure", "Temperature" };
			ImGui::ListBox("Preview Type", &item, items, 3);
			ImGui::Text("Preview: ");


			ImGuiWindow* window = ImGui::GetCurrentWindow();
			const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y));


			std::vector<NozzleData> data;
			for (float x = 0.0f; x < nozzle.outlet_length + nozzle.inlet_length; x += 0.1f)
			{
				NozzleData dat = nozzle.simulate(inlet_p, inlet_t, propellant_k, x);
				data.push_back(dat);
			}

			float max_radius = -1.0f;
			float max_temp = -1.0f;
			float max_pres = -1.0f;
			float max_mach = -1.0f;

			for (size_t i = 0; i < data.size(); i++)
			{
				if (data[i].mach > max_mach)
				{
					max_mach = data[i].mach;
				}
				if (data[i].temp > max_temp)
				{
					max_temp = data[i].temp;
				}
				if (data[i].radius > max_radius)
				{
					max_radius = data[i].radius;
				}
				if (data[i].pres > max_pres)
				{
					max_pres = data[i].pres;
				}

			}

			float mid_cord = max_radius * 2.5f;
			float graph_height = 128.0f;

			float prev_y_graph = 0.0f;

			// Draw graph skeleton
			ImVec2 g0 = ImVec2(frame_bb.Min.x, frame_bb.Min.y + mid_cord * 2.0f);
			ImVec2 g1 = ImVec2(frame_bb.Min.x + data.size(), frame_bb.Min.y + mid_cord * 2.0f + graph_height);
			window->DrawList->AddRect(g0, g1, ImColor(0.4f, 0.4f, 0.4f));

			ImVec2 pp0 = ImVec2(frame_bb.Min.x, mid_cord + frame_bb.Min.y);
			ImVec2 pp1 = ImVec2(frame_bb.Min.x, mid_cord + frame_bb.Min.y);

			ImVec2 mouse = ImGui::GetMousePos();

			for (size_t i = 0; i < data.size(); i++)
			{
				float x_scl = i;
				float y_cord = mid_cord - data[i].radius * 2.0f;
				float yy_cord = mid_cord + data[i].radius * 2.0f;
				ImVec2 p0 = ImVec2(x_scl + frame_bb.Min.x, y_cord + frame_bb.Min.y);
				ImVec2 p1 = ImVec2(x_scl + frame_bb.Min.x + 1.0f, yy_cord + frame_bb.Min.y);
				ImColor col;
				if (item == 0)
				{
					// We use two colors, blue for subsonic flow and red for supersonic
					// flow so the engine can be more intuitive
					if (data[i].mach < 1.0)
					{
						col = ImColor(0.0f, data[i].mach * 0.5f, data[i].mach, 1.0f);
					}
					else
					{
						col = ImColor((data[i].mach / max_mach), 0.5f, 1.0f - (data[i].mach / max_mach), 1.0f);
					}

				}
				else if (item == 1)
				{
					col = ImColor((data[i].pres / max_pres), 0.2f, 0.5f, 1.0f);
				}
				else
				{
					col = ImColor((data[i].temp / max_temp), 0.2f, 0.5f, 1.0f);
				}


				if ((mouse.x < p1.x && mouse.y < p1.y && mouse.x >= p0.x && mouse.y >= p0.y)
					|| (mouse.y >= g0.y && mouse.y < g1.y && mouse.x >= p0.x && mouse.x < p1.x))
				{
					ImGui::BeginTooltip();
					ImGui::Text("Mach: %f", data[i].mach);
					ImGui::Text("Pressure: %fMPa", data[i].pres);
					ImGui::Text("Temperature: %fK", data[i].temp);
					ImGui::Text("Radius: %fm", data[i].radius);
					ImGui::EndTooltip();

					// Draw Graph selector
					//col = ImColor(1.0f - col.Value.x, 1.0f - col.Value.y, 1.0f - col.Value.z);
					col = ImColor(1.0f, 0.7f, 0.7f);

					ImVec2 gl0 = ImVec2(x_scl + g0.x, g0.y);
					ImVec2 gl1 = ImVec2(x_scl + g0.x, g1.y);
					window->DrawList->AddLine(gl0, gl1, ImColor(1.0f, 0.7f, 0.7f));
				}
				window->DrawList->AddRectFilled(p0, p1, col);
				window->DrawList->AddLine(pp0, p0, ImColor(1.0f, 1.0f, 1.0f), 2.0f);
				window->DrawList->AddLine(pp1, ImVec2(p1.x - 1.0f, p1.y), ImColor(1.0f, 1.0f, 1.0f), 2.0f);

				// Draw small graph underneath
				float y_graph = 0.0f;
				if (item == 0)
				{
					y_graph = data[i].mach / max_mach;
				}
				else if (item == 1)
				{
					y_graph = data[i].pres / max_pres;
				}
				else
				{
					y_graph = data[i].temp / max_temp;
				}
				y_graph *= graph_height;

				ImVec2 l0 = ImVec2(x_scl - 1.0f + g0.x, g1.y - prev_y_graph);
				ImVec2 l1 = ImVec2(x_scl + g0.x, g1.y - y_graph);
				window->DrawList->AddLine(l0, l1, ImColor(0.8f, 0.8f, 0.8f));

				prev_y_graph = y_graph;
				pp0 = p0;
				pp1 = ImVec2(p1.x - 1.0f, p1.y);
			}



			if (item == 0)
			{
				// Draw Mach1 Line
				float y = 1.0f / max_mach;
				y *= graph_height;
				window->DrawList->AddLine(ImVec2(g0.x, g1.y - y), ImVec2(g1.x, g1.y - y), ImColor(1.0f, 0.0f, 0.0f));
			}


		}
		ImGui::End();
	}
}

RocketEngine::RocketEngine()
{
	nozzle.inlet_radius = 15.0f;
	nozzle.inlet_length = 2.0f;
	nozzle.inlet_slope = 10.0f;
	nozzle.throat_radius = 5.0f;
	nozzle.throat_slope = 1.0f;
	nozzle.outlet_radius = 15.0f;
	nozzle.outlet_length = 10.0f;
	nozzle.outlet_slope = 0.0f;
}


RocketEngine::~RocketEngine()
{
}

float Nozzle::get_radius(float t)
{
	if (t <= inlet_length)
	{
		float x = t / inlet_length;

		float t0 = (std::powf(x, 3.0f) - 2.0f * std::powf(x, 2.0f) + x);
		float t1 = (2.0f * std::powf(x, 3.0f) - 3.0f * std::powf(x, 2.0f) + 1.0f);
		float t2 = (-2.0f * std::powf(x, 3.0f) + 3.0f * std::powf(x, 2.0f));
		return t0 * inlet_slope + t1 * inlet_radius + t2 * throat_radius;
	}
	else
	{
		float x = (t - inlet_length) / outlet_length;

		float t0 = (std::powf(x, 3.0f) - 2.0f * std::powf(x, 2.0f) + x);
		float t1 = (std::powf(x, 3.0f) - std::powf(x, 2.0f));
		float t2 = (2.0f * std::powf(x, 3.0f) - 3.0f * std::powf(x, 2.0f) + 1.0f);
		float t3 = (-2.0f * std::powf(x, 3.0f) + 3.0f * std::powf(x, 2.0f));

		return t0 * throat_slope + t1 * outlet_slope + t2 * throat_radius + t3 * outlet_radius;
	}
}

static double mach_area_eval_lhs(double area, double choke_area, double ad, double mach)
{
	double mach2 = mach * mach;
	double k_div = (ad + 1.0f) / (ad - 1.0f);
	double area_div = area / choke_area;
	double t0 = 1.0 / mach2;
	double t1 = 2.0 / (ad + 1.0);
	double t2 = 1.0 + ((ad - 1.0) / 2.0) * mach2;
	double t3 = area_div * area_div;

	double res = t0 * std::powf(t1 * t2, k_div) - t3;
	return res;
	
}

static float mach_area_relation_solve(float area, float choke_area, float ad, bool super_sonic, float tol = 0.05f)
{
	if (area < choke_area)
	{
		return 0.0f;
	}
	float sol = 0.0f;

	bool exit = false;

	size_t it = 0;


	// rhs is always 0 
	// We do a newton method search for the solution
	double p;
	if (super_sonic)
	{
		p = 10.0;
	}
	else
	{
		p = 0.0001;
	}

	while (!exit)
	{
		
		double pnext;
		if (super_sonic && p >= 5.0f)
		{
			pnext = p - 1.0f;
		}
		else
		{
			pnext = p + 0.01f;
		}

		// Estimate a tangent
		double y0 = mach_area_eval_lhs(area, choke_area, ad, p);

		// Check if we are close to solution 
		if (std::abs(y0) <= tol)
		{
			return p;
		}

		double y1 = mach_area_eval_lhs(area, choke_area, ad, pnext);

		// Find x-intercept of x0->x1
		double slope = (y1 - y0) / (pnext - p);
		// y - y1 = m(x - pnext)
		// y = mx - m(pnext) + y1  |  (c =- m(pnext) + y1)
		double c = -slope * pnext + y1;
		// y = mx + c, 0 = mx + c, x = -(c/m)
		p = -(c / slope);

		it++;
		if (it > 10000)
		{
			exit = true;
		}
	}

	return sol;
}

NozzleData Nozzle::simulate(float P0, float T0, float ad, float t)
{
	NozzleData out;

	float A = 3.14f * std::powf(get_radius(t), 2.0f);

	// Check the desmos file for all this stuff
	// Solve mach (quite hard, based on the mach-area relation)
	if (t < inlet_length)
	{
		// Solve subsonic (< 1)
		out.mach = mach_area_relation_solve(A, 3.14f * std::powf(throat_radius, 2.0f), ad, false);
	}
	else if(t > inlet_length)
	{
		// Solve supersonic (> 1)
		out.mach = mach_area_relation_solve(A, 3.14f * std::powf(throat_radius, 2.0f), ad, true);
	}
	else
	{
		// Choke-point
		out.mach = 1;
	}



	// Solve pressure and temperature
	float pt = std::powf(1 + ((ad - 1.0f) / 2.0f) * out.mach * out.mach, 1.0f / (ad - 1.0f));
	out.pres = P0 / pt;

	float tt = 1.0f + ((ad - 1.0f) / 2.0f) * out.mach * out.mach;
	out.temp = T0 / tt;

	out.radius = get_radius(t);

	return out;
}
