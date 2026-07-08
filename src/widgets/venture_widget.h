/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file venture_widget.h Types related to the venture capital widgets (CITYSIM fork feature). */

#ifndef WIDGETS_VENTURE_WIDGET_H
#define WIDGETS_VENTURE_WIDGET_H

/** Widgets of the #VenturesWindow class. */
enum VentureWidgets : WidgetID {
	WID_VC_LIST, ///< List of startups.
	WID_VC_DETAILS, ///< Details of the selected startup.
	WID_VC_BUY,  ///< Buy stake in the selected startup.
	WID_VC_SELL, ///< Sell stake in the selected startup.
};

#endif /* WIDGETS_VENTURE_WIDGET_H */
