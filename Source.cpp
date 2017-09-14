#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

template<class Interface> inline void SafeRelease(Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static ID2D1Factory *m_pD2DFactory;
	static IWICImagingFactory *m_pWICFactory;
	static IDWriteFactory *m_pDWriteFactory;
	static ID2D1HwndRenderTarget *m_pRenderTarget;
	static IDWriteTextFormat *m_pTextFormat;
	static ID2D1SolidColorBrush *m_pBlackBrush;
	switch (msg)
	{
	case WM_CREATE:
		{
			static const WCHAR msc_fontName[] = L"Verdana";
			static const FLOAT msc_fontSize = 50;
			HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
			if (SUCCEEDED(hr))
				hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
			if (SUCCEEDED(hr))
				hr = m_pDWriteFactory->CreateTextFormat(msc_fontName, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, msc_fontSize, L"", &m_pTextFormat);
			if (SUCCEEDED(hr))
				hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			if (SUCCEEDED(hr))
				hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			if (FAILED(hr))
				return -1;
		}
		break;
	case WM_SIZE:
		if (m_pRenderTarget)
		{
			D2D1_SIZE_U size = { LOWORD(lParam), HIWORD(lParam) };
			m_pRenderTarget->Resize(size);
		}
		break;
	case WM_DISPLAYCHANGE:
		InvalidateRect(hWnd, 0, 0);
		break;
	case WM_PAINT:
		{
			HRESULT hr = S_OK;
			if (!m_pRenderTarget)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				D2D1_SIZE_U size = D2D1::SizeU(rect.right, rect.bottom);
				hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &m_pRenderTarget);
				if (SUCCEEDED(hr))
					hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
			}
			if (SUCCEEDED(hr))
			{
				static const WCHAR sc_helloWorld[] = L"Hello, World!";
				D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();
				m_pRenderTarget->BeginDraw();
				m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
				m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
				m_pRenderTarget->DrawText(sc_helloWorld, ARRAYSIZE(sc_helloWorld) - 1, m_pTextFormat, D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), m_pBlackBrush);
				hr = m_pRenderTarget->EndDraw();
				if (hr == D2DERR_RECREATE_TARGET)
				{
					hr = S_OK;
					SafeRelease(&m_pRenderTarget);
					SafeRelease(&m_pBlackBrush);
				}
			}
		}
		ValidateRect(hWnd, NULL);
		break;
	case WM_DESTROY:
		SafeRelease(&m_pD2DFactory);
		SafeRelease(&m_pDWriteFactory);
		SafeRelease(&m_pRenderTarget);
		SafeRelease(&m_pTextFormat);
		SafeRelease(&m_pBlackBrush);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	HeapSetInformation(0, HeapEnableTerminationOnCorruption, 0, 0);
	if (FAILED(CoInitialize(0))) return 0;
	TCHAR szClassName[] = TEXT("Window");
	WNDCLASS wndclass = { CS_HREDRAW | CS_VREDRAW,WndProc,0,0,hInstance,0,LoadCursor(0,IDC_ARROW),0,0,szClassName };
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(szClassName, TEXT("Direct2D Sample"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 0, 0, hInstance, 0);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CoUninitialize();
	return (int)msg.wParam;
}
