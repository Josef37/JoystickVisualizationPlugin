#include "pch.h"
#include "JoystickSelfCheckPluginClone.h"

BAKKESMOD_PLUGIN(JoystickSelfCheckPluginClone, "Cool Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

int input_history_length = 120;

void JoystickSelfCheckPluginClone::onLoad()
{
	enabled = std::make_shared<bool>(true);
	joystickVizSize = std::make_shared<int>(400);
	highlightDeadzone = std::make_shared<bool>(true);

	inputHistory.reserve(input_history_length);

	cvarManager->log("JoystickSelfCheckPlugin loaded");

	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput", std::bind(&JoystickSelfCheckPluginClone::OnSetInput, this, std::placeholders::_1, std::placeholders::_2));

	gameWrapper->RegisterDrawable(std::bind(&JoystickSelfCheckPluginClone::Render, this, std::placeholders::_1));

	cvarManager->registerCvar("joystick_viz_clone_enabled", "1", "Show Joystick Self-Check Visualization", true, true, 0, true, 1)
		.bindTo(enabled);
	cvarManager->registerCvar("joystick_viz_clone_size", "400", "Joystick Visualization Size", true, true, 100, true, 1000)
		.bindTo(joystickVizSize);
	cvarManager->registerCvar("joystick_viz_clone_deadzone", "1", "Highlight Points in Deadzone", true, true, 0, true, 1)
		.bindTo(highlightDeadzone);
}

void JoystickSelfCheckPluginClone::onUnload() {
	cvarManager->log("JoystickSelfCheckPlugin unloaded");
	gameWrapper->UnregisterDrawables();
}

bool JoystickSelfCheckPluginClone::isActive() {
	return (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining())
		&& !gameWrapper->IsPaused()
		&& *enabled;
}

void JoystickSelfCheckPluginClone::OnSetInput(CarWrapper cw, void* params)
{
	if (!isActive()) {
		return;
	}

	ControllerInput* ci = (ControllerInput*)params;

	if (inputHistory.size() >= input_history_length) {
		int numToRemove = 1 + inputHistory.size() - input_history_length;
		inputHistory.erase(inputHistory.begin(), inputHistory.begin() + numToRemove);
	}

	inputHistory.push_back(*ci);
}

void JoystickSelfCheckPluginClone::Render(CanvasWrapper canvas)
{
	if (!isActive()) {
		return;
	}

	int pointSize = 5;
	int boxSize = *joystickVizSize + pointSize;

	Vector2 canvasSize = canvas.GetSize();
	Vector2F canvasCenter = Vector2F(canvasSize.X / 2.0f, canvasSize.Y / 2.0f);

	canvas.SetColor(255, 255, 255, 100);
	canvas.SetPosition(Vector2F(canvasCenter.X - boxSize / 2.0f, canvasCenter.Y - boxSize / 2.0f));
	canvas.DrawBox(Vector2F(boxSize, boxSize));

	int i = 0;
	Vector2F prevPos;

	for (auto it = inputHistory.rbegin(); it != inputHistory.rend(); ++it)
	{
		ControllerInput input = *it;

		Vector2F currentPos = Vector2F(
			canvasCenter.X + input.Steer * (*joystickVizSize / 2.0f),
			canvasCenter.Y + input.Pitch * (*joystickVizSize / 2.0f)
		);

		float alpha = 255 * (1 - i / (float)input_history_length);
		bool isInDeadzone = input.Steer == 0.0f || input.Pitch == 0.0f;
		int blue = isInDeadzone && highlightDeadzone ? 0 : 255;
		canvas.SetColor(255, 255, blue, alpha);
		Vector2F offset = Vector2F(pointSize / 2.0, pointSize / 2.0);
		canvas.SetPosition(currentPos - offset);
		canvas.FillBox(Vector2({ pointSize, pointSize }));

		if (i > 0) {
			canvas.DrawLine(currentPos, prevPos);
		}

		prevPos = currentPos;
		i++;
	}
}

void JoystickSelfCheckPluginClone::RenderSettings() {
	ImGui::Text("Enable the joystick visualization to self - check accuracy of joystick inputs, externally configured deadzone shape and sensitivity, etc.");

	CVarWrapper enableCvar = cvarManager->getCvar("joystick_viz_clone_enabled");
	if (!enableCvar) { return; }
	bool enabled = enableCvar.getBoolValue();
	if (ImGui::Checkbox("Enable plugin", &enabled)) {
		enableCvar.setValue(enabled);
	}

	CVarWrapper deadzoneCvar = cvarManager->getCvar("joystick_viz_clone_deadzone");
	if (!deadzoneCvar) { return; }
	bool deadzone = deadzoneCvar.getBoolValue();
	if (ImGui::Checkbox("Highlight Points in Deadzone", &deadzone)) {
		deadzoneCvar.setValue(deadzone);
	}

	CVarWrapper sizeCvar = cvarManager->getCvar("joystick_viz_clone_size");
	if (!sizeCvar) { return; }
	int size = sizeCvar.getIntValue();
	if (ImGui::SliderInt("Visualization Size", &size, 100, 1000)) {
		sizeCvar.setValue(size);
	}

	ImGui::Text(
		"Self - Check Protips :\n"
		"-Joystick should trace smooth and consistent lines\n"
		"-Jagged / inconsistent traces may indicate controller needs to be repaired / replaced.Try using compressed air to clean out sensor.\n"
		"-Try rotating joystick all the way around edge in a circle\n"
		"-Default deadzone shape and sensitivity should trace a circle\n"
		"-Setting a higher sensitivity with an external tool(such as DS4Win or Steam controller config) will increase size of circle and cut off edges\n"
		"-A squared deadzone(changed in Steam controller config) should instead trace a square\n"
		"-NOTE: In - game sensitivity is NOT reflected in visualization\n"
		"-See plugin homepage for examples and more information\n"
		"Created by AlpacaFlightSim#0001 / @AlpacaFlightSim\n"
	);
}