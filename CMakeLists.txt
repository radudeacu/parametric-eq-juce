cmake_minimum_required(VERSION 3.22)

project(ParametricEQ VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

# JUCE 8.0.x pinned deliberately over the newly-released 9.0.0 (one week old at time of
# writing) for ecosystem/tutorial maturity.
CPMAddPackage(
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 8.0.15
    VERSION 8.0.15
)

option(PARAMETRIC_EQ_BUILD_AAX "Build the AAX format (requires a local Avid AAX SDK)" OFF)
set(AAX_SDK_PATH "" CACHE PATH "Path to a locally installed Avid AAX SDK")

set(PARAMETRIC_EQ_FORMATS Standalone)

if(PARAMETRIC_EQ_BUILD_AAX)
    if(NOT AAX_SDK_PATH OR NOT EXISTS "${AAX_SDK_PATH}")
        message(FATAL_ERROR
            "PARAMETRIC_EQ_BUILD_AAX is ON but AAX_SDK_PATH is unset or invalid. "
            "Set -DAAX_SDK_PATH=<path-to-avid-aax-sdk>, or turn PARAMETRIC_EQ_BUILD_AAX OFF.")
    endif()
    list(APPEND PARAMETRIC_EQ_FORMATS AAX)
    juce_set_aax_sdk_path(${AAX_SDK_PATH})
endif()

juce_add_plugin(ParametricEQ
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Ptcm
    PLUGIN_CODE Peq1
    FORMATS ${PARAMETRIC_EQ_FORMATS}
    PRODUCT_NAME "Parametric EQ"
    AAX_CATEGORY "AAX_ePlugInCategory_EQ"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.png"
)

target_sources(ParametricEQ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Parameters/ParameterLayout.cpp
        Source/DSP/SlopeFilterChain.cpp
        Source/DSP/LowBand.cpp
        Source/DSP/HighBand.cpp
        Source/DSP/PeakBand.cpp
        Source/DSP/TestToneGenerator.cpp
        Source/Analyzer/SpectrumAnalyzerFifo.cpp
        Source/Analyzer/SpectrumAnalyzerComponent.cpp
        Source/Analyzer/EqCurveOverlayComponent.cpp
        Source/Analyzer/AnalyzerPanelComponent.cpp
        Source/UI/BandControlComponent.cpp
        Source/UI/GlobalControlsComponent.cpp
        Source/UI/TestToneControlsComponent.cpp
        Source/UI/LatencyDisplayComponent.cpp
        Source/UI/ModernLookAndFeel.cpp
)

target_include_directories(ParametricEQ PRIVATE Source)

target_compile_definitions(ParametricEQ
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        # Enables ASIO device support (MS Windows only) using JUCE's bundled
        # ASIO headers, so real audio interfaces (e.g. Focusrite) expose
        # their low-latency ASIO driver instead of only WASAPI/DirectSound.
        JUCE_ASIO=1
)

target_link_libraries(ParametricEQ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

enable_testing()
add_executable(ParametricEQ_Tests Tests/DspUnitTests.cpp)
target_include_directories(ParametricEQ_Tests PRIVATE Source)
target_sources(ParametricEQ_Tests
    PRIVATE
        Source/Parameters/ParameterLayout.cpp
        Source/DSP/SlopeFilterChain.cpp
        Source/DSP/LowBand.cpp
        Source/DSP/HighBand.cpp
        Source/DSP/PeakBand.cpp
        Source/DSP/TestToneGenerator.cpp
)
target_link_libraries(ParametricEQ_Tests PRIVATE juce::juce_audio_utils juce::juce_dsp)
add_test(NAME DspUnitTests COMMAND ParametricEQ_Tests)
