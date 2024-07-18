#include "pch.h"
#include "JoystickVisualizationPlugin.h"

BAKKESMOD_PLUGIN(JoystickVisualizationPlugin, "Joystick Visualization", plugin_version, PLUGINTYPE_FREEPLAY);

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void JoystickVisualizationPlugin::onLoad() {
	gameWrapper->HookEventWithCaller<CarWrapper>(
		"Function TAGame.Car_TA.SetVehicleInput",
		[this](CarWrapper cw, void* params, std::string eventname) {
			OnSetInput((ControllerInput*)params);
		}
	);

	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) { Render(canvas); });

	enabled = std::make_shared<bool>(true);
	numberOfPoints = std::make_shared<int>(120);
	boxSize = std::make_shared<int>(400);
	useSensitivity = std::make_shared<bool>(true);
	clampInput = std::make_shared<bool>(true);
	pointPercentage = std::make_shared<float>(0.02);
	centerX = std::make_shared<float>(0.5f);
	centerY = std::make_shared<float>(0.5f);
	fillBox = std::make_shared<bool>(false);
	boxColor = std::make_shared<LinearColor>();
	pointColor = std::make_shared<LinearColor>();
	pointColorDeadzone = std::make_shared<LinearColor>();

	cvarManager->registerCvar(JOYSTICK_VIS_ENABLED, "1", "Show Joystick Visualization", true, true, 0, true, 1)
		.bindTo(enabled);
	cvarManager->registerCvar(JOYSTICK_VIS_POINT_COUNT, "120", "Number of Inputs to Visualize", true, true, 1)
		.addOnValueChanged(
			[this](std::string oldValue, CVarWrapper cvar) {
				int newNumberOfPoints = cvar.getIntValue();
				*numberOfPoints = newNumberOfPoints;
				inputHistory.reserve(newNumberOfPoints);
			}
		);
	cvarManager->registerCvar(JOYSTICK_VIS_SIZE, "400", "Joystick Visualization Size", true, true, 10)
		.bindTo(boxSize);
	cvarManager->registerCvar(JOYSTICK_VIS_SENSITIVITY, "0", "Scale Input with Aerial Sensitivity", true, true, 0, true, 1)
		.bindTo(useSensitivity);
	cvarManager->registerCvar(JOYSTICK_VIS_CLAMP, "1", "Clamp Values to Max Input", true, true, 0, true, 1)
		.bindTo(clampInput);
	cvarManager->registerCvar(JOYSTICK_VIS_POINT_SIZE, "0.015", "Size of Points Relative to Box", true, true, 0, true, 1)
		.bindTo(pointPercentage);
	cvarManager->registerCvar(JOYSTICK_VIS_CENTER_X, "0.5", "Center of the Visualization - X", true, true, 0, true, 1)
		.bindTo(centerX);
	cvarManager->registerCvar(JOYSTICK_VIS_CENTER_Y, "0.5", "Center of the Visualization - Y", true, true, 0, true, 1)
		.bindTo(centerY);
	cvarManager->registerCvar(JOYSTICK_VIS_FILL_BOX, "0", "Fill Box", true, true, 0, true, 1)
		.bindTo(fillBox);
	cvarManager->registerCvar(JOYSTICK_VIS_COLOR_BOX, "#FFFFFF64", "Box Color")
		.bindTo(boxColor);
	cvarManager->registerCvar(JOYSTICK_VIS_COLOR_POINT, "#FFFFFF", "Point Color")
		.bindTo(pointColor);
	cvarManager->registerCvar(JOYSTICK_VIS_COLOR_DEADZONE, "#FFFFFF", "Point Color in Deadzone")
		.bindTo(pointColorDeadzone);

	inputHistory.reserve(*numberOfPoints);
}

void JoystickVisualizationPlugin::onUnload() {
	gameWrapper->UnregisterDrawables();
}

bool JoystickVisualizationPlugin::isActive() {
	return (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining())
		&& !gameWrapper->IsPaused()
		&& *enabled;
}

void JoystickVisualizationPlugin::OnSetInput(ControllerInput* ci) {
	if (!isActive()) {
		return;
	}

	if (inputHistory.size() >= *numberOfPoints) {
		int numToRemove = 1 + inputHistory.size() - *numberOfPoints;
		inputHistory.erase(inputHistory.begin(), inputHistory.begin() + numToRemove);
	}

	inputHistory.push_back(*ci);
}

void JoystickVisualizationPlugin::Render(CanvasWrapper canvas) {
	if (!isActive()) {
		return;
	}

	float pointSize = (*boxSize) * (*pointPercentage);
	float boxSizeWithPadding = (*boxSize) + pointSize;

	Vector2 canvasSize = canvas.GetSize();
	Vector2F canvasCenter = Vector2F(
		std::lerp(0, canvasSize.X - boxSizeWithPadding, *centerX),
		std::lerp(0, canvasSize.Y - boxSizeWithPadding, *centerY)
	) + (boxSizeWithPadding / 2.0f);

	canvas.SetColor(*boxColor);
	canvas.SetPosition(Vector2F(canvasCenter.X - boxSizeWithPadding / 2.0f, canvasCenter.Y - boxSizeWithPadding / 2.0f));
	Vector2F boxDimensions = Vector2F(boxSizeWithPadding, boxSizeWithPadding);
	if (*fillBox) { canvas.FillBox(boxDimensions); }
	else { canvas.DrawBox(boxDimensions); }

	float scale = *useSensitivity
		? gameWrapper->GetSettings().GetGamepadSettings().AirControlSensitivity
		: 1;

	int i = 0;
	Vector2F prevPos;

	for (auto iter = inputHistory.rbegin(); iter != inputHistory.rend(); ++iter)
	{
		ControllerInput controllerInput = *iter;

		Vector2F rawInput = Vector2F(controllerInput.Steer, controllerInput.Pitch) * scale;
		Vector2F clampedInput = Vector2F(std::clamp(rawInput.X, -1.0f, 1.0f), std::clamp(rawInput.Y, -1.0f, 1.0f));
		Vector2F input = *clampInput ? clampedInput : rawInput;
		Vector2F currentPos = canvasCenter + input * (*boxSize) / 2.0f;

		float opacity = 1 - i / (float)(*numberOfPoints);
		bool isInDeadzone = input.X == 0.0f || input.Y == 0.0f;
		LinearColor color = isInDeadzone ? *pointColorDeadzone : *pointColor;
		color.A *= opacity;
		canvas.SetColor(color);
		Vector2F offset = Vector2F(pointSize / 2.0, pointSize / 2.0);
		canvas.SetPosition(currentPos - offset);
		canvas.FillBox(Vector2F(pointSize, pointSize));

		if (i > 0) {
			canvas.DrawLine(currentPos, prevPos);
		}

		prevPos = currentPos;
		i++;
	}
}
