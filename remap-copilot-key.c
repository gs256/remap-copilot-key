#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>

// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#scan-codes
#define RIGHT_CTRL_SCAN_CODE 0x1D

// Sent when copilot key is pressed
INPUT emulatedCtrlDownInputs[] = {
    // Release win
    {
        .type = INPUT_KEYBOARD,
        .ki = {.wVk = VK_LWIN, .dwFlags = KEYEVENTF_KEYUP }
    },
    // Release shift
    {
        .type = INPUT_KEYBOARD,
        .ki = {.wVk = VK_LSHIFT, .dwFlags = KEYEVENTF_KEYUP }
    },
    // Release f23
    {
        .type = INPUT_KEYBOARD,
        .ki = {.wVk = VK_F23, .dwFlags = KEYEVENTF_KEYUP }
    },
    // Press right ctrl
    {
        .type = INPUT_KEYBOARD,
        .ki = {.wScan = RIGHT_CTRL_SCAN_CODE, .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, .wVk = 0 }
    }
};

// Sent when copilot key is released
INPUT emulatedCtrlUpInputs[] = {
    // Release right ctrl
    {
        .type = INPUT_KEYBOARD,
        .ki = {.wScan = RIGHT_CTRL_SCAN_CODE, .dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, .wVk = 0 }
    }
};

bool winPressed = false;
bool shiftPressed = false;
bool f23Pressed = false;
bool eventContainsTargetKeys = false;

bool emulatedCtrlDown = false;

void emulateCtrlDown() {
    UINT uSent = SendInput(ARRAYSIZE(emulatedCtrlDownInputs), emulatedCtrlDownInputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(emulatedCtrlDownInputs)) {
        printf("failed to send ctrl down\n");
    }
    emulatedCtrlDown = true;
    printf("emulating ctrl down\n");
}

void emulateCtrlUp() {
    UINT uSent = SendInput(ARRAYSIZE(emulatedCtrlUpInputs), emulatedCtrlUpInputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(emulatedCtrlUpInputs)) {
        printf("failed to send ctrl up\n");
    }
    emulatedCtrlDown = false;
    printf("emulating ctrl up\n");
}

void emulateCtrlUpIfDown() {
    if (emulatedCtrlDown) {
        emulateCtrlUp();
    }
}

LRESULT CALLBACK handleKeyboardEvent(int iCode, WPARAM wParam, LPARAM lParam) {
    PKBDLLHOOKSTRUCT pHook = (PKBDLLHOOKSTRUCT)lParam;
    unsigned char keyCode = (unsigned char)pHook->vkCode;
    eventContainsTargetKeys = false;

    if (keyCode == VK_LWIN || keyCode == VK_RWIN) {
        eventContainsTargetKeys = true;
        if (wParam == WM_KEYDOWN) {
            winPressed = true;
        }
        else if (wParam == WM_KEYUP) {
            winPressed = false;
            emulateCtrlUpIfDown();
        }
    }

    if (keyCode == VK_LSHIFT || keyCode == VK_RSHIFT) {
        eventContainsTargetKeys = true;
        if (wParam == WM_KEYDOWN) {
            shiftPressed = true;
        }
        else if (wParam == WM_KEYUP) {
            shiftPressed = false;
            emulateCtrlUpIfDown();
        }
    }

    if (keyCode == VK_F23) {
        eventContainsTargetKeys = true;
        if (wParam == WM_KEYDOWN) {
            f23Pressed = true;
        }
        else if (wParam == WM_KEYUP) {
            f23Pressed = false;
            emulateCtrlUpIfDown();
        }
    }

    if (eventContainsTargetKeys) {
        if (winPressed && shiftPressed && f23Pressed) {
            emulateCtrlDown();
            // Cancel default copilot key (win+shift+f23) behaviour
            return 1;
        }
    }

    return CallNextHookEx(NULL, iCode, wParam, lParam);
}


int main() {
    HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\remap-copilot-key");

    // Ensure only one instance of the app is running
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 1;
    }

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, handleKeyboardEvent, NULL, NULL);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    return 0;
}
