#include "BooleanFeature.h"
#include "ExtrudeFeature.h"
#include "ModelViewer.h"
#include "Sketch.h"
#include "TrackedShape.h"

#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#include <iostream>

namespace
{
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
            std::cout << "\n\t Wire " << wireId;
        }

        std::cout << '\n';
        ++faceIndex;
    }
}
} // namespace

int main()
{
    Sketch sketch1{1, gp_Pln{gp::XOY()}};

    sketch1.addClosedWire(1, {
                                 gp_Pnt2d(0.0, 0.0),
                                 gp_Pnt2d(10.0, 0.0),
                                 gp_Pnt2d(10.0, 10.0),
                                 gp_Pnt2d(0.0, 10.0),
                             });

    ExtrudeFeature extrude1{1, sketch1, 1, 10.0};
    const TrackedShape& body{extrude1.result()};

    const gp_Ax3 sketch2Axis{
        gp_Pnt{15.0, 0.0, 0.0},
        gp_Dir{1.0, 0.0, 0.0},
        gp_Dir{0.0, 1.0, 0.0},
    };

    Sketch sketch2{2, gp_Pln{sketch2Axis}};

    sketch2.addClosedWire(2, {
                                 gp_Pnt2d{2.0, 12.0},
                                 gp_Pnt2d{8.0, 12.0},
                                 gp_Pnt2d{8.0, 6.0},
                                 gp_Pnt2d{2.0, 6.0},
                             });

    ExtrudeFeature extrude2{2, sketch2, 2, -10.0};
    const TrackedShape& tool{extrude2.result()};

    BooleanFeature cut{3, BooleanType::Subtract, body, tool};
    const TrackedShape& result{cut.result()};

    printFaceOrigins(result);

    ModelViewer viewer{result};
    viewer.run();

    return 0;
}