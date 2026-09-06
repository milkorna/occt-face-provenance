#include "BooleanFeature.h"
#include "ExtrudeFeature.h"
#include "Sketch.h"
#include "TrackedShape.h"

#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

// const gp_Pnt point1(0., 0., 0.);
// const gp_Pnt point2(10., 0., 0.);
// const gp_Pnt point3(10., 10., 0.);
// const gp_Pnt point4(0., 10., 0.);

// const gp_Pnt point1(15., 2., 12.);
// const gp_Pnt point2(15., 8., 12.);
// const gp_Pnt point3(15., 8., 6.);
// const gp_Pnt point4(15., 2., 6.);

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
        gp_Dir{-1.0, 0.0, 0.0},
        gp_Dir{0.0, 1.0, 0.0},
    };

    Sketch sketch2{2, gp_Pln{sketch2Axis}};

    sketch2.addClosedWire(2, {
                                 gp_Pnt2d{2.0, 12.0},
                                 gp_Pnt2d{8.0, 12.0},
                                 gp_Pnt2d{8.0, 6.0},
                                 gp_Pnt2d{2.0, 6.0},
                             });

    ExtrudeFeature extrude2{2, sketch2, 2, 10.0};
    const TrackedShape& tool{extrude2.result()};

    BooleanFeature cut{3, BooleanType::Subtract, body, tool};
    const TrackedShape& result{cut.result()};

    return 0;
}