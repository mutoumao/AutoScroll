#include <Windows.h>

// 全局控制变量
bool g_running = true;
bool g_paused = false;
bool g_spacePressed = false;

// 窗口控件ID
#define ID_BUTTON 101
#define ID_STATUS 102

// 窗口消息处理函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 创建按钮
            CreateWindow("BUTTON", "立即退出", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         10, 10, 100, 30, hWnd, (HMENU)ID_BUTTON, NULL, NULL);
            
            // 创建状态文本
            CreateWindow("STATIC", "状态: 运行中", WS_VISIBLE | WS_CHILD,
                         10, 50, 150, 20, hWnd, (HMENU)ID_STATUS, NULL, NULL);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUTTON) {
                g_running = false; // 设置退出标志
                PostQuitMessage(0); // 退出消息循环
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// 滚动控制线程
DWORD WINAPI ScrollThread(LPVOID lpParam) {
    HWND hStatus = GetDlgItem((HWND)lpParam, ID_STATUS);
    DWORD lastTime = GetTickCount();
    const DWORD interval = 1000;
    const int scrollAmount = -120;

    while (g_running) {
        // 检测空格键
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            if (!g_spacePressed) {
                g_paused = !g_paused;
                g_spacePressed = true;
                // 更新状态显示
                SetWindowText(hStatus, g_paused ? "状态: 已暂停" : "状态: 运行中");
            }
        } else {
            g_spacePressed = false;
        }

        // 执行滚动
        if (!g_paused && (GetTickCount() - lastTime >= interval)) {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = scrollAmount;
            SendInput(1, &input, sizeof(INPUT));
            lastTime = GetTickCount();
        }
        Sleep(100);
    }
    return 0;
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 注册窗口类
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "AutoScrollWindow";
    RegisterClass(&wc);

    // 创建窗口
    HWND hWnd = CreateWindow("AutoScrollWindow", "自动滚轮控制 v1.2", 
                            WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                            CW_USEDEFAULT, CW_USEDEFAULT, 250, 150, 
                            NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 启动滚动线程
    CreateThread(NULL, 0, ScrollThread, hWnd, 0, NULL);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}