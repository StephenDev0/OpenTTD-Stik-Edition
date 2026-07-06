add_files(
    ios.mm
    ios_stdafx.h
    survey_ios.cpp
    CONDITION IOS
)

if(IOS)
    target_sources(openttd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ios_main.mm)
endif()
