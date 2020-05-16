#include "demoapp.h"

//
//
// d2d-simple
// Direct2D Demo App
//
// Based on the code sample / tutorial provided by Microsoft here:
// https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-quickstart
//
// Removed call to GetDesktopDpi() as this API call is now deprecated and generates a compilation error
//
// April 15, 2020
//
// This approach is suitable to build any basic Direct2D application that would like to use the entire window surface (except title bar) as a drawing surface
//
//







//
//
// Utility stuff
//
//


template<class Interface>
inline void SafeRelease(
    Interface** ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}




#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif



#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif





//
//
// Application class
// DemoApp
//
//


class DemoApp
{

public:

    DemoApp() :
        m_hwnd(NULL),
        m_pDirect2dFactory(NULL),
        m_pRenderTarget(NULL),
        m_pLightSlateGrayBrush(NULL),
        m_pCornflowerBlueBrush(NULL)
    {}


    ~DemoApp()
    {
        SafeRelease(&m_pDirect2dFactory);
        SafeRelease(&m_pRenderTarget);
        SafeRelease(&m_pLightSlateGrayBrush);
        SafeRelease(&m_pCornflowerBlueBrush);
    }


    // Register the window class and call methods for instantiating drawing resources
    HRESULT Initialize()
    {
        HRESULT hr;

        // Initialize device-indpendent resources, such
        // as the Direct2D factory
        hr = CreateDeviceIndependentResources();

        if (SUCCEEDED(hr))
        {
            // Register the window class.
            WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WndProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = sizeof(LONG_PTR);
            wcex.hInstance = HINST_THISCOMPONENT;
            wcex.hbrBackground = NULL;
            wcex.lpszMenuName = NULL;
            wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
            wcex.lpszClassName = L"D2DDemoApp";

            RegisterClassEx(&wcex);


            // Because the CreateWindow function takes its size in pixels,
            // obtain the system DPI and use it to scale the window size
            //FLOAT dpiX, dpiY;                                      // Commented out as no longer needed

            // The factory returns the current system DPI. This is also the value it will use
            // to create its own windows
            //m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);      // This call is deprecated


            // Create the window
            m_hwnd = CreateWindow(
                L"D2DDemoApp",
                L"Direct2D Demo App",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT /* Removed reference to DPI here for simplicity */,
                CW_USEDEFAULT /* Removed reference to DPI here for simplicity */,
                NULL,
                NULL,
                HINST_THISCOMPONENT,
                this
            );

            hr = m_hwnd ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                ShowWindow(m_hwnd, SW_SHOWNORMAL);
                UpdateWindow(m_hwnd);
            }
        }

        return hr;
    }

    
    // Process and dispatch messages
    void RunMessageLoop()
    {
        MSG msg;

        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }



private:

    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush;


    // Initialize device-independent resources
    HRESULT CreateDeviceIndependentResources()
    {
        HRESULT hr = S_OK;

        // Create a Direct2D factory.
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

        return hr;
    }


    // Initialize device-dependent resources
    HRESULT CreateDeviceResources()
    {
        HRESULT hr = S_OK;

        if (!m_pRenderTarget)
        {
            RECT rc;
            GetClientRect(m_hwnd, &rc);

            D2D1_SIZE_U size = D2D1::SizeU(
                rc.right - rc.left,
                rc.bottom - rc.top
            );

            // Create a Direct2D render target
            hr = m_pDirect2dFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(m_hwnd, size),
                &m_pRenderTarget
            );

            if (SUCCEEDED(hr))
            {
                // Create a gray brush
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                    &m_pLightSlateGrayBrush
                );
            }
            if (SUCCEEDED(hr))
            {
                // Create a blue brush
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                    &m_pCornflowerBlueBrush
                );
            }
        }

        return hr;
    }


    // Release device-dependent resource
    void DiscardDeviceResources()
    {
        SafeRelease(&m_pRenderTarget);
        SafeRelease(&m_pLightSlateGrayBrush);
        SafeRelease(&m_pCornflowerBlueBrush);
    }


    // Draw content
    HRESULT OnRender()
    {
        HRESULT hr = S_OK;

        hr = CreateDeviceResources();

        if (SUCCEEDED(hr))
        {
            m_pRenderTarget->BeginDraw();

            m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

            m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

            D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

            // Draw a grid background
            int width = static_cast<int>(rtSize.width);
            int height = static_cast<int>(rtSize.height);

            for (int x = 0; x < width; x += 10)
            {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                    D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
                    m_pLightSlateGrayBrush,
                    0.5f
                );
            }

            for (int y = 0; y < height; y += 10)
            {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                    D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
                    m_pLightSlateGrayBrush,
                    0.5f
                );
            }

            // Draw two rectangles
            D2D1_RECT_F rectangle1 = D2D1::RectF(
                rtSize.width / 2 - 50.0f,
                rtSize.height / 2 - 50.0f,
                rtSize.width / 2 + 50.0f,
                rtSize.height / 2 + 50.0f
            );

            D2D1_RECT_F rectangle2 = D2D1::RectF(
                rtSize.width / 2 - 100.0f,
                rtSize.height / 2 - 100.0f,
                rtSize.width / 2 + 100.0f,
                rtSize.height / 2 + 100.0f
            );

            // Draw a filled rectangle
            m_pRenderTarget->FillRectangle(&rectangle1, m_pLightSlateGrayBrush);

            // Draw the outline of a rectangle
            m_pRenderTarget->DrawRectangle(&rectangle2, m_pCornflowerBlueBrush);

            hr = m_pRenderTarget->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                hr = S_OK;
                DiscardDeviceResources();
            }
        
        }

        return hr;
    }


    // Resize the render target
    void OnResize(
        UINT width,
        UINT height
    )
    {
        if (m_pRenderTarget)
        {
            // Note: This method can fail, but it's okay to ignore the
            // error here, because the error will be returned again
            // the next time EndDraw is called
            m_pRenderTarget->Resize(D2D1::SizeU(width, height));
        }
    }


    // The windows procedure
    static LRESULT CALLBACK WndProc(
        HWND hwnd, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
    )
    {
        LRESULT result = 0;

        if (message == WM_CREATE)
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;

            ::SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pDemoApp)
            );

            result = 1;
        }
        else
        {
            DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(static_cast<LONG_PTR>(
                ::GetWindowLongPtrW(
                    hwnd,
                    GWLP_USERDATA
                )));

            bool wasHandled = false;

            if (pDemoApp)
            {
                switch (message)
                {
                case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pDemoApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_PAINT:
                {
                    pDemoApp->OnRender();
                    ValidateRect(hwnd, NULL);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
                }
            }

            if (!wasHandled)
            {
                result = DefWindowProc(hwnd, message, wParam, lParam);
            }
        }

        return result;
    }


};



int WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
)
{
    // Use HeapSetInformation to specify that the process should
    // terminate if the heap manager detects an error in any heap used
    // by the process
    // The return value is ignored, because we want to continue running in the
    // unlikely event that HeapSetInformation fails
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            DemoApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}