#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>

// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#scan-codes
#define RIGHT_CTRL_SCAN_CODE 0x1D
#define MENU_SCAN_CODE 0xE05D

#define CTRL_SUFFIX L"ctrl"
#define MENU_SUFFIX L"menu"

INPUT emulateWhenCopilotPressed[] = {
    // Release win
    {
        .type = INPUT_KEYBOARD,
        .ki = { .wVk = VK_LWIN, .dwFlags = KEYEVENTF_KEYUP }
    },
    // Release shift
    {
        .type = INPUT_KEYBOARD,
        .ki = { .wVk = VK_LSHIFT, .dwFlags = KEYEVENTF_KEYUP }
    },
    // Release f23
    {
        .type = INPUT_KEYBOARD,
        .ki = { .wVk = VK_F23, .dwFlags = KEYEVENTF_KEYUP }
    },
    // Press target key (set later)
    {
        .type = INPUT_KEYBOARD,
        .ki = { .wScan = 0, .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, .wVk = 0 }
    }
};

INPUT emulateWhenCopilotReleased[] = {
    // Release target key (set later)
    {
        .type = INPUT_KEYBOARD,
        .ki = { .wScan = 0, .dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY, .wVk = 0 }
    }
};

bool winPressed = false;
bool shiftPressed = false;
bool f23Pressed = false;
bool eventContainsKeysOfCopilotSequence = false;
bool targetKeyDown = false;

void emulateTargetKeyDown() {
    UINT uSent = SendInput(ARRAYSIZE(emulateWhenCopilotPressed), emulateWhenCopilotPressed, sizeof(INPUT));
    if (uSent != ARRAYSIZE(emulateWhenCopilotPressed)) {
        printf("failed to send target key down\n");
    }
    targetKeyDown = true;
    printf("emulating target key down\n");
}

void emulateTargetKeyUp() {
    UINT uSent = SendInput(ARRAYSIZE(emulateWhenCopilotReleased), emulateWhenCopilotReleased, sizeof(INPUT));
    if (uSent != ARRAYSIZE(emulateWhenCopilotReleased)) {
        printf("failed to send target key up\n");
    }
    targetKeyDown = false;
    printf("emulating target key up\n");
}

void emulateTargetKeyUpIfDown() {
    if (targetKeyDown) {
        emulateTargetKeyUp();
    }
}

LRESULT CALLBACK handleKeyboardEvent(int iCode, WPARAM wParam, LPARAM lParam) {
    PKBDLLHOOKSTRUCT pHook = (PKBDLLHOOKSTRUCT)lParam;
    unsigned char keyCode = (unsigned char)pHook->vkCode;
    eventContainsKeysOfCopilotSequence = false;

    if (keyCode == VK_LWIN || keyCode == VK_RWIN) {
        eventContainsKeysOfCopilotSequence = true;
        if (wParam == WM_KEYDOWN) {
            winPressed = true;
        }
        else if (wParam == WM_KEYUP) {
            winPressed = false;
            emulateTargetKeyUpIfDown();
        }
    }

    if (keyCode == VK_LSHIFT || keyCode == VK_RSHIFT) {
        eventContainsKeysOfCopilotSequence = true;
        if (wParam == WM_KEYDOWN) {
            shiftPressed = true;
        }
        else if (wParam == WM_KEYUP) {
            shiftPressed = false;
            emulateTargetKeyUpIfDown();
        }
    }

    if (keyCode == VK_F23) {
        eventContainsKeysOfCopilotSequence = true;
        if (wParam == WM_KEYDOWN) {
            f23Pressed = true;
        }
        else if (wParam == WM_KEYUP) {
            f23Pressed = false;
            emulateTargetKeyUpIfDown();
        }
    }

    if (eventContainsKeysOfCopilotSequence) {
        if (winPressed && shiftPressed && f23Pressed) {
            emulateTargetKeyDown();
            return 1; // cancels default copilot sequence behavior (win+shift+f23)
        }
    }

    return CallNextHookEx(NULL, iCode, wParam, lParam);
}

bool executableContainsSubstring(const wchar_t* substring) {
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    wchar_t* filename = wcsrchr(path, L'\\');
    if (filename)
        filename++;
    else
        filename = path;

    wchar_t lower[MAX_PATH];
    wcsncpy_s(lower, MAX_PATH, filename, _TRUNCATE);
    _wcslwr_s(lower, MAX_PATH);

    return wcsstr(lower, substring);
}

void setTargetKeyBasedOnExecutableName()
{
    WORD targetKey;
    if (executableContainsSubstring(CTRL_SUFFIX)) {
        targetKey = RIGHT_CTRL_SCAN_CODE;
    }
    else if (executableContainsSubstring(MENU_SUFFIX)) {
        targetKey = MENU_SCAN_CODE;
    }
    else {
        printf("invalid executable name\n");
        exit(EXIT_FAILURE);
    }

    emulateWhenCopilotPressed[3].ki.wScan = targetKey;
    emulateWhenCopilotReleased[0].ki.wScan = targetKey;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\remap-copilot-key");

    // Ensure only one instance of the app is running
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return EXIT_FAILURE;
    }

    setTargetKeyBasedOnExecutableName();

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, handleKeyboardEvent, NULL, 0);

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

    return EXIT_SUCCESS;
}
