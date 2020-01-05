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
  translation1(0, 2) -= centerX;
  translation1(1, 2) -= centerY;
  Matrix3x3 scale = Matrix3x3::identity();
  scale(0, 0)     /= 2*vspan;
  scale(1, 1)     /= 2*vspan;
  Matrix3x3 translation2 = Matrix3x3::identity();
  translation2(0, 2) += 0.5;
  translation2(1, 2) += 0.5;
  set_svg_2_norm(translation2*scale*translation1);

}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->centerX -= dx;
  this->centerY -= dy;
  this->vspan *= scale;
  set_viewbox( centerX, centerY, vspan );
}

} // namespace CMU462
