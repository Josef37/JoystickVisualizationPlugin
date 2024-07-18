#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "GuiBase.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

constexpr auto JOYSTICK_VIS_ENABLED = "joystick_vis_enabled";
constexpr auto JOYSTICK_VIS_POINT_COUNT = "joystick_vis_point_count";
constexpr auto JOYSTICK_VIS_SIZE = "joystick_vis_size";
constexpr auto JOYSTICK_VIS_SENSITIVITY = "joystick_vis_sensitivity";
constexpr auto JOYSTICK_VIS_CLAMP = "joystick_vis_clamp";
constexpr auto JOYSTICK_VIS_POINT_SIZE = "joystick_vis_point_size";
constexpr auto JOYSTICK_VIS_CENTER_X = "joystick_vis_center_x";
constexpr auto JOYSTICK_VIS_CENTER_Y = "joystick_vis_center_y";
constexpr auto JOYSTICK_VIS_FILL_BOX = "joystick_vis_fill_box";
constexpr auto JOYSTICK_VIS_COLOR_BOX = "joystick_vis_color_box";
constexpr auto JOYSTICK_VIS_COLOR_POINT = "joystick_vis_color_point";
constexpr auto JOYSTICK_VIS_COLOR_DEADZONE = "joystick_vis_color_deadzone";

class JoystickVisualizationPlugin : public BakkesMod::Plugin::BakkesModPlugin, public SettingsWindowBase
{
	std::vector<ControllerInput> inputHistory;

	std::shared_ptr<bool> enabled;
	std::shared_ptr<int> numberOfPoints;
	std::shared_ptr<int> boxSize;
	std::shared_ptr<bool> useSensitivity;
	std::shared_ptr<bool> clampInput;
	std::shared_ptr<float> pointPercentage;
	std::shared_ptr<float> centerX;
	std::shared_ptr<float> centerY;
	std::shared_ptr<bool> fillBox;
	std::shared_ptr<LinearColor> boxColor;
	std::shared_ptr<LinearColor> pointColor;
	std::shared_ptr<LinearColor> pointColorDeadzone;

	void onLoad() override;
	void onUnload() override;
	void OnSetInput(ControllerInput* ci);
	void Render(CanvasWrapper canvas);
	void RenderSettings() override;
	bool isActive();
};