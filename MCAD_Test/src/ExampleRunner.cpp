#include "ExampleRunner.h"

#include "BooleanFeature.h"
#include "ExtrudeFeature.h"
#include "ModelViewer.h"
#include "Sketch.h"
#include "TrackedShape.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{
gp_Pln makeSketchPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& xDirection)
{
    return gp_Pln{gp_Ax3{origin, normal, xDirection}};
}

void printFaceOrigins(const TrackedShape& trackedShape)
{
    int faceIndex{1};

    for (TopExp_Explorer explorer{trackedShape.shape(), TopAbs_FACE}; explorer.More(); explorer.Next())
    {
        const TopoDS_Face& face{TopoDS::Face(explorer.Current())};
        const WireIdSet& origins{trackedShape.faceOrigins(face)};

        std::cout << "Face " << faceIndex << ":";

        if (origins.empty())
        {
            std::cout << " no source wire";
        }

        for (const int wireId : origins)
        {
            std::cout << "\n\tWire " << wireId;
        }

        std::cout << '\n';
        ++faceIndex;
    }
}

void showResult(const TrackedShape& result)
{
    printFaceOrigins(result);

    ModelViewer viewer{result};
    viewer.run();
}
} // namespace

void ExampleRunner::runBasicSubtract()
{
    constexpr double extrusionLength{10.0};
    Sketch sketch1{gp_Pln{gp::XOY()}};

    const int wire1Id = sketch1.addClosedWire({
        gp_Pnt2d{0.0, 0.0},
        gp_Pnt2d{10.0, 0.0},
        gp_Pnt2d{10.0, 10.0},
        gp_Pnt2d{0.0, 10.0},
    });

    ExtrudeFeature extrude1{sketch1, wire1Id, extrusionLength};
    const TrackedShape& body{extrude1.result()};

    Sketch sketch2{makeSketchPlane(gp_Pnt{15.0, 0.0, 0.0}, gp::DX(), gp::DY())};

    const int wire2Id = sketch2.addClosedWire({
        gp_Pnt2d{2.0, 12.0},
        gp_Pnt2d{8.0, 12.0},
        gp_Pnt2d{8.0, 6.0},
        gp_Pnt2d{2.0, 6.0},
    });

    ExtrudeFeature extrude2{sketch2, wire2Id, -extrusionLength};
    const TrackedShape& tool{extrude2.result()};

    BooleanFeature subtract{BooleanType::Subtract, body, tool};
    const TrackedShape& result{subtract.result()};

    showResult(result);
}

void ExampleRunner::runUnionWithThreeHoles()
{
    constexpr double baseExtrusionLength{12.0};
    constexpr double holeExtrusionLength{14.0};

    const std::vector<gp_Pnt2d> profilePoints{
        gp_Pnt2d{0.0, 0.0},
        gp_Pnt2d{10.0, 0.0},
        gp_Pnt2d{10.0, 10.0},
        gp_Pnt2d{0.0, 10.0},
    };

    Sketch sketch1{makeSketchPlane(gp_Pnt{0.0, 0.0, 0.0}, gp::DX(), gp::DY())};
    const int wire1Id = sketch1.addClosedWire(profilePoints);

    ExtrudeFeature extrude1{sketch1, wire1Id, baseExtrusionLength};
    const TrackedShape& body1{extrude1.result()};

    Sketch sketch2{makeSketchPlane(gp_Pnt{20.0, 0.0, 10.0}, gp_Dir{-1.0, 0.0, 0.0}, gp::DY())};

    const int wire2Id = sketch2.addClosedWire(profilePoints);

    ExtrudeFeature extrude2{sketch2, wire2Id, baseExtrusionLength};
    const TrackedShape& body2{extrude2.result()};

    BooleanFeature unionFeature{BooleanType::Union, body1, body2};
    const TrackedShape& combinedBody{unionFeature.result()};

    Sketch holeSketch{makeSketchPlane(gp_Pnt{0.0, 0.0, -2.0}, gp::DZ(), gp::DX())};

    const int hole1Id = holeSketch.addClosedWire({
        gp_Pnt2d{3.0, 3.0},
        gp_Pnt2d{5.0, 3.0},
        gp_Pnt2d{5.0, 8.0},
        gp_Pnt2d{3.0, 8.0},
    });

    const int hole2Id = holeSketch.addClosedWire({
        gp_Pnt2d{9.0, 3.0},
        gp_Pnt2d{11.0, 3.0},
        gp_Pnt2d{10.0, 8.0},
    });

    const int hole3Id = holeSketch.addClosedWire({
        gp_Pnt2d{15.0, 3.0},
        gp_Pnt2d{17.0, 3.0},
        gp_Pnt2d{18.0, 6.0},
        gp_Pnt2d{16.0, 8.0},
        gp_Pnt2d{14.0, 6.0},
    });

    ExtrudeFeature holeExtrude1{holeSketch, hole1Id, holeExtrusionLength};
    const TrackedShape& holeTool1{holeExtrude1.result()};

    BooleanFeature subtract1{BooleanType::Subtract, combinedBody, holeTool1};
    const TrackedShape& result1{subtract1.result()};

    ExtrudeFeature holeExtrude2{holeSketch, hole2Id, holeExtrusionLength};
    const TrackedShape& holeTool2{holeExtrude2.result()};

    BooleanFeature subtract2{BooleanType::Subtract, result1, holeTool2};
    const TrackedShape& result2{subtract2.result()};

    ExtrudeFeature holeExtrude3{holeSketch, hole3Id, holeExtrusionLength};
    const TrackedShape& holeTool3{holeExtrude3.result()};

    BooleanFeature subtract3{BooleanType::Subtract, result2, holeTool3};
    const TrackedShape& result{subtract3.result()};

    showResult(result);
}

void ExampleRunner::runUntrackedBaseWithCut()
{
    BRepPrimAPI_MakeBox boxMaker{gp::Origin(), gp_Pnt{20.0, 12.0, 8.0}};
    boxMaker.Build();
    if (!boxMaker.IsDone())
    {
        throw std::runtime_error("Failed to build untracked base box");
    }

    const TopoDS_Shape baseShape{boxMaker.Shape()};
    const TrackedShape base{baseShape};

    Sketch holeSketch{makeSketchPlane(gp_Pnt{0.0, 0.0, -2.0}, gp::DZ(), gp::DX())};

    const int holeWireId = holeSketch.addClosedWire({
        gp_Pnt2d{8.0, 3.0},
        gp_Pnt2d{12.0, 3.0},
        gp_Pnt2d{14.0, 6.0},
        gp_Pnt2d{12.0, 9.0},
        gp_Pnt2d{8.0, 9.0},
        gp_Pnt2d{6.0, 6.0},
    });

    ExtrudeFeature holeExtrude{holeSketch, holeWireId, 12.0};
    const TrackedShape& tool{holeExtrude.result()};

    BooleanFeature subtract{BooleanType::Subtract, base, tool};
    const TrackedShape& result{subtract.result()};

    showResult(result);
}

void ExampleRunner::runTripleUnionWithThroughHole()
{
    constexpr double extrusionLength{15.0};

    const std::vector<gp_Pnt2d> profilePoints{
        gp_Pnt2d{0.0, 0.0},
        gp_Pnt2d{10.0, 0.0},
        gp_Pnt2d{10.0, 10.0},
        gp_Pnt2d{0.0, 10.0},
    };

    Sketch sketch1{gp_Pln{gp::XOY()}};
    const int wire1Id = sketch1.addClosedWire(profilePoints);

    ExtrudeFeature extrude1{sketch1, wire1Id, extrusionLength};
    const TrackedShape& body1{extrude1.result()};

    Sketch sketch2{makeSketchPlane(gp_Pnt{0.0, 0.0, 5.0}, gp::DZ(), gp::DX())};
    const int wire2Id = sketch2.addClosedWire(profilePoints);

    ExtrudeFeature extrude2{sketch2, wire2Id, extrusionLength};
    const TrackedShape& body2{extrude2.result()};

    BooleanFeature union1{BooleanType::Union, body1, body2};
    const TrackedShape& result1{union1.result()};

    Sketch sketch3{makeSketchPlane(gp_Pnt{0.0, 0.0, 10.0}, gp::DZ(), gp::DX())};
    const int wire3Id = sketch3.addClosedWire(profilePoints);

    ExtrudeFeature extrude3{sketch3, wire3Id, extrusionLength};
    const TrackedShape& body3{extrude3.result()};

    BooleanFeature union2{BooleanType::Union, result1, body3};
    const TrackedShape& combinedBody{union2.result()};

    const int holeWireId = sketch1.addClosedWire({
        gp_Pnt2d{2.0, 2.0},
        gp_Pnt2d{8.0, 2.0},
        gp_Pnt2d{8.0, 8.0},
        gp_Pnt2d{2.0, 8.0},
    });

    ExtrudeFeature holeExtrude{sketch1, holeWireId, 30.0};
    const TrackedShape& holeTool{holeExtrude.result()};

    BooleanFeature subtract{BooleanType::Subtract, combinedBody, holeTool};
    const TrackedShape& result{subtract.result()};

    showResult(result);
}