#include "core/gl/oxgl.h"

#include "nanovg.h"

#define NANOVG_GLES2_IMPLEMENTATION
//!!! or  #define NANOVG_GL2_IMPLEMENTATION  if you building for PC!!!

#define glLinkProgram oxglLinkProgram
#define glActiveTexture oxglActiveTexture
#define glBindBuffer oxglBindBuffer
#define glDeleteBuffers oxglDeleteBuffers
#define glGenBuffers oxglGenBuffers
#define glBufferData oxglBufferData
#define glStencilOpSeparate oxglStencilOpSeparate
#define glAttachShader oxglAttachShader
#define glBindAttribLocation oxglBindAttribLocation
#define glCompileShader oxglCompileShader
#define glCreateProgram oxglCreateProgram
#define glCreateShader oxglCreateShader
#define glDeleteProgram oxglDeleteProgram
#define glDeleteShader oxglDeleteShader
#define glDisableVertexAttribArray oxglDisableVertexAttribArray
#define glEnableVertexAttribArray oxglEnableVertexAttribArray
#define glGetProgramiv oxglGetProgramiv
#define glGetProgramInfoLog oxglGetProgramInfoLog
#define glGetShaderiv oxglGetShaderiv
#define glGetShaderInfoLog oxglGetShaderInfoLog
#define glGetUniformLocation oxglGetUniformLocation
#define glShaderSource oxglShaderSource
#define glUseProgram oxglUseProgram
#define glUniform1i oxglUniform1i
#define glUniform2fv oxglUniform2fv
#define glUniform4fv oxglUniform4fv
#define glVertexAttribPointer oxglVertexAttribPointer
#define glBindRenderbuffer oxglBindRenderbuffer
#define glDeleteRenderbuffers oxglDeleteRenderbuffers
#define glGenRenderbuffers oxglGenRenderbuffers
#define glRenderbufferStorage oxglRenderbufferStorage
#define glBindFramebuffer oxglBindFramebuffer
#define glDeleteFramebuffers oxglDeleteFramebuffers
#define glGenFramebuffers oxglGenFramebuffers
#define glCheckFramebufferStatus oxglCheckFramebufferStatus
#define glFramebufferTexture2D oxglFramebufferTexture2D
#define glFramebufferRenderbuffer oxglFramebufferRenderbuffer
#define glGenerateMipmap oxglGenerateMipmap
#define glBlendFuncSeparate oxglBlendFuncSeparate


#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"
#include "Material.h"
