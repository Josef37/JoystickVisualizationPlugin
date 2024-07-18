#include "pch.h"
#include "JoystickVisualizationPlugin.h"

static void Separator() {
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}

void JoystickVisualizationPlugin::RenderSettings() {
	CVarWrapper enableCvar = cvarManager->getCvar(JOYSTICK_VIS_ENABLED);
	if (!enableCvar) { return; }
	bool enabled = enableCvar.getBoolValue();
	if (ImGui::Checkbox("Show Visualization", &enabled)) {
		enableCvar.setValue(enabled);
	}

	CVarWrapper pointCountCVar = cvarManager->getCvar(JOYSTICK_VIS_POINT_COUNT);
	if (!pointCountCVar) { return; }
	int pointCount = pointCountCVar.getIntValue();
	if (ImGui::SliderInt("Number of Inputs to Visualize", &pointCount, 1, 600)) {
		pointCountCVar.setValue(pointCount);
	}

	CVarWrapper sensitivityCvar = cvarManager->getCvar(JOYSTICK_VIS_SENSITIVITY);
	if (!sensitivityCvar) { return; }
	bool sensitivity = sensitivityCvar.getBoolValue();
	if (ImGui::Checkbox("Scale Input with Aerial Sensitivity", &sensitivity)) {
		sensitivityCvar.setValue(sensitivity);
	}

	CVarWrapper clampCvar = cvarManager->getCvar(JOYSTICK_VIS_CLAMP);
	if (!clampCvar) { return; }
	bool clamp = clampCvar.getBoolValue();
	if (ImGui::Checkbox("Clamp Values to Max Input", &clamp)) {
		clampCvar.setValue(clamp);
	}

	CVarWrapper colorBoxCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_BOX);
	if (!colorBoxCvar) { return; }
	LinearColor colorBox = colorBoxCvar.getColorValue() / 255;
	if (ImGui::ColorEdit4("Box Color", &colorBox.R, ImGuiColorEditFlags_AlphaBar)) {
		colorBoxCvar.setValue(colorBox * 255);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##BoxColor")) {
		colorBoxCvar.setValue(LinearColor(255, 255, 255, 100));
	}

	CVarWrapper colorPointCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_POINT);
	if (!colorPointCvar) { return; }
	LinearColor colorPoint = colorPointCvar.getColorValue() / 255;
	if (ImGui::ColorEdit4("Point Color", &colorPoint.R, ImGuiColorEditFlags_AlphaBar)) {
		colorPointCvar.setValue(colorPoint * 255);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##PointColor")) {
		colorPointCvar.setValue(LinearColor(255, 255, 255, 255));
	}

	CVarWrapper colorDeadzoneCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_DEADZONE);
	if (!colorDeadzoneCvar) { return; }
	LinearColor colorDeadzone = colorDeadzoneCvar.getColorValue() / 255;
	if (ImGui::ColorEdit4("Point Color in Deadzone", &colorDeadzone.R, ImGuiColorEditFlags_AlphaBar)) {
		colorDeadzoneCvar.setValue(colorDeadzone * 255);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset##DeadzoneColor")) {
		colorDeadzoneCvar.setValue(colorPointCvar.getColorValue());
	}

	Separator();
	ImGui::Text("Sizing");

	CVarWrapper sizeCvar = cvarManager->getCvar(JOYSTICK_VIS_SIZE);
	if (!sizeCvar) { return; }
	int size = sizeCvar.getIntValue();
	if (ImGui::SliderInt("Visualization Size", &size, 100, 1000)) {
		sizeCvar.setValue(size);
	}

	CVarWrapper centerXCvar = cvarManager->getCvar(JOYSTICK_VIS_CENTER_X);
	if (!centerXCvar) { return; }
	float centerX = centerXCvar.getFloatValue() * 100;
	if (ImGui::SliderFloat("Center X", &centerX, 0.0f, 100.0f, "%.1f %%")) {
		centerXCvar.setValue(centerX / 100.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Center##X")) {
		centerXCvar.setValue(0.5f);
	}

	CVarWrapper centerYCvar = cvarManager->getCvar(JOYSTICK_VIS_CENTER_Y);
	if (!centerYCvar) { return; }
	float centerY = centerYCvar.getFloatValue() * 100;
	if (ImGui::SliderFloat("Center Y", &centerY, 0.0f, 100.0f, "%.1f %%")) {
		centerYCvar.setValue(centerY / 100.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Center##Y")) {
		centerYCvar.setValue(0.5f);
	}

	CVarWrapper pointSizeCvar = cvarManager->getCvar(JOYSTICK_VIS_POINT_SIZE);
	if (!pointSizeCvar) { return; }
	float pointSize = pointSizeCvar.getFloatValue() * 100;
	if (ImGui::SliderFloat("Size of Points Relative to Box", &pointSize, 0.0f, 10.0f, "%.1f %%")) {
		pointSizeCvar.setValue(pointSize / 100.0f);
	}

	Separator();

	ImGui::Text("Tips");
	ImGui::Text(
		"- Joystick should trace smooth and consistent lines.\n"
		"- Jagged / inconsistent traces may indicate controller needs to be repaired or replaced. Try using compressed air to clean out the sensor.\n"
		"- Try rotating the joystick all the way around edge in a circle.\n"
		"- The default deadzone shape and sensitivity should trace a circle.\n"
		"- Setting a higher sensitivity with an external tool (such as DS4Win or Steam controller config) will increase the size of circle and cut off edges.\n"
		"- A squared deadzone (configured in external tools) should instead trace a square.\n"
		"- The visualization ignores inputs within the deadzone. Only inputs affecting the movement of the car are shown. This is a limitation in Bakkesmod.\n"
		"- See plugin homepage for examples and more information�."
	);

	Separator();

	ImGui::Text("Originally created by @AlpacaFlightSim, updated by @Brotzeitsepp.");
}
