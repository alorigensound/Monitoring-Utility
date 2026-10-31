cmake_minimum_required(VERSION 3.22)
project(MonitorUtility VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.10
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(MonitorUtility
    COMPANY_NAME "Independent"
    BUNDLE_ID "com.independent.monitorutility"
    PLUGIN_MANUFACTURER_CODE InDp
    PLUGIN_CODE MnUt
    FORMATS VST3
    PRODUCT_NAME "Monitor Utility"
    PLUGIN_NAME "Monitor Utility"
    DESCRIPTION "Band solo and mono monitoring utility"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Tools Stereo
)

target_sources(MonitorUtility PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(MonitorUtility PRIVATE cxx_std_17)
target_compile_definitions(MonitorUtility PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MonitorUtility PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
