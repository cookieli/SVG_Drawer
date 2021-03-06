#include "software_renderer.h"

#include <cmath>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <algorithm>
#include <utility>
#include "triangulation.h"
#include "Rectangle.h"
#include "GeometricRelation.h"

using namespace std;

namespace CMU462 {


// Implements SoftwareRenderer //

void SoftwareRendererImp::draw_svg( SVG& svg ) {

  // set top level transformation
  transformation = svg_2_screen;
  auto old_transformation = transformation;

  // draw all elements
  for ( size_t i = 0; i < svg.elements.size(); ++i ) {
    draw_element(svg.elements[i]);
    transformation = old_transformation;
  }

  // draw canvas outline
  Vector2D a = transform(Vector2D(    0    ,     0    )); a.x--; a.y--;
  Vector2D b = transform(Vector2D(svg.width,     0    )); b.x++; b.y--;
  Vector2D c = transform(Vector2D(    0    ,svg.height)); c.x--; c.y++;
  Vector2D d = transform(Vector2D(svg.width,svg.height)); d.x++; d.y++;

  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to render target
  resolve();

}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->sample_rate = sample_rate;
  this->sample_h = this->target_h * this->sample_rate;
  this->sample_w = this->target_w * this->sample_rate;
//  this->supersample_render_target.resize(4 * this->sample_w * this->sample_h);
  this->reset_buffer();
}

void SoftwareRendererImp::set_render_target( unsigned char* render_target,
                                             size_t width, size_t height ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->render_target = render_target;
  this->target_w = width;
  this->target_h = height;
  this->sample_h = this->target_h * this->sample_rate;
  this->sample_w = this->target_w * this->sample_rate;
  this->reset_buffer();
//  this->supersample_render_target.resize(4 * this->sample_w * this->sample_h);
//  this->color_buffer.resize(this->sample_w * this->sample_h);
}

void SoftwareRendererImp::draw_element( SVGElement* element ) {

  // Task 5 (part 1):
  // Modify this to implement the transformation stack
  transformation = transformation * element->transform;
  switch(element->type) {
    case POINT:
      draw_point(static_cast<Point&>(*element));
      break;
    case LINE:
      draw_line(static_cast<Line&>(*element));
      break;
    case POLYLINE:
      draw_polyline(static_cast<Polyline&>(*element));
      break;
    case RECT:
      draw_rect(static_cast<Rect&>(*element));
      break;
    case POLYGON:
      draw_polygon(static_cast<Polygon&>(*element));
      break;
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse&>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image&>(*element));
      break;
    case GROUP:
      draw_group(static_cast<Group&>(*element));
      break;
    default:
      break;
  }

}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) { 

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;
  
  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));
  
  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
    rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Extra credit 

}

void SoftwareRendererImp::draw_image( Image& image ) {

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

    auto old_transformation = transformation;
    for (size_t i = 0; i < group.elements.size(); ++i) {
        draw_element(group.elements[i]);
        transformation = old_transformation;
    }
}

// Rasterization //

// The input arguments in the rasterization functions 
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

    // fill in the nearest pixel
//    x *= sample_rate;
//    y *= sample_rate;
    int sx = (int) floor(x);
    int sy = (int) floor(y);

    // check bounds
    if (sx < 0 || sx >= sample_w) return;
    if (sy < 0 || sy >= sample_h) return;
    for (int x = sx; x < sx + sample_rate; x++) {
        for (int y = sy; y < sy + sample_rate; y++) {
            int pos = 4 * (x + y * sample_w);
            int color_pos = x + y * sample_w;
            Color canvas_color = color_buffer[color_pos];
            Color new_color;
            new_color.a = 1 - (1 - color.a) * (1 - canvas_color.a);
            new_color.r = (1 - color.a) * canvas_color.r + color.r;
            new_color.g = (1 - color.a) * canvas_color.g + color.g;
            new_color.b = (1 - color.a) * canvas_color.b + color.b;
            color_buffer[color_pos] = new_color;
            supersample_render_target[pos    ] = (uint8_t) (new_color.r * 255);
            supersample_render_target[pos + 1] = (uint8_t) (new_color.g * 255);
            supersample_render_target[pos + 2] = (uint8_t) (new_color.b * 255);
            supersample_render_target[pos + 3] = (uint8_t) (new_color.a * 255);
        }
    }

    // fill sample - NOT doing alpha blending!
//  render_target[4 * (sx + sy * target_w)    ] = (uint8_t) (color.r * 255);
//  render_target[4 * (sx + sy * target_w) + 1] = (uint8_t) (color.g * 255);
//  render_target[4 * (sx + sy * target_w) + 2] = (uint8_t) (color.b * 255);
//  render_target[4 * (sx + sy * target_w) + 3] = (uint8_t) (color.a * 255);

}

void SoftwareRendererImp::fill_sample(size_t sx, size_t sy, CMU462::Color c) {
    if(sx < 0 || sx >= sample_w) return;
    if(sy < 0 || sy >= sample_h) return;
    c = c.premultiply(c.a);
    size_t pos = (size_t)4 * (sx + sy * sample_w);
    int color_pos = sx + sy * sample_w;
    Color canvas_color = color_buffer[color_pos];
    canvas_color.premultiply(canvas_color.a);
    Color new_color;
//    if(canvas_color == Color::Black){
        new_color.a = 1 - (1 - c.a) * (1 - canvas_color.a);
        new_color.r = (1 - c.a) * canvas_color.r + c.r;
        new_color.g = (1 - c.a) * canvas_color.g + c.g;
        new_color.b = (1 - c.a) * canvas_color.b + c.b;
//    } else{
//        new_color = c;
//    }
    new_color.un_premultiply(new_color.a);
    color_buffer[color_pos] = new_color;
    supersample_render_target[pos    ] = (uint8_t)(new_color.r * 255);
    supersample_render_target[pos + 1] = (uint8_t)(new_color.g * 255);
    supersample_render_target[pos + 2] = (uint8_t)(new_color.b * 255);
    supersample_render_target[pos + 3] = (uint8_t)(new_color.a * 255);
}

void SoftwareRendererImp::rasterize_point(point p, Color c){
    rasterize_point(p.first, p.second, c);
}

void SoftwareRendererImp::drawLineOfSuperSample(float x0, float y0,
                           float x1, float y1,
                           Color color){
    int sx = (int) floor(x0), sy = (int) floor(y0),
            ex = (int) floor(x1), ey = (int) floor(y1);
    if (sx > ex) {
        std::swap(sx, ex);
        std::swap(sy, ey);
    }
    int dx = ex - sx,
            dy = ey - sy,
            eps = 0;
    bool is_postive_slope = true;
    if (dy * dx < 0) is_postive_slope = false;
    bool exchange_x_y = false;
    if (abs(dy) > abs(dx)) {
        exchange_x_y = true;
        std::swap(dy, dx);
        std::swap(sx, sy);
        std::swap(ex, ey);
    }
    if (sx > ex) {
        std::swap(sx, ex);
        std::swap(sy, ey);
        dy = -dy;
        dx = -dx;
    }

    int y = sy;
    for (int x = sx; x <= ex; x++) {
        if (exchange_x_y) {
            fill_sample(y, x, color);

        }
        else {
            fill_sample(x, y, color);
        }
        eps += dy;
        if (is_postive_slope) {
            if ((eps << 1) >= dx) {
                y++;
                eps -= dx;
            }
        } else {
            if ((eps * 2) <= -dx) {
                y--;
                eps += dx;
            }
        }
    }


}

void SoftwareRendererImp::rasterize_line( float x0, float y0,
                                          float x1, float y1,
                                          Color color, bool need_anti_alias) {

  // Task 2: 
  // Implement line rasterization
    if(need_anti_alias) {
        x0 *= sample_rate;
        y0 *= sample_rate;
        x1 *= sample_rate;
        y1 *= sample_rate;
    }
    int sx = (int) floor(x0), sy = (int) floor(y0),
            ex = (int) floor(x1), ey = (int) floor(y1);
    if (sx > ex) {
        std::swap(sx, ex);
        std::swap(sy, ey);
    }
    int dx = ex - sx,
            dy = ey - sy,
            eps = 0;
    bool is_postive_slope = true;
    if (dy * dx < 0) is_postive_slope = false;
    bool exchange_x_y = false;
    if (abs(dy) > abs(dx)) {
        exchange_x_y = true;
        std::swap(dy, dx);
        std::swap(sx, sy);
        std::swap(ex, ey);
    }
    if (sx > ex) {
        std::swap(sx, ex);
        std::swap(sy, ey);
        dy = -dy;
        dx = -dx;
    }

    int y = sy;
    for (int x = sx; x <= ex; x++) {
        if (exchange_x_y) {
            if (need_anti_alias)  fill_sample(y, x, color);
            else                  rasterize_point(y, x, color);
        }
        else {
            if(need_anti_alias) fill_sample(x, y, color);
            else                rasterize_point(x, y, color);
        }
        eps += dy;
        if (is_postive_slope) {
            if ((eps << 1) >= dx) {
                y++;
                eps -= dx;
            }
        } else {
            if ((eps * 2) <= -dx) {
                y--;
                eps += dx;
            }
        }
    }

}

/*
 * this function returns a rectangle that just boxing the triangle
 * and return rectangle four endpoint indices.
 * the index order is leftTop, rightTop, rightBottom, leftBottom
 * */
Rectangle box_triangle(float x0, float y0,
                                             float x1, float y1,
                                             float x2, float y2){
    using point = SoftwareRenderer::point ;
    vector<point> rectangle;
    float rightmost_x = ceil(max({x0, x1, x2}));
    float leftmost_x  = floor(min({x0, x1, x2})) + 0.5;
    float top_y       = ceil(max({y0, y1, y2}));
    float bottom_y    = floor(min({y0, y1, y2})) + 0.5;
    point leftTop     = make_pair(leftmost_x, top_y);
    point rightTop    = make_pair(rightmost_x, top_y);
    point rightBottom = make_pair(rightmost_x, bottom_y);
    point leftBottom  = make_pair(leftmost_x, bottom_y);
    rectangle.push_back(leftTop);
    rectangle.push_back(rightTop);
    rectangle.push_back(rightBottom);
    rectangle.push_back(leftBottom);
    Rectangle r = Rectangle(rectangle);
    return r;

}

//untested
vector<std::pair<float, float>> anticlock_triangle_points(float x0, float y0,
                       float x1, float y1,
                       float x2, float y2){
    using point = SoftwareRenderer::point;
    point point1 = make_pair(x0, y0);
    point point2 = make_pair(x1, y1);
    point point3 = make_pair(x2, y2);
    vector<point> points;
    points.push_back(point1);
    points.push_back(point2);
    points.push_back(point3);
    return points;

}






void SoftwareRendererImp::fill_in_rectangle(Rectangle &r, Color color) {

    float start_y_index = r.bottomLeft.second;
    while(start_y_index <= r.topLeft.second){
        drawLineOfSuperSample(r.bottomLeft.first, start_y_index,
                       r.bottomRight.first, start_y_index,
                       color);
        start_y_index += 1;
    }
}

void SoftwareRendererImp::rasterize_points_in_box(Rectangle r,
                                                  vector<point> &anticlockTriangle, Color color) {

    for(float start_y_index = floor(r.bottomLeft.second) + 0.5; start_y_index <= ceil(r.topLeft.second); start_y_index++){
        for(float start_x_index = floor(r.bottomLeft.first) + 0.5; start_x_index <= ceil(r.bottomRight.first); start_x_index++){
            point p = make_pair(start_x_index, start_y_index);
            GeometricRelation::PointTriangleRelation  relation = GeometricRelation::point_in_triangle(p, anticlockTriangle);
            if (relation == GeometricRelation::IN_TRIANGLE || relation == GeometricRelation::ON_TRIANGLE){
                fill_sample(p.first,p.second, color);
            }
        }
    }
}


void SoftwareRendererImp::rasterize_triangle( float x0, float y0,
                                              float x1, float y1,
                                              float x2, float y2,
                                              Color color ) {
  // Task 3: 
  // Implement triangle rasterization

    x0 *= sample_rate;
    y0 *= sample_rate;
    x1 *= sample_rate;
    y1 *= sample_rate;
    x2 *= sample_rate;
    y2 *= sample_rate;
//    drawLineOfSuperSample(x0, y0,
//                   x1, y1,
//                   color);
//    drawLineOfSuperSample(x1, y1,
//                   x2, y2,
//                   color);
//    drawLineOfSuperSample(x2, y2,
//                   x0, y0,
//                   color);
    auto rectangle = box_triangle(x0, y0,
                                  x1, y1,
                                  x2, y2);
//  rasterize_rectangle(rectangle, color);
    auto anti_clock = anticlock_triangle_points(x0, y0,
                                                x1, y1,
                                                x2, y2);


    while (rectangle.has_next_box()) {
        Rectangle r = rectangle.next_box();
        GeometricRelation::BoxTriangleRelation relation = GeometricRelation::box_triangle_relation(anti_clock, r);
        if (relation == GeometricRelation::INSIDE_TRIANGLE) {
            fill_in_rectangle(r, color);
        } else if (relation == GeometricRelation::CROSS_TRIANGLE) {
            rasterize_points_in_box(r, anti_clock, color);
        } else if (!rectangle.has_split()) {
            rasterize_points_in_box(r, anti_clock, color);
        }
    }
}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task 6: 
  // Implement image rasterization
  x0 *= sample_rate; y0 *= sample_rate;
  x1 *= sample_rate; y1 *= sample_rate;
  float w = x1 - x0, h = y1 - y0;
  float du = 1.0f/w, dv = 1.0f/h;
  float u_scale = tex.width / w, v_scale = tex.height / h;
//  Sampler2DImp sample;
    for(float x = x0, u = 0.0f; x <= x1; x+=1.0, u += du){
        for(float y = y0, v = 0.0f; y <= y1; y+=1.0, v += dv){
          fill_sample(x, y, sampler->sample_trilinear(tex, u, v, u_scale, v_scale));
//            fill_sample(x, y, sampler->sample_bilinear(tex, u, v, 0));
      }
  }

}

// resolve samples to render target
void SoftwareRendererImp::resolve( void ) {

  // Task 4: 
  // Implement supersampling
  // You may also need to modify other functions marked with "Task 4".
  size_t sample_num = sample_rate * sample_rate;

  //first resampling
  for(size_t x = 0; x <=sample_w - sample_rate; x+= sample_rate){
      for(size_t y = 0; y<= sample_h - sample_rate; y += sample_rate){
          uint16_t r = 0, g = 0, b = 0,a = 0;
          for(size_t i = 0; i < sample_rate; i++){
              for(size_t j = 0; j < sample_rate; j++){
                  size_t pos = 4* (x+i + (y+j) * sample_w);
                  r += supersample_render_target[pos];
                  g += supersample_render_target[pos+1];
                  b += supersample_render_target[pos+2];
                  a += supersample_render_target[pos+3];

              }
          }
          r /= sample_num;
          g /= sample_num;
          b /= sample_num;
          a /= sample_num;
          size_t pos = 4 * (x/sample_rate + (y/sample_rate) * target_w);
          render_target[pos]   = (uint8_t)r;
          render_target[pos+1] = (uint8_t)g;
          render_target[pos+2] = (uint8_t)b;
          render_target[pos+3] = (uint8_t)a;
      }
  }
  return;

}


} // namespace CMU462
