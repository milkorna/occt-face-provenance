#include "ModelViewer.h"

#include <Windows.h>

#include <AIS_DisplayMode.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Handle.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_NameOfColor.hxx>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <WNT_Window.hxx>

#include <stdexcept>

namespace
{
constexpr wchar_t WINDOW_CLASS_NAME[]{L"ViewerWindow"};
constexpr wchar_t WINDOW_TITLE[]{L"Viewer"};
constexpr int WINDOW_WIDTH{900};
constexpr int WINDOW_HEIGHT{700};
} // namespace

ModelViewer::ModelViewer(const TopoDS_Shape& shape)
{
    if (shape.IsNull())
    {
        throw std::invalid_argument("Cannot display null shape");
    }

    createWindow();
    initializeViewer();
    displayShape(shape);
}

void ModelViewer::createWindow()
{
    const HINSTANCE instance{GetModuleHandleW(nullptr)};

    WNDCLASSW windowClass{};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = ModelViewer::windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    if (RegisterClassW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        throw std::runtime_error("Failed to register viewer window class");
    }

    m_windowHandle = CreateWindowExW(0, WINDOW_CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                     CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, instance, this);

    if (m_windowHandle == nullptr)
    {
        throw std::runtime_error("Failed to create viewer window");
    }

    ShowWindow(m_windowHandle, SW_SHOW);
    UpdateWindow(m_windowHandle);
}

void ModelViewer::initializeViewer()
{
    const Handle(Aspect_DisplayConnection) displayConnection{new Aspect_DisplayConnection{}};

    m_graphicDriver = new OpenGl_GraphicDriver{displayConnection};

    m_viewer = new V3d_Viewer{m_graphicDriver};
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_context = new AIS_InteractiveContext{m_viewer};

    m_view = m_viewer->CreateView();

    const Handle(WNT_Window) window{new WNT_Window{reinterpret_cast<Aspect_Handle>(m_windowHandle)}};

    m_view->SetWindow(window);

    if (!window->IsMapped())
    {
        window->Map();
    }

    m_view->SetBackgroundColor(Quantity_Color{Quantity_NOC_GRAY});
    m_view->SetProj(V3d_XposYposZpos);
}

void ModelViewer::displayShape(const TopoDS_Shape& shape)
{
    m_shapePresentation = new AIS_Shape{shape};
    m_shapePresentation->SetDisplayMode(AIS_Shaded);

    m_context->Display(m_shapePresentation, false);

    m_view->FitAll();
    m_view->Redraw();
}

void ModelViewer::run()
{
    MSG message{};

    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

LRESULT CALLBACK ModelViewer::windowProcedure(const HWND windowHandle, const UINT message, const WPARAM wParam,
                                              const LPARAM lParam)
{
    ModelViewer* viewer{reinterpret_cast<ModelViewer*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA))};

    if (message == WM_NCCREATE)
    {
        const CREATESTRUCTW* createStruct{reinterpret_cast<CREATESTRUCTW*>(lParam)};
        viewer = static_cast<ModelViewer*>(createStruct->lpCreateParams);

        SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(viewer));
    }

    switch (message)
    {
    case WM_SIZE:
        if (viewer != nullptr && !viewer->m_view.IsNull())
        {
            viewer->m_view->MustBeResized();
        }

        return 0;

    case WM_PAINT:
        if (viewer != nullptr && !viewer->m_view.IsNull())
        {
            PAINTSTRUCT paintStruct{};
            BeginPaint(windowHandle, &paintStruct);

            viewer->m_view->Redraw();

            EndPaint(windowHandle, &paintStruct);
        }

        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(windowHandle, message, wParam, lParam);
    }
}