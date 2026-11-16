cmake_minimum_required(VERSION 3.22)
# Versionsnummer steht in der Datei VERSION (wird bei jeder Aenderung hochgezaehlt)
file(STRINGS VERSION RPH_VERSION LIMIT_COUNT 1)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS VERSION)
project(Radiophone VERSION ${RPH_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Laufzeitbibliothek fest einbinden: das Plugin braucht dann kein VC++-Redistributable
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(RADIOPHONE_DSP_ONLY "Build portable DSP regression tests without JUCE" OFF)
include(CTest)
if(BUILD_TESTING)
    find_package(Threads REQUIRED)
    add_executable(RadiophoneDSPTests Tests/DSPTests.cpp)
    target_include_directories(RadiophoneDSPTests PRIVATE Source)
    target_link_libraries(RadiophoneDSPTests PRIVATE Threads::Threads)
    add_test(NAME DSP COMMAND RadiophoneDSPTests)
    add_executable(RadiophoneBenchmark Tests/Benchmark.cpp)
    target_include_directories(RadiophoneBenchmark PRIVATE Source)
endif()
if(RADIOPHONE_DSP_ONLY)
    return()
endif()

set(RADIOPHONE_JUCE_PATH "" CACHE PATH "Lokaler JUCE-Ordner (leer = automatisch laden)")
set(RADIOPHONE_FORMATS "VST3" CACHE STRING "Plugin-Formate")

if(RADIOPHONE_JUCE_PATH)
    add_subdirectory(${RADIOPHONE_JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# Eingebaute Aufnahmen (Squelch, Senderwechsel) nur, wenn der Ordner lokal vorhanden ist.
# Die Aufnahmen stammen aus der HoerTalk-Community und sind dort frei nutzbar.
set(RPH_ASSET_SOURCES Assets/logo.png)
set(RPH_SAMPLE_DEFS "RPH_HAS_SAMPLES=0")
file(GLOB RPH_SAMPLE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Assets/samples/*.wav)
if(RPH_SAMPLE_FILES)
    list(APPEND RPH_ASSET_SOURCES ${RPH_SAMPLE_FILES})
    set(RPH_SAMPLE_DEFS "RPH_HAS_SAMPLES=1")
endif()
juce_add_binary_data(RadiophoneAssets SOURCES ${RPH_ASSET_SOURCES})

juce_add_plugin(Radiophone
    COMPANY_NAME "Stromschlag Audio"
    PLUGIN_MANUFACTURER_CODE Strm
    PLUGIN_CODE Rdph
    FORMATS ${RADIOPHONE_FORMATS}
    PRODUCT_NAME "Radiophone"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(Radiophone PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Radiophone PUBLIC
    PLUGIN_VERSION_TEXT="${RPH_VERSION}"
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    ${RPH_SAMPLE_DEFS})

if(MSVC)
    target_compile_options(Radiophone PRIVATE /utf-8)
endif()

target_link_libraries(Radiophone
    PRIVATE
        RadiophoneAssets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(BUILD_TESTING)
    juce_add_console_app(RadiophoneProcessorTests PRODUCT_NAME "Radiophone processor tests")
    target_sources(RadiophoneProcessorTests PRIVATE Tests/ProcessorTests.cpp)
    target_include_directories(RadiophoneProcessorTests PRIVATE Source)
    target_compile_definitions(RadiophoneProcessorTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(RadiophoneProcessorTests PRIVATE Radiophone juce::juce_audio_utils)
    add_test(NAME Processor COMMAND RadiophoneProcessorTests)
endif()
