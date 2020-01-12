//
// Created by lzx on 12/28/19.
//

#ifndef DRAWSVG_RECTANGLE_H
#define DRAWSVG_RECTANGLE_H

#include <utility>
#include <vector>
#include <memory>
//#include "software_renderer.h"

namespace CMU462{
    class Rectangle {
    public:
        using point = std::pair<float, float>;
        friend class SoftwareRendererImp;
        Rectangle(std::vector<point> points):topLeft(points[0]), topRight(points[1]),
                                             bottomRight(points[2]), bottomLeft(points[3]),
                                             split(25), box_count(0),point_vec(points){};
        Rectangle(point topLeft, point topRight, point bottomRight, point bottomLeft):
        topLeft(topLeft), topRight(topRight), bottomLeft(bottomLeft), bottomRight(bottomRight),split(25), box_count(0){
            point_vec.push_back(topLeft);
            point_vec.push_back(topRight);
            point_vec.push_back(bottomRight);
            point_vec.push_back(bottomLeft);
        }
        virtual ~Rectangle(){}
        std::vector<Rectangle> split_boxes(int slt){
            split = slt;
        };
        Rectangle next_box();
        bool has_next_box(){ return box_count < split;}
        bool has_split(){return split > 1;}
        std::vector<point> to_vec(){return point_vec;}



    private:
        point topLeft;
        point topRight;
        point bottomRight;
        point bottomLeft;
        int split;
        int box_count;
        std::vector<point> point_vec;
    };
}


#endif //DRAWSVG_RECTANGLE_H
