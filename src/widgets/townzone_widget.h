/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file townzone_widget.h Types related to the town zones widgets (CITYSIM fork feature). */

#ifndef WIDGETS_TOWNZONE_WIDGET_H
#define WIDGETS_TOWNZONE_WIDGET_H

/** Widgets of the #TownZonesWindow class. */
enum TownZoneWidgets : WidgetID {
	WID_TZ_CAPTION, ///< Caption showing the town name.
	WID_TZ_BARS,    ///< Panel with the three demand bars.
};

#endif /* WIDGETS_TOWNZONE_WIDGET_H */
