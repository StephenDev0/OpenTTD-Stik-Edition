/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture_gui.cpp GUI for venture capital investments (CITYSIM fork feature). */

#include "stdafx.h"
#include "window_gui.h"
#include "window_func.h"
#include "command_func.h"
#include "company_base.h"
#include "company_func.h"
#include "core/geometry_func.hpp"
#include "gfx_func.h"
#include "palette_func.h"
#include "strings_func.h"
#include "venture.h"
#include "venture_cmd.h"

#include "widgets/venture_widget.h"

#include "table/strings.h"

#include "safeguards.h"

/** Window listing the investable startups. */
struct VenturesWindow : Window {
	int selected = -1; ///< Selected startup slot, or -1.

	VenturesWindow(WindowDesc &desc, WindowNumber window_number) : Window(desc)
	{
		this->InitNested(window_number);
	}

	/** Build the display line for one startup slot. */
	std::string GetVentureRowString(const Venture &v) const
	{
		uint16_t own_bp = Company::IsValidID(_local_company) ? v.stakes[_local_company.base()] : 0;
		return GetString(STR_VENTURE_ROW, GetVentureNameString(v), GetVentureStatusString(v), v.valuation, own_bp / 100);
	}

	void UpdateWidgetSize(WidgetID widget, Dimension &size, [[maybe_unused]] const Dimension &padding, [[maybe_unused]] Dimension &fill, [[maybe_unused]] Dimension &resize) override
	{
		if (widget == WID_VC_DETAILS) {
			size.width = std::max(size.width, GetStringBoundingBox(GetString(STR_VENTURE_DETAIL_ACTIVE, STR_VENTURE_NAME_FIRST, uint16_t{100}, Money{100000}, Money{100000}, Money{100000}, uint8_t{1}, uint8_t{2})).width + WidgetDimensions::scaled.framerect.Horizontal());
			size.height = GetCharacterHeight(FontSize::Normal) * 3 + WidgetDimensions::scaled.framerect.Vertical();
			return;
		}
		if (widget != WID_VC_LIST) return;

		Dimension d = {0, 0};
		for (const Venture &v : _ventures) {
			d = maxdim(d, GetStringBoundingBox(this->GetVentureRowString(v)));
		}
		size.width = std::max(size.width, d.width + WidgetDimensions::scaled.framerect.Horizontal());
	size.height = NUM_VENTURES * GetCharacterHeight(FontSize::Normal) + WidgetDimensions::scaled.framerect.Vertical();
	}

	void DrawWidget(const Rect &r, WidgetID widget) const override
	{
		if (widget == WID_VC_DETAILS) {
			Rect ir = r.Shrink(WidgetDimensions::scaled.framerect);
			if (this->selected < 0 || !Company::IsValidID(_local_company)) {
				DrawString(ir, STR_VENTURE_DETAIL_NONE);
				return;
			}

			const Venture &v = _ventures[this->selected];
			uint16_t own_bp = v.stakes[_local_company.base()];
			Money stake_value = GetVentureStakeValue(v, own_bp);
			Money sell_value = v.state == VentureState::Active ? GetVentureSellValue(v, own_bp) : Money{0};
			Money trade_sell_value = v.state == VentureState::Active ? GetVentureSellValue(v, VENTURE_TRADE_UNIT_BP) : Money{0};
			Money cost_basis = v.cost_basis[_local_company.base()];
			DrawString(ir.left, ir.right, ir.top, GetString(STR_VENTURE_DETAIL_ACTIVE, GetVentureNameString(v), GetTotalVentureStakeBp(v) / 100, stake_value, sell_value, cost_basis, v.quarters_in_stage, uint8_t{2}));
		ir.top += GetCharacterHeight(FontSize::Normal);
			DrawString(ir.left, ir.right, ir.top, GetString(STR_VENTURE_DETAIL_LIMITS, VENTURE_MAX_COMPANY_STAKE_BP / 100, VENTURE_MAX_TOTAL_STAKE_BP / 100, VENTURE_SELL_PCT));
		ir.top += GetCharacterHeight(FontSize::Normal);
			DrawString(ir.left, ir.right, ir.top, GetString(STR_VENTURE_DETAIL_TRADE, GetVentureStakeValue(v, VENTURE_TRADE_UNIT_BP), trade_sell_value));
			return;
		}
		if (widget != WID_VC_LIST) return;

		Rect ir = r.Shrink(WidgetDimensions::scaled.framerect);
		int y = ir.top;
		for (uint slot = 0; slot < NUM_VENTURES; slot++) {
			const Venture &v = _ventures[slot];
			if (static_cast<int>(slot) == this->selected) {
				GfxFillRect(ir.left, y, ir.right, y + GetCharacterHeight(FontSize::Normal) - 1, PC_DARK_GREY);
			}
			DrawString(ir.left, ir.right, y, this->GetVentureRowString(v));
		y += GetCharacterHeight(FontSize::Normal);
		}
	}

	void OnPaint() override
	{
		bool can_trade = this->selected >= 0 && Company::IsValidID(_local_company) &&
				_ventures[this->selected].state == VentureState::Active;
		this->SetWidgetDisabledState(WID_VC_BUY, !can_trade);
		this->SetWidgetDisabledState(WID_VC_SELL, !can_trade ||
				_ventures[this->selected].stakes[_local_company.base()] == 0);
		this->DrawWidgets();
	}

	void OnClick([[maybe_unused]] Point pt, WidgetID widget, [[maybe_unused]] int click_count) override
	{
		switch (widget) {
			case WID_VC_LIST: {
				const NWidgetBase *wid = this->GetWidget<NWidgetBase>(WID_VC_LIST);
		int row = (pt.y - wid->pos_y - WidgetDimensions::scaled.framerect.top) / GetCharacterHeight(FontSize::Normal);
				this->selected = (row >= 0 && row < static_cast<int>(NUM_VENTURES)) ? row : -1;
				this->SetDirty();
				break;
			}

			case WID_VC_BUY:
				if (this->selected >= 0) {
					Command<Commands::BuyVentureStake>::Post(STR_ERROR_CAN_T_BUY_STAKE,
							static_cast<uint8_t>(this->selected),
							static_cast<uint16_t>(_ctrl_pressed ? 10 * VENTURE_TRADE_UNIT_BP : VENTURE_TRADE_UNIT_BP));
				}
				break;

			case WID_VC_SELL:
				if (this->selected >= 0) {
					Command<Commands::SellVentureStake>::Post(STR_ERROR_CAN_T_SELL_STAKE,
							static_cast<uint8_t>(this->selected),
							static_cast<uint16_t>(_ctrl_pressed ? 10 * VENTURE_TRADE_UNIT_BP : VENTURE_TRADE_UNIT_BP));
				}
				break;
		}
	}
};

static constexpr std::initializer_list<NWidgetPart> _nested_ventures_widgets = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, Colours::Brown),
		NWidget(WWT_CAPTION, Colours::Brown), SetStringTip(STR_VENTURE_CAPTION, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_SHADEBOX, Colours::Brown),
		NWidget(WWT_STICKYBOX, Colours::Brown),
	EndContainer(),
	NWidget(WWT_PANEL, Colours::Brown, WID_VC_LIST), SetToolTip(STR_VENTURE_LIST_TOOLTIP), EndContainer(),
	NWidget(WWT_PANEL, Colours::Brown, WID_VC_DETAILS), EndContainer(),
	NWidget(NWID_HORIZONTAL, NWidContainerFlag::EqualSize),
		NWidget(WWT_PUSHTXTBTN, Colours::Brown, WID_VC_BUY), SetFill(1, 0), SetStringTip(STR_VENTURE_BUY_BUTTON, STR_VENTURE_BUY_TOOLTIP),
		NWidget(WWT_PUSHTXTBTN, Colours::Brown, WID_VC_SELL), SetFill(1, 0), SetStringTip(STR_VENTURE_SELL_BUTTON, STR_VENTURE_SELL_TOOLTIP),
	EndContainer(),
};

static WindowDesc _ventures_desc(
	WindowPosition::Automatic, "ventures", 0, 0,
	WindowClass::VentureCapital, WindowClass::None,
	{},
	_nested_ventures_widgets
);

/** Open the venture capital window. */
void ShowVenturesWindow()
{
	AllocateWindowDescFront<VenturesWindow>(_ventures_desc, 0);
}
