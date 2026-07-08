/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file townzone_gui.cpp GUI showing per-town zone demand bars (CITYSIM fork feature). */

#include "stdafx.h"
#include "window_gui.h"
#include "window_func.h"
#include "gfx_func.h"
#include "palette_func.h"
#include "strings_func.h"
#include "timer/timer.h"
#include "timer/timer_window.h"
#include "town.h"
#include "zoom_func.h"
#include "townzone.h"

#include "widgets/townzone_widget.h"

#include "table/strings.h"

#include "safeguards.h"

/** One demand bar: zone, label and bar colour. */
struct TownZoneBarInfo {
	TownZone zone;
	StringID label;
	PixelColour colour;
};

static const TownZoneBarInfo _townzone_bars[] = {
	{TownZone::Residential, STR_TOWN_ZONES_RESIDENTIAL, PC_GREEN},
	{TownZone::Commercial,  STR_TOWN_ZONES_COMMERCIAL,  PC_LIGHT_BLUE},
	{TownZone::Industrial,  STR_TOWN_ZONES_INDUSTRIAL,  PC_ORANGE},
};

/** Window showing the R/C/I demand bars of a town. */
struct TownZonesWindow : Window {
	TownZonesWindow(WindowDesc &desc, WindowNumber window_number) : Window(desc)
	{
		this->InitNested(window_number);
	}

	std::string GetWidgetString(WidgetID widget, StringID stringid) const override
	{
		if (widget == WID_TZ_CAPTION) return GetString(STR_TOWN_ZONES_CAPTION, this->window_number);
		return this->Window::GetWidgetString(widget, stringid);
	}

	void UpdateWidgetSize(WidgetID widget, Dimension &size, [[maybe_unused]] const Dimension &padding, [[maybe_unused]] Dimension &fill, [[maybe_unused]] Dimension &resize) override
	{
		if (widget != WID_TZ_BARS) return;

		uint label_width = 0;
		for (const auto &bar : _townzone_bars) {
			label_width = std::max(label_width, GetStringBoundingBox(GetString(bar.label, 100u)).width);
		}
		size.width = std::max(size.width, label_width + ScaleGUITrad(120) + WidgetDimensions::scaled.framerect.Horizontal() + WidgetDimensions::scaled.hsep_wide);
		size.height = std::size(_townzone_bars) * (GetCharacterHeight(FS_NORMAL) + WidgetDimensions::scaled.vsep_normal) + WidgetDimensions::scaled.framerect.Vertical();
	}

	void DrawWidget(const Rect &r, WidgetID widget) const override
	{
		if (widget != WID_TZ_BARS) return;

		const Town *t = Town::GetIfValid(this->window_number);
		if (t == nullptr) return;

		Rect ir = r.Shrink(WidgetDimensions::scaled.framerect);
		int line_height = GetCharacterHeight(FS_NORMAL);
		int y = ir.top;

		uint label_width = 0;
		for (const auto &bar : _townzone_bars) {
			label_width = std::max(label_width, GetStringBoundingBox(GetString(bar.label, 100u)).width);
		}

		for (const auto &bar : _townzone_bars) {
			uint demand = GetTownZoneDemand(t, bar.zone);
			DrawString(ir.left, ir.left + label_width, y, GetString(bar.label, demand));

			/* The bar itself: dark background, coloured fill proportional to demand. */
			int bar_left = ir.left + label_width + WidgetDimensions::scaled.hsep_wide;
			int bar_top = y + 1;
			int bar_bottom = y + line_height - 2;
			GfxFillRect(bar_left, bar_top, ir.right, bar_bottom, PC_BLACK);
			if (demand > 0) {
				int fill = (ir.right - bar_left - 2) * static_cast<int>(demand) / 100;
				GfxFillRect(bar_left + 1, bar_top + 1, bar_left + 1 + fill, bar_bottom - 1, bar.colour);
			}

			y += line_height + WidgetDimensions::scaled.vsep_normal;
		}
	}

	/** Demand changes as the town evolves; refresh regularly. */
	const IntervalTimer<TimerWindow> refresh_interval = {std::chrono::seconds(1), [this](auto) {
		this->SetDirty();
	}};
};

static constexpr std::initializer_list<NWidgetPart> _nested_town_zones_widgets = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, COLOUR_BROWN),
		NWidget(WWT_CAPTION, COLOUR_BROWN, WID_TZ_CAPTION),
		NWidget(WWT_STICKYBOX, COLOUR_BROWN),
	EndContainer(),
	NWidget(WWT_PANEL, COLOUR_BROWN, WID_TZ_BARS), SetToolTip(STR_TOWN_ZONES_BARS_TOOLTIP), EndContainer(),
};

static WindowDesc _town_zones_desc(
	WDP_AUTO, "town_zones", 0, 0,
	WC_TOWN_ZONES, WC_TOWN_VIEW,
	{},
	_nested_town_zones_widgets
);

/**
 * Open the zone demand window of a town.
 * @param town The town to show.
 */
void ShowTownZonesWindow(TownID town)
{
	AllocateWindowDescFront<TownZonesWindow>(_town_zones_desc, town);
}
