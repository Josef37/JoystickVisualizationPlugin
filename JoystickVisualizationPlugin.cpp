#include "pch.h"
#include "JoystickVisualizationPlugin.h"

BAKKESMOD_PLUGIN(JoystickVisualizationPlugin, "Joystick Visualization", plugin_version, PLUGINTYPE_FREEPLAY);

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

bool DoubleJumped;
bool IsDodging;

void JoystickVisualizationPlugin::onLoad() {
	// This line is required for `LOG()` to work and must be before any use of `LOG()`.
	_globalCvarManager = cvarManager;

	// Record input and determine if it caused a dodge or double jump.
	// That's why we also hook into the "Post" event.
	gameWrapper->HookEventWithCaller<CarWrapper>(
		"Function TAGame.Car_TA.SetVehicleInput",
		[this](CarWrapper car, void* params, std::string eventName) {
			OnSetInput(car, (ControllerInput*)params);
		}
	);
	gameWrapper->HookEventWithCallerPost<CarWrapper>(
		"Function TAGame.Car_TA.SetVehicleInput",
		[this](CarWrapper car, void* params, std::string eventName) {
			OnSetInputPost(car, (ControllerInput*)params);
		}
	);

	gameWrapper->RegisterDrawable(
		[this](CanvasWrapper canvas) {
			Render(canvas);
		}
	);

	persistentStorage = std::make_shared<PersistentStorage>(this, "joystick_vis", true, true);

	enabled = std::make_shared<bool>(true);
	numberOfPoints = std::make_shared<int>(120);
	boxSize = std::make_shared<int>(400);
	useSensitivity = std::make_shared<bool>(true);
	clampInput = std::make_shared<bool>(true);
	pointPercentage = std::make_shared<float>(0.015);
	pointJumpPercentage = std::make_shared<float>(0.05);
	centerX = std::make_shared<float>(0.5f);
	centerY = std::make_shared<float>(0.5f);
	fillBox = std::make_shared<bool>(false);
	boxColor = std::make_shared<LinearColor>();
	pointColor = std::make_shared<LinearColor>();
	pointDeadzoneColor = std::make_shared<LinearColor>();
	pointJumpColor = std::make_shared<LinearColor>();

	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_ENABLED, "1", "Show Joystick Visualization", true, true, 0, true, 1)
		.bindTo(enabled);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_POINT_COUNT, "120", "Number of Inputs to Visualize", true, true, 1)
		.addOnValueChanged(
			[this](std::string oldValue, CVarWrapper cvar) {
				int newNumberOfPoints = cvar.getIntValue();
				*numberOfPoints = newNumberOfPoints;
				inputHistory.reserve(newNumberOfPoints);
			}
		);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_SIZE, "400", "Joystick Visualization Size", true, true, 10)
		.bindTo(boxSize);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_SENSITIVITY, "0", "Scale Input with Aerial Sensitivity", true, true, 0, true, 1)
		.bindTo(useSensitivity);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_CLAMP, "1", "Clamp Values to Max Input", true, true, 0, true, 1)
		.bindTo(clampInput);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_POINT_SIZE, "0.015", "Size of Points Relative to Box", true, true, 0, true, 1)
		.bindTo(pointPercentage);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_JUMP_SIZE, "0.05", "Size of Points for Flips Relative to Box", true, true, 0, true, 1)
		.bindTo(pointJumpPercentage);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_CENTER_X, "0.5", "Center of the Visualization - X", true, true, 0, true, 1)
		.bindTo(centerX);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_CENTER_Y, "0.5", "Center of the Visualization - Y", true, true, 0, true, 1)
		.bindTo(centerY);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_FILL_BOX, "0", "Fill Box", true, true, 0, true, 1)
		.bindTo(fillBox);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_COLOR_BOX, "#FFFFFF64", "Box Color")
		.bindTo(boxColor);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_COLOR_POINT, "#FFFFFF", "Point Color")
		.bindTo(pointColor);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_COLOR_DEADZONE, "#AAFFAA", "Point Color in Deadzone")
		.bindTo(pointDeadzoneColor);
	persistentStorage->RegisterPersistentCvar(JOYSTICK_VIS_COLOR_JUMP, "#FF0000", "Point Color for Flips")
		.bindTo(pointJumpColor);

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

bool JoystickVisualizationPlugin::isLocalCar(CarWrapper& car) {
	auto localCar = gameWrapper->GetLocalCar();
	if (!localCar) return false;
	return car.memory_address == localCar.memory_address;
}

void JoystickVisualizationPlugin::OnSetInput(CarWrapper& car, ControllerInput* input) {
	if (!isActive() || !isLocalCar(car)) {
		return;
	}

	// `ControllerInput->Jumped` is only true when the jump button is initially pressed.
	// We record the values for double jump and dodging before the processing takes places to compare them later.
	if (input->Jumped) {
		DoubleJumped = car.GetbDoubleJumped();
		IsDodging = car.IsDodging();
	}
}

void JoystickVisualizationPlugin::OnSetInputPost(CarWrapper& car, ControllerInput* input) {
	if (!isActive() || !isLocalCar(car)) {
		return;
	}

	// We compare the values for the same input event before and after processing.
	// This way we can determine if the current jump input causes a double jump or dodge.
	// `ControllerInput->Jumped` is only true when the jump button is initially pressed.
	JumpType jumpType = !input->Jumped ? JumpType::Other
		: !IsDodging && car.IsDodging() ? JumpType::Dodge
		: !DoubleJumped && car.GetbDoubleJumped() ? JumpType::Double
		: JumpType::Other;

	if (inputHistory.size() >= *numberOfPoints) {
		int numToRemove = 1 + inputHistory.size() - *numberOfPoints;
		inputHistory.erase(inputHistory.begin(), inputHistory.begin() + numToRemove);
	}

	inputHistory.push_back({ jumpType, *input });
}

void JoystickVisualizationPlugin::Render(CanvasWrapper& canvas) {
	if (!isActive()) {
		return;
	}

	float boxSizeWithPadding = *boxSize * (1 + *pointPercentage);

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
		ControllerInput controllerInput = iter->controllerInput;
		JumpType jumpType = iter->jumpType;

		Vector2F rawInput = Vector2F(controllerInput.Steer, controllerInput.Pitch) * scale;
		Vector2F clampedInput = Vector2F(std::clamp(rawInput.X, -1.0f, 1.0f), std::clamp(rawInput.Y, -1.0f, 1.0f));
		Vector2F input = *clampInput ? clampedInput : rawInput;
		bool jumped = jumpType == JumpType::Dodge || jumpType == JumpType::Double;
		Vector2F currentPos = canvasCenter + input * (*boxSize) / 2.0f;

		float opacity = 1 - i / (float)(*numberOfPoints);
		bool isInDeadzone = input.X == 0.0f || input.Y == 0.0f;
		LinearColor colorByPosition = isInDeadzone ? *pointDeadzoneColor : *pointColor;
		LinearColor color = jumped ? *pointJumpColor : colorByPosition;
		color.A *= opacity;
		canvas.SetColor(color);
		float pointSize = *boxSize * (jumped ? *pointJumpPercentage : *pointPercentage);
		Vector2F offset = Vector2F(pointSize / 2, pointSize / 2);
		canvas.SetPosition(currentPos - offset);
		canvas.FillBox(Vector2F(pointSize, pointSize));

		if (i > 0) {
			canvas.DrawLine(currentPos, prevPos);
		}

		prevPos = currentPos;
		i++;
	}
}
