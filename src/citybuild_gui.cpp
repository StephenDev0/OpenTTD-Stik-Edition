/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file citybuild_gui.cpp GUI for player-driven city building (CITYSIM fork feature). */

#include "stdafx.h"
#include "window_gui.h"
#include "window_func.h"
#include "command_func.h"
#include "company_base.h"
#include "company_func.h"
#include "strings_func.h"
#include "hotkeys.h"
#include "gui.h"
#include "tilehighlight_func.h"
#include "citybuild_cmd.h"
#include "road_cmd.h"
#include "townzone.h"

#include "widgets/citybuild_widget.h"

#include "table/sprites.h"
#include "table/strings.h"

#include "safeguards.h"

/** Toolbar window for player-driven city building. */
struct CityBuildToolbarWindow : Window {
	WidgetID last_clicked_widget = INVALID_WIDGET; ///< Placement button that armed the current tool.

	CityBuildToolbarWindow(WindowDesc &desc, WindowNumber window_number) : Window(desc)
	{
		this->InitNested(window_number);
	}

	void OnClick([[maybe_unused]] Point pt, WidgetID widget, [[maybe_unused]] int click_count) override
	{
		switch (widget) {
			case WID_CBT_PLACE_HOUSE:
			case WID_CBT_PLACE_COMMERCIAL:
				if (HandlePlacePushButton(this, widget, SPR_CURSOR_TOWN, HT_RECT)) this->last_clicked_widget = widget;
				break;

			case WID_CBT_FUND_INDUSTRY:
				ShowBuildIndustryWindow();
				break;

			default: break;
		}
	}

	void OnPlaceObject([[maybe_unused]] Point pt, TileIndex tile) override
	{
		TownZone zone = (this->last_clicked_widget == WID_CBT_PLACE_COMMERCIAL) ? TownZone::Commercial : TownZone::Residential;
		Command<CMD_PLACE_PLAYER_HOUSE>::Post(STR_ERROR_CAN_T_BUILD_HOUSE, CcPlaySound_CONSTRUCTION_OTHER, tile, zone);
	}

	void OnPlaceObjectAbort() override
	{
		this->RaiseButtons();
	}

	void UpdateWidgetSize(WidgetID widget, Dimension &size, [[maybe_unused]] const Dimension &padding, [[maybe_unused]] Dimension &fill, [[maybe_unused]] Dimension &resize) override
	{
		if (widget != WID_CBT_STATUS) return;

		size.width = std::max(size.width, GetStringBoundingBox(GetString(STR_CITYBUILD_TOOLBAR_STATUS, TOWNZONE_PLACE_THRESHOLD, GetTownZoneBuildCost(TownZone::Residential), GetTownZoneBuildCost(TownZone::Commercial))).width + WidgetDimensions::scaled.framerect.Horizontal());
		size.height = GetCharacterHeight(FS_NORMAL) + WidgetDimensions::scaled.framerect.Vertical();
	}

	void DrawWidget(const Rect &r, WidgetID widget) const override
	{
		if (widget != WID_CBT_STATUS) return;

		DrawString(r.Shrink(WidgetDimensions::scaled.framerect), GetString(STR_CITYBUILD_TOOLBAR_STATUS, TOWNZONE_PLACE_THRESHOLD, GetTownZoneBuildCost(TownZone::Residential), GetTownZoneBuildCost(TownZone::Commercial)));
	}

	/**
	 * Handler for global hotkeys of the CityBuildToolbarWindow.
	 * @param hotkey Hotkey
	 * @return ES_HANDLED if hotkey was accepted.
	 */
	static EventState CityBuildToolbarGlobalHotkeys(int hotkey)
	{
		if (_game_mode != GM_NORMAL) return ES_NOT_HANDLED;
		Window *w = ShowCityBuildToolbar();
		if (w == nullptr) return ES_NOT_HANDLED;
		return w->OnHotkey(hotkey);
	}

	static inline HotkeyList hotkeys{"citybuildtoolbar", {
		Hotkey('1', "place_house", WID_CBT_PLACE_HOUSE),
		Hotkey('2', "place_commercial", WID_CBT_PLACE_COMMERCIAL),
		Hotkey('3', "fund_industry", WID_CBT_FUND_INDUSTRY),
	}, CityBuildToolbarGlobalHotkeys};
};

/** Nested widget parts of the city building toolbar. */
static constexpr std::initializer_list<NWidgetPart> _nested_citybuild_toolbar_widgets = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, COLOUR_DARK_GREEN),
		NWidget(WWT_CAPTION, COLOUR_DARK_GREEN), SetStringTip(STR_CITYBUILD_TOOLBAR_CAPTION, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_STICKYBOX, COLOUR_DARK_GREEN),
	EndContainer(),
	NWidget(NWID_HORIZONTAL_LTR),
		NWidget(WWT_IMGBTN, COLOUR_DARK_GREEN, WID_CBT_PLACE_HOUSE), SetToolbarMinimalSize(1), SetFill(0, 1), SetSpriteTip(SPR_IMG_TOWN, STR_CITYBUILD_TOOLBAR_PLACE_HOUSE_TOOLTIP),
		NWidget(WWT_IMGBTN, COLOUR_DARK_GREEN, WID_CBT_PLACE_COMMERCIAL), SetToolbarMinimalSize(1), SetFill(0, 1), SetSpriteTip(SPR_IMG_COMPANY_GENERAL, STR_CITYBUILD_TOOLBAR_PLACE_COMMERCIAL_TOOLTIP),
		NWidget(WWT_IMGBTN, COLOUR_DARK_GREEN, WID_CBT_FUND_INDUSTRY), SetToolbarMinimalSize(1), SetFill(0, 1), SetSpriteTip(SPR_IMG_INDUSTRY, STR_CITYBUILD_TOOLBAR_FUND_INDUSTRY_TOOLTIP),
	EndContainer(),
	NWidget(WWT_PANEL, COLOUR_DARK_GREEN, WID_CBT_STATUS), EndContainer(),
};

static WindowDesc _citybuild_toolbar_desc(
	WDP_ALIGN_TOOLBAR, "toolbar_citybuild", 0, 0,
	WC_CITY_BUILD_TOOLBAR, WC_NONE,
	WindowDefaultFlag::Construction,
	_nested_citybuild_toolbar_widgets,
	&CityBuildToolbarWindow::hotkeys
);

/**
 * Open the city building toolbar window.
 * @return newly opened city building toolbar, or nullptr if the toolbar could not be opened.
 */
Window *ShowCityBuildToolbar()
{
	if (!Company::IsValidID(_local_company)) return nullptr;

	CloseWindowByClass(WC_CITY_BUILD_TOOLBAR);
	return AllocateWindowDescFront<CityBuildToolbarWindow>(_citybuild_toolbar_desc, 0);
}
