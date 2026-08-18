#pragma once

#include "native_elan_framebuffer.h"
#include "jvs_837_13551.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

class NativeWindowsPresenter {
public:
    void present(const std::vector<NativeElanFrameScene>& scenes,
                 const std::vector<uint8_t>* naomi2Vram = nullptr) {
        if (!requestChecked_) {
            requestChecked_ = true;
            const char* value = std::getenv("IDAS3_NATIVE_WINDOW");
            enabled_ = value && *value && std::strcmp(value, "0") != 0;
            const char* fpsText = std::getenv("IDAS3_NATIVE_WINDOW_FPS");
            if (fpsText && *fpsText) {
                char* end = nullptr;
                const unsigned long parsed = std::strtoul(fpsText, &end, 10);
                if (end != fpsText && *end == '\0' && parsed <= 240u)
                    maximumFps_ = static_cast<uint32_t>(parsed);
            }
            if (maximumFps_ != 0u)
                minimumRasterInterval_ = std::chrono::microseconds(
                    1000000u / maximumFps_);
        }
        if (!enabled_) return;
        if (!window_ && !createWindow()) {
            enabled_ = false;
            return;
        }
        pumpMessages();
        if (!window_) return;
        pollInitialDInput();
        if (scenes.empty()) {
            if (!waitingPainted_) {
                waitingPainted_ = true;
                InvalidateRect(window_, nullptr, FALSE);
                UpdateWindow(window_);
            }
            return;
        }
        const size_t sceneIndex =
            NativeElanDiagnosticFramebuffer::selectMostCompleteRecentScene(scenes);
        const uint64_t frame = scenes[sceneIndex].frame;
        // A retained scene is immutable. If its projection/state produced no
        // drawable batches, retrying the same scene on every guest service
        // call cannot change that result and burns host CPU for no output.
        // Cache the same eight-frame horizon as the source scene tail because
        // several sparse passes can alternate as that tail rolls forward.
        if (frame == presentedFrame_) return;
        if (std::find(emptySceneFrames_.begin(), emptySceneFrames_.end(), frame) !=
            emptySceneFrames_.end()) {
            cachedEmptySkips_.fetch_add(1u, std::memory_order_relaxed);
            return;
        }
        candidateFrames_.fetch_add(1u, std::memory_order_relaxed);
        // The game may service ELAN more than 100 times per wall-clock second.
        // Rasterizing every service call only starves translated guest code;
        // it cannot make a normal display show more useful frames. Keep the
        // first scene immediate, then cap only host preview rasterization.
        // Guest execution, ELAN capture, and the bounded scene tail continue
        // on every original frame-service call. Set FPS=0 for uncapped A/Bs.
        const auto rasterStarted = std::chrono::steady_clock::now();
        if (minimumRasterInterval_.count() != 0 &&
            lastRasterStarted_ != std::chrono::steady_clock::time_point{} &&
            rasterStarted - lastRasterStarted_ < minimumRasterInterval_) {
            throttledFrames_.fetch_add(1u, std::memory_order_relaxed);
            return;
        }
        lastRasterStarted_ = rasterStarted;
        NativeElanFramebufferImage image =
            NativeElanDiagnosticFramebuffer::renderLatestSceneRgb(
                scenes, 640u, 480u, naomi2Vram, sceneIndex);
        if (image.rgb.empty()) {
            emptyFrames_.fetch_add(1u, std::memory_order_relaxed);
            if (emptySceneFrames_.size() >= 8u)
                emptySceneFrames_.erase(emptySceneFrames_.begin());
            emptySceneFrames_.push_back(frame);
            return;
        }
        rasterizedFrames_.fetch_add(1u, std::memory_order_relaxed);
        presentedFrame_ = frame;
        stats_ = image.result;
        bgr_.resize(image.rgb.size());
        for (size_t i = 0; i < image.rgb.size(); i += 3u) {
            bgr_[i + 0u] = image.rgb[i + 2u];
            bgr_[i + 1u] = image.rgb[i + 1u];
            bgr_[i + 2u] = image.rgb[i + 0u];
        }
        if (!firstSceneLogged_) {
            firstSceneLogged_ = true;
            std::fprintf(stderr,
                "[NATIVE-WINDOW] first_scene=%llu batches=%u vertices=%u triangles=%u\n",
                static_cast<unsigned long long>(presentedFrame_),stats_.acceptedBatches,
                stats_.vertices,stats_.triangles);
        }
        InvalidateRect(window_, nullptr, FALSE);
        UpdateWindow(window_);
    }

    void dumpStats() const {
        std::fprintf(stderr,
            "[NATIVE-WINDOW-CADENCE] max_fps=%u candidate_frames=%llu "
            "rasterized_frames=%llu empty_frames=%llu cached_empty_skips=%llu "
            "throttled_frames=%llu input_polls=%llu input_transitions=%llu "
            "coin_edges=%llu\n",
            maximumFps_,
            static_cast<unsigned long long>(
                candidateFrames_.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                rasterizedFrames_.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                emptyFrames_.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                cachedEmptySkips_.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                throttledFrames_.load(std::memory_order_relaxed)),
            static_cast<unsigned long long>(g_naomiJvs13551().hostInputPolls),
            static_cast<unsigned long long>(g_naomiJvs13551().hostInputTransitions),
            static_cast<unsigned long long>(g_naomiJvs13551().hostCoinEdges));
    }

private:
    static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
        NativeWindowsPresenter* self = reinterpret_cast<NativeWindowsPresenter*>(
            GetWindowLongPtrA(window, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            const auto* create = reinterpret_cast<const CREATESTRUCTA*>(lparam);
            self = static_cast<NativeWindowsPresenter*>(create->lpCreateParams);
            SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->window_ = window;
        }
        if (!self) return DefWindowProcA(window, message, wparam, lparam);
        if (message == WM_CLOSE) {
            DestroyWindow(window);
            return 0;
        }
        if (message == WM_DESTROY) {
            self->window_ = nullptr;
            self->enabled_ = false;
            return 0;
        }
        if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
            // Rendering a software frame can take longer than a quick key
            // tap. Preserve the initial key-down edge until the next cabinet
            // input poll instead of relying solely on GetAsyncKeyState.
            const bool repeated = (static_cast<uintptr_t>(lparam) &
                                   (uintptr_t{1u} << 30u)) != 0u;
            if (!repeated && wparam < 256u)
                self->pressedKeys_[static_cast<uint8_t>(wparam)] = true;
        }
        if (message == WM_KILLFOCUS)
            std::memset(self->pressedKeys_, 0, sizeof(self->pressedKeys_));
        if (message == WM_ERASEBKGND) return 1;
        if (message == WM_PAINT) {
            self->paint();
            return 0;
        }
        return DefWindowProcA(window, message, wparam, lparam);
    }

    bool createWindow() {
        const HINSTANCE instance = GetModuleHandleA(nullptr);
        WNDCLASSEXA wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = &NativeWindowsPresenter::windowProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = "IDAS3SH4RecompNativePresenter";
        RegisterClassExA(&wc);
        RECT desired{0, 0, 640, 524};
        AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, FALSE);
        window_ = CreateWindowExA(0, wc.lpszClassName,
            "Initial D Arcade Stage 3 - SH4Recomp Native Graphics Preview",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            desired.right - desired.left, desired.bottom - desired.top,
            nullptr, nullptr, instance, this);
        if (!window_) return false;
        std::fprintf(stderr,
            "[NATIVE-WINDOW] created=1 title='Initial D Arcade Stage 3 - SH4Recomp Native Graphics Preview' size=640x524 max_fps=%u\n",
            maximumFps_);
        ShowWindow(window_, SW_SHOWNORMAL);
        UpdateWindow(window_);
        return true;
    }

    void pumpMessages() {
        MSG message{};
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

    void pollInitialDInput() {
        InitialDHostInputState input{};
        if (GetForegroundWindow() == window_) {
            const auto down = [](int key) {
                return (GetAsyncKeyState(key) & 0x8000) != 0;
            };
            const auto pressed = [&](int key) {
                const bool value = key >= 0 && key < 256 && pressedKeys_[key];
                if (key >= 0 && key < 256) pressedKeys_[key] = false;
                return value;
            };
            const bool left = down(VK_LEFT) || down('A');
            const bool right = down(VK_RIGHT) || down('D');
            const bool startPressed = pressed(VK_RETURN);
            const bool viewPressed = pressed('V');
            const bool gearUpPressed = pressed('E');
            const bool gearDownPressed = pressed('Q');
            const bool coinPressed = pressed('5');
            const bool servicePressed = pressed(VK_F1);
            const bool testPressed = pressed(VK_F2);
            input.steering = left == right ? 0x8000u :
                (left ? 0x0000u : 0xFFFFu);
            input.accel = (down(VK_UP) || down('W')) ? 0xFFFFu : 0u;
            input.brake = (down(VK_DOWN) || down('S')) ? 0xFFFFu : 0u;
            input.start = down(VK_RETURN) || startPressed;
            input.view = down('V') || viewPressed;
            input.gearUp = down('E') || gearUpPressed;
            input.gearDown = down('Q') || gearDownPressed;
            input.coin = down('5') || coinPressed;
            input.service = down(VK_F1) || servicePressed;
            input.test = down(VK_F2) || testPressed;
        }
        if (!(input == lastInput_)) {
            std::fprintf(stderr,
                "[NATIVE-INPUT] start=%u view=%u gear=%u,%u coin=%u "
                "service=%u test=%u steering=%04X accel=%04X brake=%04X\n",
                input.start ? 1u : 0u, input.view ? 1u : 0u,
                input.gearUp ? 1u : 0u, input.gearDown ? 1u : 0u,
                input.coin ? 1u : 0u, input.service ? 1u : 0u,
                input.test ? 1u : 0u, input.steering, input.accel, input.brake);
            lastInput_ = input;
        }
        g_naomiJvs13551().applyInitialDHostInput(input);
    }

    void paint() {
        if (!window_) return;
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(window_, &ps);
        RECT client{};
        GetClientRect(window_, &client);
        const HBRUSH background = CreateSolidBrush(RGB(34, 34, 38));
        FillRect(dc, &client, background);
        DeleteObject(background);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(238, 238, 240));
        char status[256]{};
        if (bgr_.empty()) {
            std::snprintf(status, sizeof(status),
                "Running exact game code - waiting for the first finalized frontend scene...");
        } else {
            std::snprintf(status, sizeof(status),
                "Native diagnostic frame %llu | %u batch | %u vertices | %u triangles",
                static_cast<unsigned long long>(presentedFrame_), stats_.acceptedBatches,
                stats_.vertices, stats_.triangles);
        }
        TextOutA(dc, 12, 12, status, static_cast<int>(std::strlen(status)));
        static const char controls[] =
            "Drive: arrows/WASD  Start: Enter  Shift: Q/E  View: V  Coin: 5  Service/Test: F1/F2";
        TextOutA(dc, 12, 31, controls, static_cast<int>(sizeof(controls) - 1u));
        if (!bgr_.empty()) {
            BITMAPINFO info{};
            info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            info.bmiHeader.biWidth = static_cast<LONG>(stats_.width);
            info.bmiHeader.biHeight = -static_cast<LONG>(stats_.height);
            info.bmiHeader.biPlanes = 1;
            info.bmiHeader.biBitCount = 24;
            info.bmiHeader.biCompression = BI_RGB;
            const int outputWidth = std::max(1L, client.right - client.left);
            const int outputHeight = std::max(1L, client.bottom - client.top - 62L);
            StretchDIBits(dc, 0, 62, outputWidth, outputHeight,
                0, 0, static_cast<int>(stats_.width), static_cast<int>(stats_.height),
                bgr_.data(), &info, DIB_RGB_COLORS, SRCCOPY);
        }
        EndPaint(window_, &ps);
    }

    bool requestChecked_ = false;
    bool enabled_ = false;
    bool waitingPainted_ = false;
    bool firstSceneLogged_ = false;
    HWND window_ = nullptr;
    uint32_t maximumFps_ = 30u;
    std::chrono::microseconds minimumRasterInterval_{0};
    std::chrono::steady_clock::time_point lastRasterStarted_{};
    std::vector<uint64_t> emptySceneFrames_;
    std::atomic<uint64_t> candidateFrames_{0u};
    std::atomic<uint64_t> rasterizedFrames_{0u};
    std::atomic<uint64_t> emptyFrames_{0u};
    std::atomic<uint64_t> cachedEmptySkips_{0u};
    std::atomic<uint64_t> throttledFrames_{0u};
    uint64_t presentedFrame_ = ~uint64_t{0};
    bool pressedKeys_[256]{};
    InitialDHostInputState lastInput_{};
    NativeElanFramebufferResult stats_{};
    std::vector<uint8_t> bgr_;
};

#else

class NativeWindowsPresenter {
public:
    void present(const std::vector<NativeElanFrameScene>&,
                 const std::vector<uint8_t>* = nullptr) {}
    void dumpStats() const {}
};

#endif

inline NativeWindowsPresenter& g_nativeWindowsPresenter() {
    static NativeWindowsPresenter presenter;
    return presenter;
}
