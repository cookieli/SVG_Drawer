//
// Created by lzx on 12/30/19.
//

#include "GeometricRelation.h"
namespace CMU462 {
    using namespace std;
    bool GeometricRelation::onSegment(CMU462::point p, CMU462::point a, CMU462::point b) {
        return p.first <= max(a.first, b.first) && p.first >= min(a.first, b.first) &&
               p.second <= max(a.second, b.second) && p.second >= min(a.second, b.second);
    }

    GeometricRelation::LineRelation  GeometricRelation::lineRelation(point a, point b, point c, point d) {
        float a1 = b.second - a.second;
        float b1 = a.first - b.first;
        float c1 = a.first * a1 + a.second * b1;

        float a2 = d.second - c.second;
        float b2 = c.first - d.first;
        float c2 = c.first * a2 + c.second * b2;
        float determinant = a1 * b2 - a2 * b1;
        if (determinant == 0) {
            return PARALLEL;
        } else {
            float inter_x = (b2 * c1 - b1 * c2) / determinant;
            float inter_y = (a1 * c2 - a2 * c1) / determinant;
            point p = make_pair(inter_x, inter_y);
            if (onSegment(p, a, b) && onSegment(p, c, d)){
                return CROSS_INTERSECTION;
            } else {
                return NON_INTERSECTION;
            }
        }
    }
    GeometricRelation::PointLineRelation GeometricRelation::pointLineRelation(point p, point line_start,
                                                                              point line_end){
        float dx = line_end.first - line_start.first;
        float dy = line_end.second - line_start.second;
        float dpoint_x = p.first - line_start.first;
        float dpoint_y = p.second - line_start.second;
        int point_line_relation =-dpoint_x * dy + dpoint_y * dx;
        if (point_line_relation > 0)      return IN_LINE;
        else if(point_line_relation == 0) return ON_LINE;
        else                              return OUT_LINE;
    }

    GeometricRelation::PointTriangleRelation  GeometricRelation::point_in_triangle(point p,
                                                                                   vector<point> &anti_clock){
        PointLineRelation  r1 = pointLineRelation(p, anti_clock[0], anti_clock[1]);
        PointLineRelation  r2 = pointLineRelation(p, anti_clock[1], anti_clock[2]);
        PointLineRelation  r3 = pointLineRelation(p, anti_clock[2], anti_clock[0]);
        if(r1 == ON_LINE || r2 == ON_LINE || r3 == ON_LINE){
            return ON_TRIANGLE;
        }
        if(r1 == OUT_LINE || r2 == OUT_LINE || r3 == OUT_LINE){
            return OUT_TRIANGLE;
        }
        return IN_TRIANGLE;

    }
    GeometricRelation::BoxTriangleRelation
    GeometricRelation::box_triangle_relation(vector<point> &anticlockTriangle,
                                               Rectangle r) {
        int in_triangle_cnt = 0,
                on_triangle_cnt = 0,
                out_triangle_cnt= 0;
        for(point p: r.to_vec()){
            if(point_in_triangle(p, anticlockTriangle) == ON_TRIANGLE){
                on_triangle_cnt +=1;
            } else if(point_in_triangle(p, anticlockTriangle) == IN_TRIANGLE){
                in_triangle_cnt += 1;
            } else {
                out_triangle_cnt +=1;
            }
        }
        if(in_triangle_cnt+ on_triangle_cnt == r.to_vec().size()){
            return INSIDE_TRIANGLE;
        } else {
            for(int i = 0; i < r.to_vec().size(); i++){
                point p1 = r.to_vec()[i];
                point p2 = r.to_vec()[(i+1)%r.to_vec().size()];
                for(int j = 0; j < anticlockTriangle.size(); j++) {
                    LineRelation  lr = lineRelation(p1, p2, anticlockTriangle[j], anticlockTriangle[(j+1) % anticlockTriangle.size()]);
                    if (lr== CROSS_INTERSECTION ) {
                        return  CROSS_TRIANGLE;
                    }
                }

            }
            if(in_triangle_cnt == 0)   return  OUTSIDE_TRIANGLE;
            else                       return  CROSS_TRIANGLE;
        }
    }

}