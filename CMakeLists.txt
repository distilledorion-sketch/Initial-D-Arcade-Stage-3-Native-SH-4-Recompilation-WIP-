cmake_minimum_required(VERSION 3.20)
project(idas3_native_sh4_recomp_public_checks LANGUAGES CXX)

enable_testing()

add_executable(native_elan_bridge_test tests/native_elan_bridge_test.cpp)
target_compile_features(native_elan_bridge_test PRIVATE cxx_std_17)
target_include_directories(native_elan_bridge_test PRIVATE src/runtime)

add_test(NAME native_elan_bridge_classifier COMMAND native_elan_bridge_test)

add_executable(native_controller_bindings_test
    tests/native_controller_bindings_test.cpp)
target_compile_features(native_controller_bindings_test PRIVATE cxx_std_17)
target_include_directories(native_controller_bindings_test PRIVATE src/runtime)

add_test(NAME native_controller_bindings COMMAND native_controller_bindings_test)

if(WIN32)
    add_executable(native_xinput_hardware_probe
        tests/native_xinput_hardware_probe.cpp)
    target_compile_features(native_xinput_hardware_probe PRIVATE cxx_std_17)
    target_include_directories(native_xinput_hardware_probe PRIVATE src/runtime)
    add_test(NAME native_xinput_runtime
        COMMAND native_xinput_hardware_probe --allow-none)

    add_executable(native_audio_endpoint_probe
        tests/native_audio_endpoint_probe.cpp)
    target_compile_features(native_audio_endpoint_probe PRIVATE cxx_std_17)
    target_include_directories(native_audio_endpoint_probe PRIVATE src/runtime)
    target_link_libraries(native_audio_endpoint_probe PRIVATE winmm)
    add_test(NAME native_audio_endpoint
        COMMAND native_audio_endpoint_probe --allow-none)
endif()
