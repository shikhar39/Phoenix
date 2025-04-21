#pragma once


class DXWindow
{
public:
	bool Init();
private:
	ATOM m_wndClass = 0;
	HWND m_window = nullptr;

	UINT m_width = 800;
	UINT m_height = 600;

	static constexpr size_t FrameCount = 2;
	static constexpr size_t GetFrameCount() {
		return FrameCount;
	}
private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

