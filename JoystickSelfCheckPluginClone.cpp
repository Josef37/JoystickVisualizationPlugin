#include "pch.h"
#include "JoystickSelfCheckPluginClone.h"

BAKKESMOD_PLUGIN(JoystickSelfCheckPluginClone, "Cool Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

int input_history_length = 120;

void JoystickSelfCheckPluginClone::onLoad()
{
	enabled = std::make_shared<bool>(false);
	joystickVizSize = std::make_shared<int>(400);

	inputHistory.reserve(input_history_length);

	cvarManager->log("JoystickSelfCheckPlugin loaded");

	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput", std::bind(&JoystickSelfCheckPluginClone::OnSetInput, this, std::placeholders::_1, std::placeholders::_2));

	gameWrapper->RegisterDrawable(std::bind(&JoystickSelfCheckPluginClone::Render, this, std::placeholders::_1));

	cvarManager->registerCvar("joystick_viz_clone_enabled", "0", "Show Joystick Self-Check Visualization", true, true, 0.f, true, 1.f)
		.bindTo(enabled);
	cvarManager->registerCvar("joystick_viz_clone_size", "400", "Joystick Visualization Size", true, true, 100, true, 1000)
		.bindTo(joystickVizSize);
}

void JoystickSelfCheckPluginClone::onUnload() {
	cvarManager->log("JoystickSelfCheckPlugin unloaded");
	gameWrapper->UnregisterDrawables();
}

bool JoystickSelfCheckPluginClone::isActive() {
	return (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining()) 
		&& !gameWrapper->IsPaused() 
		&& cvarManager->getCvar("joystick_viz_clone_enabled").getBoolValue();
}

void JoystickSelfCheckPluginClone::OnSetInput(CarWrapper cw, void* params)
{
	if (!isActive()) {
		return;
	}

	ControllerInput* ci = (ControllerInput*)params;

	if (cvarManager->getCvar("joystick_viz_clone_enabled").getBoolValue()) {
		if (inputHistory.size() >= input_history_length) {
			int numToRemove = 1 + inputHistory.size() - input_history_length;
			inputHistory.erase(inputHistory.begin(), inputHistory.begin() + numToRemove);
		}

		inputHistory.push_back(*ci);
	}
}

void JoystickSelfCheckPluginClone::Render(CanvasWrapper canvas)
{
	if (!isActive()) {
		return;
	}

	Vector2 canvasSize = canvas.GetSize();
	Vector2 canvasCenter = { canvasSize.X / 2, canvasSize.Y / 2 };

	canvas.SetColor(255, 255, 255, 100);
	canvas.SetPosition(Vector2({ canvasCenter.X - *joystickVizSize / 2, canvasCenter.Y - *joystickVizSize / 2 }));
	canvas.DrawBox(Vector2({ *joystickVizSize, *joystickVizSize }));

	int i = 0;
	Vector2 prevPos;

	for (auto it = inputHistory.end(); it != inputHistory.begin(); --it)
	{
		ControllerInput input = *it;

		Vector2 currentPos = { canvasCenter.X + (int)(*joystickVizSize / 2 * input.Steer) - 3, canvasCenter.Y + (int)(*joystickVizSize / 2 * input.Pitch) - 3 };

		canvas.SetColor(255, 255, 255, 255 - i * 2);
		canvas.SetPosition(currentPos);
		canvas.FillBox(Vector2({ 5, 5 }));

		if (i > 0) {
			ControllerInput prevInput = inputHistory[inputHistory.size() - i - 1];
			Vector2 prevPos = { canvasCenter.X + (int)(*joystickVizSize / 2 * prevInput.Steer), canvasCenter.Y + (int)(*joystickVizSize / 2 * prevInput.Pitch) };

			canvas.DrawLine({ canvasCenter.X + (int)(*joystickVizSize / 2 * input.Steer), canvasCenter.Y + (int)(*joystickVizSize / 2 * input.Pitch) }, prevPos);
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
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Show Joystick Self - Check Visualization");
	}

	CVarWrapper sizeCvar = cvarManager->getCvar("joystick_viz_clone_size");
	if (!sizeCvar) { return; }
	int size = sizeCvar.getIntValue();
	if (ImGui::SliderInt("Visualization Size", &size, 100, 1000)) {
		sizeCvar.setValue(size);
	}
	if (ImGui::IsItemHovered()) {
		std::string hoverText = "size is " + std::to_string(size);
		ImGui::SetTooltip(hoverText.c_str());
	}

	ImGui::Text(
		"Self - Check Protips :"
		"-Joystick should trace smooth and consistent lines"
		"-Jagged / inconsistent traces may indicate controller needs to be repaired / replaced.Try using compressed air to clean out sensor."
		"-Try rotating joystick all the way around edge in a circle"
		"-Default deadzone shape and sensitivity should trace a circle"
		"-Setting a higher sensitivity with an external tool(such as DS4Win or Steam controller config) will increase size of circle and cut off edges"
		"-A squared deadzone(changed in Steam controller config) should instead trace a square"
		"-NOTE: In - game sensitivity is NOT reflected in visualization"
		"-See plugin homepage for examples and more information"
		"Created by AlpacaFlightSim#0001 / @AlpacaFlightSim"
	);
}