#include "texture.h"
#include "color.h"

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

namespace CMU462 {

inline void uint8_to_float( float dst[4], unsigned char* src ) {
  uint8_t* src_uint8 = (uint8_t *)src;
  dst[0] = src_uint8[0] / 255.f;
  dst[1] = src_uint8[1] / 255.f;
  dst[2] = src_uint8[2] / 255.f;
  dst[3] = src_uint8[3] / 255.f;
}

inline void float_to_uint8( unsigned char* dst, float src[4] ) {
  uint8_t* dst_uint8 = (uint8_t *)dst;
  dst_uint8[0] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[0])));
  dst_uint8[1] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[1])));
  dst_uint8[2] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[2])));
  dst_uint8[3] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[3])));
}

void Sampler2DImp::generate_mips(Texture& tex, int startLevel) {

  // NOTE: 
  // This starter code allocates the mip levels and generates a level 
  // map by filling each level with placeholder data in the form of a 
  // color that differs from its neighbours'. You should instead fill
  // with the correct data!

  // Task 7: Implement this

  // check start level
  if ( startLevel >= tex.mipmap.size() ) {
    std::cerr << "Invalid start level"; 
  }

  // allocate sublevels
  int baseWidth  = tex.mipmap[startLevel].width;
  int baseHeight = tex.mipmap[startLevel].height;
  int numSubLevels = (int)(log2f( (float)max(baseWidth, baseHeight)));

  numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
  tex.mipmap.resize(startLevel + numSubLevels + 1);

  int width  = baseWidth;
  int height = baseHeight;
  for (int i = 1; i <= numSubLevels; i++) {

    MipLevel& level = tex.mipmap[startLevel + i];

    // handle odd size texture by rounding down
    width  = max( 1, width  / 2); assert(width  > 0);
    height = max( 1, height / 2); assert(height > 0);

    level.width = width;
    level.height = height;
    level.texels = vector<unsigned char>(4 * width * height);
  }


  for(size_t i = 1; i < tex.mipmap.size(); ++i) {
      MipLevel &pre_mip = tex.mipmap[i-1];
      MipLevel &cur_mip = tex.mipmap[i];
      for (size_t w = 0; w < cur_mip.width; ++w){
          size_t pre_w = 2 * w;
          for(size_t h = 0; h < cur_mip.height; ++h){
              size_t pre_h = 2 * h;
              uint16_t r = 0,g = 0,b = 0,a = 0;
              for(int x = 0; x < 2; x++){
                  for(int y = 0; y < 2; y++){
                      r += pre_mip.texels[4 * (pre_w + x + (pre_h +y) * pre_mip.width)];
                      g += pre_mip.texels[4 * (pre_w + x + (pre_h +y) * pre_mip.width)+1];
                      b += pre_mip.texels[4 * (pre_w + x + (pre_h +y) * pre_mip.width)+2];
                      a += pre_mip.texels[4 * (pre_w + x + (pre_h +y) * pre_mip.width)+3];
                  }
              }
              cur_mip.texels[4 * (w + h* cur_mip.width)    ]   = r/4;
              cur_mip.texels[4 * (w + h* cur_mip.width) + 1]   = g/4;
              cur_mip.texels[4 * (w + h* cur_mip.width) + 2]   = b/4;
              cur_mip.texels[4 * (w + h* cur_mip.width) + 3]   = a/4;
          }
      }
  }

}

Color Sampler2DImp::sample_nearest(Texture& tex, 
                                   float u, float v, 
                                   int level) {

  // Task 6: Implement nearest neighbour interpolation
  
  // return magenta for invalid level
  if(level > tex.mipmap.size())  return Color(1,0,1,1);
  auto mip = tex.mipmap[level];
  int x = round(u * mip.width  - 0.5f);
  int y = round(v * mip.height - 0.5f);
  Color c;
  uint8_to_float(&c.r, &mip.texels[4*(x + y *mip.width)]);
  return c;

}

Color Sampler2DImp::sample_bilinear(Texture& tex, 
                                    float u, float v, 
                                    int level) {
  
  // Task 6: Implement bilinear filtering

  // return magenta for invalid level
    if(level > tex.mipmap.size())  return Color(1,0,1,1);
    auto& mip = tex.mipmap[level];
    u = u * mip.width - 0.5f;
    v = v * mip.height - 0.5f;
    size_t x = floor(u);
    size_t y = floor(v);
    Color c1, c2, c3, c4;
    uint8_to_float(&c1.r, &mip.texels[4 * (x     + mip.width * y)]);
    uint8_to_float(&c2.r, &mip.texels[4 * (x + 1 + mip.width * y)]);
    uint8_to_float(&c3.r, &mip.texels[4 * (x     + mip.width * (y + 1))]);
    uint8_to_float(&c4.r, &mip.texels[4 * (x + 1 + mip.width * (y + 1))]);
    float u_ratio = u - x;
    float u_opposite = 1 - u_ratio;
    float v_ratio    = v - y;
    float v_opposite = 1 - v_ratio;
//    auto u0 = u_opposite * c3 + u_ratio * c4;
//    auto u1 = u_opposite * c1 + u_ratio * c2;
    return v_opposite * (u_opposite * c1 + u_ratio * c2)+
           v_ratio    * (u_opposite * c3 + u_ratio * c4) ;

}

Color Sampler2DImp::sample_trilinear(Texture& tex, 
                                     float u, float v, 
                                     float u_scale, float v_scale) {

  // Task 7: Implement trilinear filtering

  // return magenta for invalid level
  float d = max(0.0f, log2f(max(u_scale, v_scale)));
  int l0 = floor(d), l1 = l0 + 1;
  float a = l1 - d, b = 1 - a;
  Color c0 = sample_bilinear(tex, u, v, l0);
  Color c1 = sample_bilinear(tex, u, v, l1);

  return c0*a + b*c1;

}

} // namespace CMU462
