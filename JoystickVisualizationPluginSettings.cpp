#include "pch.h"
#include "JoystickVisualizationPlugin.h"

static void RenderSeparator() {
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}

static void RenderCheckbox(CVarWrapper cw, const char* label) {
	if (!cw) { return; }
	bool value = cw.getBoolValue();
	if (ImGui::Checkbox(label, &value)) {
		cw.setValue(value);
	}
}

static void RenderSliderInt(CVarWrapper cw, const char* label, int min, int max) {
	if (!cw) { return; }
	int value = cw.getIntValue();
	if (ImGui::SliderInt(label, &value, min, max)) {
		cw.setValue(value);
	}
}

static void RenderSliderPercentage(CVarWrapper cw, const char* label, float min, float max) {
	if (!cw) { return; }
	float value = cw.getFloatValue() * 100;
	if (ImGui::SliderFloat(label, &value, min, max, "%.1f %%")) {
		cw.setValue(value / 100);
	}
}

static void RenderColorEdit(CVarWrapper cw, const char* label) {
	if (!cw) { return; }
	LinearColor color = cw.getColorValue() / 255;
	if (ImGui::ColorEdit4(label, &color.R, ImGuiColorEditFlags_AlphaBar)) {
		cw.setValue(color * 255);
	}
}

void JoystickVisualizationPlugin::RenderSettings() {
	CVarWrapper enableCvar = cvarManager->getCvar(JOYSTICK_VIS_ENABLED);
	RenderCheckbox(enableCvar, "Show Visualization");

	CVarWrapper pointCountCVar = cvarManager->getCvar(JOYSTICK_VIS_POINT_COUNT);
	RenderSliderInt(pointCountCVar, "Number of Inputs to Visualize", 1, 600);

	CVarWrapper sensitivityCvar = cvarManager->getCvar(JOYSTICK_VIS_SENSITIVITY);
	RenderCheckbox(sensitivityCvar, "Scale Input with Aerial Sensitivity");

	CVarWrapper clampCvar = cvarManager->getCvar(JOYSTICK_VIS_CLAMP);
	RenderCheckbox(clampCvar, "Clamp Values to Max Input");

	RenderSeparator();
	ImGui::Text("Styling");

	CVarWrapper colorBoxCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_BOX);
	RenderColorEdit(colorBoxCvar, "Box Color"); ImGui::SameLine();
	if (ImGui::Button("Reset##BoxColor")) { colorBoxCvar.ResetToDefault(); }

	CVarWrapper fillBoxCvar = cvarManager->getCvar(JOYSTICK_VIS_FILL_BOX); ImGui::SameLine();
	RenderCheckbox(fillBoxCvar, "Fill Box");

	CVarWrapper colorPointCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_POINT);
	RenderColorEdit(colorPointCvar, "Point Color"); ImGui::SameLine();
	if (ImGui::Button("Reset##PointColor")) { colorPointCvar.ResetToDefault(); }

	CVarWrapper colorDeadzoneCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_DEADZONE);
	RenderColorEdit(colorDeadzoneCvar, "Point Color in Deadzone"); ImGui::SameLine();
	if (ImGui::Button("Match Point")) { colorDeadzoneCvar.setValue(colorPointCvar.getColorValue()); } ImGui::SameLine();
	if (ImGui::Button("Reset##DeadzoneColor")) { colorDeadzoneCvar.ResetToDefault(); }

	CVarWrapper colorPointJumpCvar = cvarManager->getCvar(JOYSTICK_VIS_COLOR_JUMP);
	RenderColorEdit(colorPointJumpCvar, "Point Color for Flips"); ImGui::SameLine();
	if (ImGui::Button("Reset##JumpColor")) { colorPointJumpCvar.ResetToDefault(); }

	RenderSeparator();
	ImGui::Text("Sizing");

	CVarWrapper sizeCvar = cvarManager->getCvar(JOYSTICK_VIS_SIZE);
	RenderSliderInt(sizeCvar, "Visualization Size", 100, 1000);

	CVarWrapper centerXCvar = cvarManager->getCvar(JOYSTICK_VIS_CENTER_X);
	RenderSliderPercentage(centerXCvar, "Center X", 0, 100); ImGui::SameLine();
	if (ImGui::Button("Reset##X")) { centerXCvar.ResetToDefault(); }

	CVarWrapper centerYCvar = cvarManager->getCvar(JOYSTICK_VIS_CENTER_Y);
	RenderSliderPercentage(centerYCvar, "Center Y", 0, 100); ImGui::SameLine();
	if (ImGui::Button("Reset##Y")) { centerYCvar.ResetToDefault(); }

	CVarWrapper pointSizeCvar = cvarManager->getCvar(JOYSTICK_VIS_POINT_SIZE);
	RenderSliderPercentage(pointSizeCvar, "Size of Points Relative to Box", 0, 10);

	CVarWrapper pointJumpSizeCvar = cvarManager->getCvar(JOYSTICK_VIS_JUMP_SIZE);
	RenderSliderPercentage(pointJumpSizeCvar, "Size of Points for Flips Relative to Box", 0, 20);

	RenderSeparator();
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

	RenderSeparator();
	ImGui::Text("Originally created by @AlpacaFlightSim, improved upon by @Brotzeitsepp.");
}
