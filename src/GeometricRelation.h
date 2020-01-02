//
// Created by lzx on 12/30/19.
//

#ifndef DRAWSVG_GEOMETRICRELATION_H
#define DRAWSVG_GEOMETRICRELATION_H

#include "software_renderer.h"

namespace CMU462 {
    using point= SoftwareRenderer::point;
    class GeometricRelation {
    public:
        enum PointLineRelation {ON_LINE, IN_LINE, OUT_LINE};
        enum PointTriangleRelation{
            IN_TRIANGLE,
            ON_TRIANGLE,
            OUT_TRIANGLE
        };
        enum LineRelation {
            CROSS_INTERSECTION,
            PARALLEL,
            NON_INTERSECTION,
        };
        enum BoxTriangleRelation{
            INSIDE_TRIANGLE,
            OUTSIDE_TRIANGLE,
            CROSS_TRIANGLE
        };

        static bool onSegment(point p, point a, point b);
        static LineRelation lineRelation(point a, point b, point c, point d);
        static PointLineRelation pointLineRelation(point p, point line_start, point line_end);
        static PointTriangleRelation point_in_triangle
                                    (point p, std::vector<SoftwareRenderer::point> &anti_clock);
        static BoxTriangleRelation box_triangle_relation(std::vector<point> &anticlockTriangle,
                                                  Rectangle r);


    };
}


#endif //DRAWSVG_GEOMETRICRELATION_H
