#include "viewport.h"

#include "CMU462.h"

namespace CMU462 {

void ViewportImp::set_viewbox( float centerX, float centerY, float vspan ) {

  // Task 5 (part 2): 
  // Set svg coordinate to normalized device coordinate transformation. Your input
  // arguments are defined as normalized SVG canvas coordinates.
  this->centerX = centerX;
  this->centerY = centerY;
  this->vspan = vspan;
  Matrix3x3 translation1 = Matrix3x3::identity();
  translation1(0, 2) -= centerX-vspan;
  translation1(1, 2) -= centerY-vspan;
//  translation1(0, 2) += vspan;
//  translation1(1, 2) += vspan;
//  Matrix3x3 scale = Matrix3x3::identity();
  translation1(0, 0)     /= 2*vspan;
  translation1(1, 1)     /= 2*vspan;
  translation1(0, 2)     /= 2*vspan;
  translation1(1, 2)     /= 2*vspan;
  set_svg_2_norm(translation1);

}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->centerX -= dx;
  this->centerY -= dy;
  this->vspan *= scale;
  set_viewbox( centerX, centerY, vspan );
}

} // namespace CMU462
