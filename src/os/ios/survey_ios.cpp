/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file survey_ios.cpp iOS implementation of OS-specific survey information. */

#include "../../stdafx.h"

#include "../../3rdparty/fmt/format.h"
#include "../../survey.h"
#include "../macosx/macos.h"

#include <sys/utsname.h>
#include <thread>

#include "../../safeguards.h"

void SurveyOS(nlohmann::json &json)
{
	auto [ver_maj, ver_min, ver_bug] = GetMacOSVersion();

	struct utsname uts{};
	uname(&uts);

	json["os"] = "iOS";
	json["release"] = fmt::format("{}.{}.{}", ver_maj, ver_min, ver_bug);
	json["machine"] = uts.machine;

	json["memory"] = SurveyMemoryToText(MacOSGetPhysicalMemory());
	json["hardware_concurrency"] = std::thread::hardware_concurrency();
}
