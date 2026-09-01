#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <xinput.h>

#include "native_controller_bindings.h"

#include <array>
#include <string>
#include <vector>

namespace idas3input {

// Runtime loading keeps the public build independent of a particular XInput
// import library while still preferring the current Windows implementation.
// The presenter and the no-window hardware acceptance probe deliberately share
// this class so a probe success proves the product's real discovery path.
class NativeXInputRuntime {
public:
    using GetStateFunction = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);

    NativeXInputRuntime() {
        char systemDirectory[MAX_PATH]{};
        const UINT length = GetSystemDirectoryA(
            systemDirectory, static_cast<UINT>(std::size(systemDirectory)));
        if (length == 0u || length >= std::size(systemDirectory)) return;

        static constexpr const char* candidates[] = {
            "xinput1_4.dll", "xinput9_1_0.dll", "xinput1_3.dll"};
        for (const char* candidate : candidates) {
            std::string path(systemDirectory, length);
            path.push_back('\\');
            path += candidate;
            module_ = LoadLibraryA(path.c_str());
            if (!module_) continue;
            getState_ = reinterpret_cast<GetStateFunction>(
                GetProcAddress(module_, "XInputGetState"));
            if (getState_) {
                moduleName_ = candidate;
                break;
            }
            FreeLibrary(module_);
            module_ = nullptr;
        }
    }

    ~NativeXInputRuntime() {
        if (module_) FreeLibrary(module_);
    }

    NativeXInputRuntime(const NativeXInputRuntime&) = delete;
    NativeXInputRuntime& operator=(const NativeXInputRuntime&) = delete;

    bool available() const { return getState_ != nullptr; }
    const char* moduleName() const {
        return moduleName_.empty() ? "none" : moduleName_.c_str();
    }

    DWORD getState(DWORD user, XINPUT_STATE* state) const {
        return getState_ ? getState_(user, state) : ERROR_DEVICE_NOT_CONNECTED;
    }

    std::vector<DWORD> connectedUsers() const {
        std::vector<DWORD> users;
        for (DWORD user = 0u; user < XUSER_MAX_COUNT; ++user) {
            XINPUT_STATE state{};
            if (getState(user, &state) == ERROR_SUCCESS)
                users.push_back(user);
        }
        return users;
    }

private:
    HMODULE module_ = nullptr;
    GetStateFunction getState_ = nullptr;
    std::string moduleName_{};
};

inline ControllerSnapshot nativeXInputSnapshot(const XINPUT_STATE& state) {
    ControllerSnapshot snapshot{};
    snapshot.axes[0] = normalizeSignedAxis(
        state.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    snapshot.axes[1] = normalizeSignedAxis(
        state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    snapshot.axes[2] = normalizeSignedAxis(
        state.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    snapshot.axes[3] = normalizeSignedAxis(
        state.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    snapshot.axes[4] = normalizeUnsignedAxis(
        state.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    snapshot.axes[5] = normalizeUnsignedAxis(
        state.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    static constexpr WORD masks[] = {
        XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X,
        XINPUT_GAMEPAD_Y, XINPUT_GAMEPAD_LEFT_SHOULDER,
        XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_BACK,
        XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_LEFT_THUMB,
        XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_DPAD_UP,
        XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT,
        XINPUT_GAMEPAD_DPAD_RIGHT};
    for (size_t index = 0u; index < std::size(masks); ++index)
        snapshot.buttons[index] =
            (state.Gamepad.wButtons & masks[index]) != 0u ? 1u : 0u;
    return snapshot;
}

}  // namespace idas3input

#endif  // _WIN32
