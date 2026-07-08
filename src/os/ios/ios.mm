/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file ios.mm Code related to iOS. Provides the same OS hooks as os/macosx/macos.mm, but using UIKit instead of AppKit. */

#include "../../stdafx.h"
#include "../../core/bitmath_func.hpp"
#include "../../rev.h"
#include "../macosx/macos.h"
#include "../../string_func.h"
#include "../../fileio_func.h"
#include <atomic>
#include <pthread.h>
#include <sys/param.h>

#import <UIKit/UIKit.h>

#ifdef WITH_SDL2
#	include <SDL.h>
#	include <SDL_syswm.h>
#endif

#ifdef WITH_SDL2

/* Recent iOS SDKs require adopting the UIScene life cycle (TN3187), which SDL2
 * does not implement. Info.plist declares a scene manifest pointing at
 * OTTDSceneDelegate below; the UIWindow that SDL creates is attached to the
 * connected scene, whichever of the two shows up first. */

static UIWindowScene *_ottd_window_scene = nil; ///< The scene connected by UIKit.
static SDL_Window *_ottd_sdl_window = nullptr;  ///< The main window created by the video driver.
static std::atomic<bool> _ottd_scene_active(false); ///< Whether our scene is foreground-active.

/** Whether the app may render; iOS rejects GPU work from non-active apps. */
bool IOSAppIsActive()
{
	return _ottd_scene_active.load(std::memory_order_relaxed);
}

/** Put SDL's window on the connected scene once both exist. */
static void IOSAttachWindowToScene()
{
	if (_ottd_window_scene == nil || _ottd_sdl_window == nullptr) return;

	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);
	if (!SDL_GetWindowWMInfo(_ottd_sdl_window, &wminfo)) return;

	UIWindow *window = wminfo.info.uikit.window;
	if (window == nil || window.windowScene == _ottd_window_scene) return;

	window.windowScene = _ottd_window_scene;
	[ window makeKeyAndVisible ];
}

/** Called by the SDL2 video driver after it has created its window. */
void IOSSetSDLWindow(SDL_Window *window)
{
	_ottd_sdl_window = window;
	IOSAttachWindowToScene();
}

@interface OTTDSceneDelegate : NSObject <UIWindowSceneDelegate>
@end

@implementation OTTDSceneDelegate
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options
{
	if ([ scene isKindOfClass:[ UIWindowScene class ] ]) {
		_ottd_window_scene = (UIWindowScene *)scene;
		IOSAttachWindowToScene();
	}
}

- (void)sceneDidDisconnect:(UIScene *)scene
{
	if (_ottd_window_scene == scene) _ottd_window_scene = nil;
}

- (void)sceneDidBecomeActive:(UIScene *)scene
{
	_ottd_scene_active.store(true, std::memory_order_relaxed);
}

- (void)sceneWillResignActive:(UIScene *)scene
{
	_ottd_scene_active.store(false, std::memory_order_relaxed);
}
@end

#endif /* WITH_SDL2 */

/** Get the iOS version; reuses the macOS entry point so MacOSVersionIsAtLeast keeps working. */
std::tuple<int, int, int> GetMacOSVersion()
{
	NSOperatingSystemVersion ver = [ [ NSProcessInfo processInfo ] operatingSystemVersion ];
	return {(int)ver.majorVersion, (int)ver.minorVersion, (int)ver.patchVersion};
}

/** There is no blocking native dialog outside the UIKit run loop; log to the on-device console. */
void ShowMacDialog(std::string_view title, std::string_view message, std::string_view buttonLabel)
{
	fmt::print(stderr, "{}: {}\n", title, message);
}

void ShowOSErrorBox(std::string_view buf, bool system)
{
	if (system) {
		ShowMacDialog("OpenTTD has encountered an error", buf, "Quit");
	} else {
		ShowMacDialog(buf, "See the readme for more info.", "Quit");
	}
}

void OSOpenBrowser(const std::string &url)
{
	NSURL *nsurl = [ NSURL URLWithString:[ NSString stringWithUTF8String:url.c_str() ] ];
	if (nsurl == nil) return;

	dispatch_async(dispatch_get_main_queue(), ^{
		[ [ UIApplication sharedApplication ] openURL:nsurl options:@{} completionHandler:nil ];
	});
}

/** Determine and return the current user's locale. */
std::optional<std::string> GetCurrentLocale(const char *)
{
	NSString *preferredLang = [ [ NSLocale preferredLanguages ] firstObject ];
	if (preferredLang == nil) return std::nullopt;

	/* iOS reports e.g. "en-US"; OpenTTD expects the POSIX-style "en_US". */
	std::string ret = [ preferredLang UTF8String ];
	for (auto &c : ret) {
		if (c == '-') c = '_';
	}
	if (ret.empty()) return std::nullopt;
	return ret;
}

/** Register the (flat) application bundle, which holds the shipped game data, as a search path. */
void CocoaSetApplicationBundleDir()
{
	extern EnumIndexArray<std::string, Searchpath, Searchpath::End> _searchpaths;

	char tmp[MAXPATHLEN];
	CFAutoRelease<CFURLRef> url(CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle()));
	if (CFURLGetFileSystemRepresentation(url.get(), true, (unsigned char *)tmp, MAXPATHLEN)) {
		_searchpaths[Searchpath::ApplicationBundleDir] = tmp;
		AppendPathSeparator(_searchpaths[Searchpath::ApplicationBundleDir]);
	} else {
		_searchpaths[Searchpath::ApplicationBundleDir].clear();
	}
}

/** Check if a font is a monospace font. */
bool IsMonospaceFont(CFStringRef name)
{
	UIFont *font = [ UIFont fontWithName:(__bridge NSString *)name size:0.0f ];
	if (font == nil) return false;

	UIFontDescriptorSymbolicTraits traits = font.fontDescriptor.symbolicTraits;
	return (traits & UIFontDescriptorTraitMonoSpace) != 0;
}

/** Set the name of the current thread for the debugger. */
void MacOSSetThreadName(const std::string &name)
{
	pthread_setname_np(name.c_str());

	NSThread *cur = [ NSThread currentThread ];
	if (cur != nil) [ cur setName:[ NSString stringWithUTF8String:name.c_str() ] ];
}

uint64_t MacOSGetPhysicalMemory()
{
	return [ [ NSProcessInfo processInfo ] physicalMemory ];
}

/** Suggested interface scale; buttons must be large enough to hit with a finger. */
int IOSGetSuggestedUIScale()
{
	return [ UIDevice currentDevice ].userInterfaceIdiom == UIUserInterfaceIdiomPad ? 150 : 125;
}
