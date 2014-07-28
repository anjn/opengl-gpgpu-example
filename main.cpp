// http://www.seas.upenn.edu/~cis565/fbo.htm

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

void initGLEW (void) {
  // init GLEW, obtain function pointers
  int err = glewInit();
  // Warning: This does not check if all extensions used
  // in a given implementation are actually supported.
  // Function entry points created by glewInit() will be
  // NULL in that case!
  if (GLEW_OK != err) {
    printf((char*)glewGetErrorString(err));
    exit(1);
  }
}

int main(int argc, char **argv) {
  // set up glut to get valid GL context and
  // get extension entry points
  glutInit(&argc, argv);
  glutCreateWindow("TEST1");

  initGLEW();

  // declare texture size, the actual data will be a vector
  // of size texSize*texSize*4
  int texSize = 2;
  // create test data
  float* data   = (float*)malloc(4*texSize*texSize*sizeof(float));
  float* result = (float*)malloc(4*texSize*texSize*sizeof(float));
  for (int i=0; i<texSize*texSize*4; i++) data[i] = i+1.0;

  // viewport transform for 1:1 pixel=texel=data mapping
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0,texSize,0.0,texSize);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0,0,texSize,texSize);

  // create FBO and bind it (that is, use offscreen render target)
  GLuint fb;
  glGenFramebuffersEXT(1,&fb);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fb);

  // create texture
  GLuint tex;
  glGenTextures (1, &tex);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB,tex);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // define texture with floating point format
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA32F_ARB,
      texSize,texSize,0,GL_RGBA,GL_FLOAT,0);

  // attach texture
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
      GL_COLOR_ATTACHMENT0_EXT,
      GL_TEXTURE_RECTANGLE_ARB,tex,0);

  // transfer data to texture
  glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB,0,0,0,texSize,texSize,
      GL_RGBA,GL_FLOAT,data);

  // and read back
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glReadPixels(0, 0, texSize, texSize,GL_RGBA,GL_FLOAT,result);

  // print out results
  printf("Data before roundtrip:\n");
  for (int i=0; i<texSize*texSize*4; i++)
    printf("%f\n",data[i]);
  printf("Data after roundtrip:\n");
  for (int i=0; i<texSize*texSize*4; i++)
    printf("%f\n",result[i]);

  // clean up
  free(data);
  free(result);
  glDeleteFramebuffersEXT (1,&fb);
  glDeleteTextures (1,&tex);

  return 0;
}
