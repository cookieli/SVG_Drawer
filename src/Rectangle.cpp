//
// Created by lzx on 12/28/19.
//

#include "Rectangle.h"
#include <cmath>
using namespace std;
namespace CMU462{
    using point = Rectangle::point ;
    Rectangle Rectangle::next_box() {
        int axis_split = sqrt(split);
        int y_part = box_count/axis_split;
        int x_part = box_count % axis_split;
        int split_box_x_length = ceil((topRight.first - topLeft.first)/axis_split);
        int split_box_y_length = ceil((topLeft.second - bottomLeft.second)/axis_split);
        point box_bottomLeft = make_pair(floor(bottomLeft.first + x_part*split_box_x_length) + 0.5,
                                         floor(bottomLeft.second + y_part*split_box_y_length) + 0.5);
        float bottomRightXindex = ceil(box_bottomLeft.first + split_box_x_length);
        if(x_part == axis_split - 1) bottomRightXindex = topRight.first;
        point box_bottomRight =make_pair(bottomRightXindex,
                                         box_bottomLeft.second);
        float topLeftYindex = ceil(box_bottomLeft.second + split_box_y_length);
        if(y_part == axis_split - 1) topLeftYindex = topLeft.second;
        point box_topLeft = make_pair(box_bottomLeft.first,
                                      topLeftYindex);
        point box_topRight = make_pair(box_bottomRight.first,
                                       topLeftYindex);
        Rectangle small_box(box_topLeft, box_topRight, box_bottomRight, box_bottomLeft);
        box_count++;
        return small_box;
    }
}