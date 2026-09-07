#include "ModelViewer.h"

#include "TrackedShape.h"

#include <Windows.h>
#include <windowsx.h>

#include <AIS_DisplayMode.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Handle.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_NameOfColor.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Standard_Handle.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <WNT_Window.hxx>

#include <stdexcept>
#include <string>

namespace
{
constexpr wchar_t kWindowClassName[]{L"ViewerWindow"};
constexpr wchar_t kWindowTitle[]{L"Viewer"};
constexpr int kWindowWidth{900};
constexpr int kWindowHeight{700};

int findFaceIndex(const TrackedShape& trackedShape, const TopoDS_Face& face)
{
    int faceIndex{1};

    for (TopExp_Explorer explorer{trackedShape.shape(), TopAbs_FACE}; explorer.More(); explorer.Next())
    {
        const TopoDS_Face& currentFace{TopoDS::Face(explorer.Current())};

        if (currentFace.IsSame(face))
        {
            return faceIndex;
        }

        ++faceIndex;
    }

    return 0;
}
} // namespace

ModelViewer::ModelViewer(const TrackedShape& trackedShape)
    : m_trackedShape{trackedShape}
{
    try
    {
        const TopoDS_Shape& shape{m_trackedShape.shape()};

        createWindow();
        initializeViewer();
        displayShape(shape);

        ShowWindow(m_windowHandle, SW_SHOW);
        UpdateWindow(m_windowHandle);
    }
    catch (...)
    {
        releaseViewerResources();
        destroyWindow();
        throw;
    }
}

ModelViewer::~ModelViewer() noexcept
{
    releaseViewerResources();
    destroyWindow();
}

void ModelViewer::run()
{
    if (m_windowHandle == nullptr)
    {
        throw std::logic_error("Viewer window is not available");
    }

    m_isRunning = true;

    try
    {
        MSG message{};

        while (true)
        {
            const BOOL messageResult{GetMessageW(&message, nullptr, 0, 0)};

            if (messageResult == -1)
            {
                throw std::runtime_error("Failed to retrieve viewer window message");
            }

            if (messageResult == 0)
            {
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    catch (...)
    {
        m_isRunning = false;
        throw;
    }

    m_isRunning = false;
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
    {
        if (viewer != nullptr && !viewer->m_view.IsNull())
        {
            viewer->m_view->MustBeResized();
        }

        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT paintStruct{};
        BeginPaint(windowHandle, &paintStruct);

        if (viewer != nullptr && !viewer->m_view.IsNull())
        {
            viewer->m_view->Redraw();
        }

        EndPaint(windowHandle, &paintStruct);
        return 0;
    }
    case WM_MBUTTONDOWN:
    {
        if (viewer != nullptr && !viewer->m_view.IsNull())
        {
            SetCapture(windowHandle);

            viewer->m_context->ClearDetected(true);
            viewer->clearDetectedFaceInfo();

            viewer->m_view->StartRotation(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }

        return 0;
    }
    case WM_MOUSEMOVE:
    {
        if (viewer != nullptr && !viewer->m_view.IsNull())
        {
            const int x{GET_X_LPARAM(lParam)};
            const int y{GET_Y_LPARAM(lParam)};

            if ((wParam & MK_MBUTTON) != 0)
            {
                viewer->m_view->Rotation(x, y);
            }
            else
            {
                viewer->updateDetectedFaceInfo(x, y);
            }
        }

        return 0;
    }
    case WM_MBUTTONUP:
    {
        if (GetCapture() == windowHandle)
        {
            ReleaseCapture();
        }

        return 0;
    }
    case WM_ERASEBKGND:
    {
        return 1;
    }
    case WM_DESTROY:
    {
        if (viewer != nullptr && viewer->m_isRunning)
        {
            PostQuitMessage(0);
        }

        return 0;
    }
    case WM_NCDESTROY:
    {
        if (viewer != nullptr)
        {
            viewer->m_windowHandle = nullptr;
        }

        SetWindowLongPtrW(windowHandle, GWLP_USERDATA, 0);

        return DefWindowProcW(windowHandle, message, wParam, lParam);
    }
    default:
    {
        return DefWindowProcW(windowHandle, message, wParam, lParam);
    }
    }
}

void ModelViewer::createWindow()
{
    const HINSTANCE instance{GetModuleHandleW(nullptr)};

    WNDCLASSW windowClass{};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = ModelViewer::windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kWindowClassName;

    if (RegisterClassW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        throw std::runtime_error("Failed to register viewer window class");
    }

    m_windowHandle = CreateWindowExW(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                     CW_USEDEFAULT, kWindowWidth, kWindowHeight, nullptr, nullptr, instance, this);

    if (m_windowHandle == nullptr)
    {
        throw std::runtime_error("Failed to create viewer window");
    }
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

    const Handle(Prs3d_Drawer)& drawer{m_shapePresentation->Attributes()};
    drawer->SetFaceBoundaryDraw(true);

    m_context->Display(m_shapePresentation, false);

    m_context->Deactivate(m_shapePresentation);
    m_context->Activate(m_shapePresentation, AIS_Shape::SelectionMode(TopAbs_FACE));

    m_view->FitAll();
    m_view->Redraw();
}

void ModelViewer::updateDetectedFaceInfo(const int x, const int y)
{
    m_context->MoveTo(x, y, m_view, true);

    if (!m_context->HasDetected())
    {
        clearDetectedFaceInfo();
        return;
    }

    const Handle(SelectMgr_EntityOwner)& detectedOwner{m_context->DetectedOwner()};
    const Handle(StdSelect_BRepOwner) brepOwner{Handle(StdSelect_BRepOwner)::DownCast(detectedOwner)};

    if (brepOwner.IsNull() || !brepOwner->HasShape())
    {
        clearDetectedFaceInfo();
        return;
    }

    const TopoDS_Shape& detectedShape{brepOwner->Shape()};

    if (detectedShape.ShapeType() != TopAbs_FACE)
    {
        clearDetectedFaceInfo();
        return;
    }

    const TopoDS_Face& face{TopoDS::Face(detectedShape)};
    const int faceIndex{findFaceIndex(m_trackedShape, face)};

    if (faceIndex == 0)
    {
        clearDetectedFaceInfo();
        return;
    }

    if (faceIndex == m_detectedFaceIndex)
    {
        return;
    }

    const WireIdSet& origins{m_trackedShape.faceOrigins(face)};

    std::wstring title{L"Face "};
    title += std::to_wstring(faceIndex);
    title += L", Wires:";

    if (origins.empty())
    {
        title += L" none";
    }
    else
    {
        auto iterator = origins.begin();

        title += L" ";
        title += std::to_wstring(*iterator);
        ++iterator;

        for (; iterator != origins.end(); ++iterator)
        {
            title += L", ";
            title += std::to_wstring(*iterator);
        }
    }

    SetWindowTextW(m_windowHandle, title.c_str());

    m_detectedFaceIndex = faceIndex;
}

void ModelViewer::clearDetectedFaceInfo()
{
    if (m_detectedFaceIndex == 0)
    {
        return;
    }

    m_detectedFaceIndex = 0;
    SetWindowTextW(m_windowHandle, kWindowTitle);
}

void ModelViewer::releaseViewerResources() noexcept
{
    m_shapePresentation.Nullify();
    m_context.Nullify();
    m_view.Nullify();
    m_viewer.Nullify();
    m_graphicDriver.Nullify();
}

void ModelViewer::destroyWindow() noexcept
{
    if (m_windowHandle == nullptr)
    {
        return;
    }

    if (IsWindow(m_windowHandle))
    {
        DestroyWindow(m_windowHandle);
    }

    m_windowHandle = nullptr;
}
