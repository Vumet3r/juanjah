cmake_minimum_required(VERSION 3.27)
project(Juanjah VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
add_compile_definitions(JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING)

add_subdirectory(lib/JUCE)

juce_add_plugin(Juanjah
  VENDOR "Javier Bleda"
  PLUGIN_MANUFACTURER_CODE "JBl1"
  PLUGIN_CODE "JNJH"
  DESCRIPTION "Juanjah - Multitap Delay"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  NEEDS_MIDI_NOTIFICATIONS FALSE
  COPY_PLUGIN_AFTER_BUILD TRUE
  FORMATS VST3 AU
)

juce_generate_juce_header(Juanjah)

# Background image (optional — drop bg.png in Resources/)
juce_add_binary_data(Resources SOURCES Resources/bg.png)

target_link_libraries(Juanjah PRIVATE Resources)

target_sources(Juanjah PRIVATE
  source/PluginProcessor.cpp
  source/PluginProcessor.h
  source/PluginEditor.cpp
  source/PluginEditor.h
  source/DelayLine.h
  source/LFO.h
  source/Distortion.h
  source/OnePoleFilter.h
)
