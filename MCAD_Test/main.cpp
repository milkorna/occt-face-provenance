#include <gp.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>

#include "ExtrudeFeature.h"
#include "Sketch.h"
#include "TrackedShape.h"

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
    Sketch sketch1(1, gp_Pln(gp::XOY()));

    sketch1.addClosedWire(1, {
                                 gp_Pnt2d(0.0, 0.0),
                                 gp_Pnt2d(10.0, 0.0),
                                 gp_Pnt2d(10.0, 10.0),
                                 gp_Pnt2d(0.0, 10.0),
                             });

    ExtrudeFeature extrude1(1, sketch1, 1, 10.0);

    const TrackedShape& body = extrude1.result();

    return 0;
}