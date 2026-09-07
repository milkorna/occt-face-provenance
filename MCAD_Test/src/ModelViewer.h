#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <Standard_Handle.hxx>
#include <Standard_Macro.hxx>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <WNT_Window.hxx>

class ModelViewer
{
  public:
    explicit ModelViewer(const TopoDS_Shape& shape);

    void run();

  private:
    static LRESULT CALLBACK windowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    void createWindow();
    void initializeViewer();
    void displayShape(const TopoDS_Shape& shape);

    HWND m_windowHandle{nullptr};

    Handle(OpenGl_GraphicDriver) m_graphicDriver{};
    Handle(V3d_Viewer) m_viewer{};
    Handle(V3d_View) m_view{};
    Handle(AIS_InteractiveContext) m_context{};
    Handle(AIS_Shape) m_shapePresentation{};
};