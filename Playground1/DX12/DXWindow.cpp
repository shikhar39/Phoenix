#include "stdafx.h"
#include "DXWindow.h"


namespace PhoenixEngine {
    namespace DX12 {    
        Window::Window(int inWidth, int inHeight, std::string inName) : PhoenixEngine::Window(inWidth, inHeight, inName) {}
    }
}
// bool Window::Init()
// {
    /*
    // #################### Window Class #################################
    WNDCLASSEXW wcex{};
    
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = &Window::OnWindowMessage;
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
    
   // ########################## Swap Chain Description #############################
    DXGI_SWAP_CHAIN_DESC1 swd{};
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC sfd{};

    swd.Width = m_width;
    swd.Height = m_height;
    swd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swd.Stereo = false;
    swd.SampleDesc.Count = 1;
    swd.SampleDesc.Quality = 0;
    swd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swd.BufferCount = GetFrameCount();
    swd.Scaling = DXGI_SCALING_STRETCH;
    swd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    
    sfd.Windowed = true;

    // ########################## Swap Chain #############################
    */

//     int maj, min, rev;
//     glfwGetVersion(&maj, &min, &rev);
//
//     std::cout << "Glfw version: " << maj << min << rev << std::endl;
//     glfwInit();
//     if (!glfwInit()) {
//         std::cerr << "Sorry dude I couldn't do it:: GLFWInit()" << std::endl;
//     }
//
//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//     GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
//     return false;
// }
//
// Window::~Window()
// {
//     glfwTerminate();
// }
//
// LRESULT CALLBACK Window::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
// {
//     return LRESULT();
// }