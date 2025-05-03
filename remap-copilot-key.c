#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>

const int CTRL_SCAN_CODE = 0x1D;

bool winPressed = false;
bool shiftPressed = false;
bool f23Pressed = false;
bool eventContainsTargetKeys = false;

bool fakeCtrlPressed = false;

void press() {
    //INPUT inputs[4] = {0};

    //inputs[0].type = INPUT_KEYBOARD;
    //inputs[0].ki.wVk = VK_LWIN;
    //inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;

    //inputs[1].type = INPUT_KEYBOARD;
    //inputs[1].ki.wVk = VK_LSHIFT;
    //inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    //inputs[2].type = INPUT_KEYBOARD;
    //inputs[2].ki.wVk = VK_F23;
    //inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    //inputs[3].type = INPUT_KEYBOARD;
    //inputs[3].ki.wScan = ctrl_scan_code;
    //inputs[3].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
    //inputs[3].ki.wVk = 0; // Use scan code only


    INPUT inputs[] = {
        {
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = VK_LWIN,
                .dwFlags = KEYEVENTF_KEYUP
            }
        },
        {
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = VK_LSHIFT,
                .dwFlags = KEYEVENTF_KEYUP
            }
        },
        {
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = VK_F23,
                .dwFlags = KEYEVENTF_KEYUP
            }
        },
        {
            .type = INPUT_KEYBOARD,
            .ki = {
                .wScan = CTRL_SCAN_CODE,
                .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY,
                .wVk = 0
            }
        }
    };

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(inputs))
    {
        printf("failed to press ctrl\n");
    }

    fakeCtrlPressed = true;
    printf("Press\n");
}

void unpress() {
    //INPUT inputs[1] = { 0 };

    //inputs[0].type = INPUT_KEYBOARD;
    //inputs[0].ki.wScan = ctrl_scan_code;
    //inputs[0].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
    //inputs[0].ki.wVk = 0; // Use scan code only

    INPUT inputs[] = {
        {
            .type = INPUT_KEYBOARD,
            .ki = {
                .wScan = CTRL_SCAN_CODE,
                .dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY,
                .wVk = 0
            }
        }
    };

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(inputs))
    {
        printf("failed to unpress ctrl\n");
    }

    fakeCtrlPressed = 0;
    printf("Unpress\n");
}

void unpress_if_pressed() {
    if (fakeCtrlPressed) {
        unpress();
    }
}

LRESULT CALLBACK CheckShortcut(int iCode, WPARAM wParam, LPARAM lParam) {
    PKBDLLHOOKSTRUCT pHook = (PKBDLLHOOKSTRUCT)lParam;
    unsigned char keyCode = (unsigned char)pHook->vkCode;
    eventContainsTargetKeys = 0;

    if (keyCode == VK_LWIN || keyCode == VK_RWIN)
    {
        eventContainsTargetKeys = true;
        if (wParam == WM_KEYDOWN)
        {
            winPressed = true;
        }
        else if (wParam == WM_KEYUP)
        {
            winPressed = 0;
            unpress_if_pressed();
        }
    }

    if (keyCode == VK_LSHIFT || keyCode == VK_RSHIFT)
    {
        eventContainsTargetKeys = true;
        if (wParam == WM_KEYDOWN)
        {
            shiftPressed = true;
        }
        else if (wParam == WM_KEYUP)
        {
            shiftPressed = false;
            unpress_if_pressed();
        }
    }

    if (keyCode == VK_F23)
    {
        eventContainsTargetKeys = true;
        if (wParam == WM_KEYDOWN)
        {
            f23Pressed = true;
        }
        else if (wParam == WM_KEYUP)
        {
            f23Pressed = false;
            unpress_if_pressed();
        }
    }

    if (eventContainsTargetKeys) {
        if (winPressed && shiftPressed && f23Pressed) {
            press();
            return 1;
        }
    }

    return CallNextHookEx(NULL, iCode, wParam, lParam);
}


int main()
{
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, CheckShortcut, NULL, NULL);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);

    return 0;
}
