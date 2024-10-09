#pragma once
#include <windows.h>
#include <cstdint>
#include <GL/gl.h>

struct CTexture {
   GLuint id;
   GLenum format;
   GLenum min_filter;
   GLenum mag_filter;
   uint16_t w, h;
 };

 int png_to_gl_texture(struct CTexture * tex, char const * const filename);

