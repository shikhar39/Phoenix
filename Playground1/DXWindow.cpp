#include "stdafx.h"
#include "DXWindow.h"
bool DXWindow::Init()
{
    // #################### Window Class #################################
    WNDCLASSEXW wcex{};
    
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = &DXWindow::OnWindowMessage;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0; 
    wcex.hInstance = GetModuleHandleW(nullptr);
    wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"RTWindowCls";
    wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

    m_wndClass = RegisterClassEx(&wcex);

    if (!m_wndClass) {
        return false;
    }

    // ############################ Window ##############################
    
    m_window = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
        LPCWSTR(m_wndClass),
        L"RT Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100,
        m_width, m_height,
        nullptr,
        nullptr,
        wcex.hInstance,
        nullptr
    );

    if (m_window == nullptr) {
        return false;
    }
    
   // DXGI_SWAP_


    return false;
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return LRESULT();
}



