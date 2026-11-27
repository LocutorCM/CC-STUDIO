cmake_minimum_required(VERSION 3.22)
project(CCStudio VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
    # CC Studio targets Intel Macs running macOS Monterey (12.0) or newer.
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build Intel macOS binaries" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum supported macOS version" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(CCStudioAssets SOURCES
    Assets/cc-studio-bg.svg
)

juce_add_plugin(CCStudio
    COMPANY_NAME "CC Studio"
    BUNDLE_ID "com.ccstudio.reverb"
    PLUGIN_MANUFACTURER_CODE CcSt
    PLUGIN_CODE CcRv
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "CC Studio"
    DESCRIPTION "Creative stereo vocal reverb"
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx Reverb
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Generate JuceHeader.h and expose its build directory to the plugin target.
juce_generate_juce_header(CCStudio)

target_sources(CCStudio PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_compile_definitions(CCStudio PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(CCStudio PRIVATE
    CCStudioAssets
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
