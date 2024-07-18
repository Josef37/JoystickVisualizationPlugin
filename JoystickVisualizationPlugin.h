#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "GuiBase.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class JoystickVisualizationPlugin : public BakkesMod::Plugin::BakkesModPlugin,
	public SettingsWindowBase
{
	std::vector<ControllerInput> inputHistory;
	std::shared_ptr<int> boxSize;
	std::shared_ptr<bool> enabled;
	std::shared_ptr<bool> useSensitivity;
	std::shared_ptr<bool> clampInput;
	std::shared_ptr<float> pointPercentage;
	std::shared_ptr<float> centerX;
	std::shared_ptr<float> centerY;
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