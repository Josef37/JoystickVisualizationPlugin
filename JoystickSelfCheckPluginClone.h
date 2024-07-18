#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "GuiBase.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class JoystickSelfCheckPluginClone : public BakkesMod::Plugin::BakkesModPlugin,
	public SettingsWindowBase
{
	std::vector<ControllerInput> inputHistory;
	std::shared_ptr<int> joystickVizSize;
	std::shared_ptr<bool> enabled;
	std::shared_ptr<bool> highlightDeadzone;

	void onLoad() override;
	void onUnload() override;
	void OnSetInput(CarWrapper cw, void* params);
	void Render(CanvasWrapper canvas);
	void RenderSettings() override;
	bool isActive();
};