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

class TrackedShape;

/**
 * @brief Displays tracked shape in interactive OCCT viewer
 */
class ModelViewer
{
  public:
    /**
     * @brief Constructs viewer for tracked shape
     * @param trackedShape Shape to display
     */
    explicit ModelViewer(const TrackedShape& trackedShape);

    ~ModelViewer() noexcept;

    ModelViewer(const ModelViewer&) = delete;
    ModelViewer& operator=(const ModelViewer&) = delete;
    ModelViewer(ModelViewer&&) = delete;
    ModelViewer& operator=(ModelViewer&&) = delete;

    /**
     * @brief Runs viewer message loop
     */
    void run();

  private:
    /**
     * @brief Handles Win32 window messages
     * @param windowHandle Window handle
     * @param message Window message
     * @param wParam Message parameter
     * @param lParam Message parameter
     * @return Window procedure result
     */
    static LRESULT CALLBACK windowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Creates native viewer window
     */
    void createWindow();

    /**
     * @brief Initializes OCCT visualization resources
     */
    void initializeViewer();

    /**
     * @brief Displays shape in viewer
     * @param shape Shape to display
     */
    void displayShape(const TopoDS_Shape& shape);

    /**
     * @brief Updates information for detected face
     * @param x Cursor X coordinate
     * @param y Cursor Y coordinate
     */
    void updateDetectedFaceInfo(const int x, const int y);

    /**
     * @brief Clears detected face information
     */
    void clearDetectedFaceInfo();

    /**
     * @brief Releases OCCT visualization resources
     */
    void releaseViewerResources() noexcept;

    /**
     * @brief Destroys native viewer window
     */
    void destroyWindow() noexcept;

    const TrackedShape& m_trackedShape;

    HWND m_windowHandle{nullptr};

    Handle(OpenGl_GraphicDriver) m_graphicDriver{};
    Handle(V3d_Viewer) m_viewer{};
    Handle(V3d_View) m_view{};
    Handle(AIS_InteractiveContext) m_context{};
    Handle(AIS_Shape) m_shapePresentation{};

    int m_detectedFaceIndex{0};
    bool m_isRunning{false};
};