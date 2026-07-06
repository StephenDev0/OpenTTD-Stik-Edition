add_files(
    font_osx.cpp
    font_osx.h
    macos.mm
    macos_objective_c.h
    string_osx.cpp
    string_osx.h
    survey_osx.cpp
    CONDITION APPLE AND NOT IOS
)

# These only use POSIX and CoreFoundation APIs, which also exist on iOS.
add_files(
    crashlog_osx.cpp
    macos.h
    osx_stdafx.h
    CONDITION APPLE
)

if(APPLE AND NOT IOS)
    target_sources(openttd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/osx_main.cpp)
endif()
