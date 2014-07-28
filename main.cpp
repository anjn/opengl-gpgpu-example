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
    printf("%s",(char*)glewGetErrorString(err));
    exit(1);
  }
}

void read_ppm(const char* filename,
    int* width, int* height, unsigned char* dat) {
  FILE* fp = fopen(filename, "r");
  int type, max, p[3];
  char buf[128];
  if (fp == NULL) {
    printf("can't open '%s'\n", filename);
    exit(1);
  }
  fscanf(fp, "P%d\n", &type);
  fgets(buf, 128, fp); // skip comment
  fscanf(fp, "%d %d", width, height);
  fscanf(fp, "%d", &max);
  const int n = (*width)*(*height)*4;
  for (int i=0; i<n; i+=4) {
    fscanf(fp, "%d %d %d", p, p+1, p+2);
    dat[i+0] = p[0];
    dat[i+1] = p[1];
    dat[i+2] = p[2];
    dat[i+3] = 0;
  }
  fclose(fp);
}

void write_ppm(const char* filename,
    int width, int height, unsigned char* dat) {
  FILE* fp = fopen(filename, "w");
  fprintf(fp, "P3\n%d %d\n255\n", width, height);
  const int n = width*height*4;
  for (int i=0; i<n; i+=4) {
    fprintf(fp, "%d %d %d\n", dat[i+0], dat[i+1], dat[i+2]);
  }
  fclose(fp);
}

struct texture {
  GLuint name;
  texture(int w, int h, unsigned char* dat = NULL) {
    // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGenTextures.xml
    glGenTextures(
        1,    // n, the number of textures to be generated
        &name // textures
        );
    glBindTexture(GL_TEXTURE_2D, name);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // define texture with floating point format
    // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexImage2D.xml
    glTexImage2D(
        GL_TEXTURE_2D,    // target
        0,                // level
        GL_RGBA,          // internalformat
        texSize,          // width
        texSize,          // height
        0,                // border
        GL_RGBA,          // foramt
        GL_UNSIGNED_BYTE, // type
        dat               // data
        );
  }

  ~texture() {
    glDeleteTextures (1,&name);
  }
};

int main(int argc, char **argv) {
  // set up glut to get valid GL context and
  // get extension entry points
  glutInit(&argc, argv);
  glutCreateWindow("TEST1");

  initGLEW();

  // declare texture size, the actual data will be a vector
  // of size texSize*texSize*4
  int texSize = 512;
  // create test data
  unsigned char* data   = new unsigned char[4*texSize*texSize];
  unsigned char* result = new unsigned char[4*texSize*texSize];

  int w, h;
  read_ppm("/Users/jun/work/test01.ppm", &w, &h, data);

  // viewport transform for 1:1 pixel=texel=data mapping
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //gluOrtho2D(0.0,texSize,0.0,texSize);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0,0,texSize,texSize);

  // create FBO and bind it (that is, use offscreen render target)
  GLuint fb;
  glGenFramebuffers(1, &fb);
  glBindFramebuffer(GL_FRAMEBUFFER, fb);

  // create texture
  GLuint tex;
  // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGenTextures.xml
  glGenTextures(
      1,   // n, the number of textures to be generated
      &tex // textures
      );
  glBindTexture(GL_TEXTURE_2D, tex);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // define texture with floating point format
  // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexImage2D.xml
  glTexImage2D(
      GL_TEXTURE_2D,    // target
      0,                // level
      GL_RGBA,          // internalformat
      texSize,          // width
      texSize,          // height
      0,                // border
      GL_RGBA,          // foramt
      GL_UNSIGNED_BYTE, // type
      0                 // data
      );

  // attach texture
  // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glFramebufferTexture2D.xml
  glFramebufferTexture2D(
      GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D,
      tex,
      0
      );

  // copy from cpu memory to gpu texture
  // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexSubImage2D.xml
  //glTexSubImage2D(
  //    GL_TEXTURE_2D,    // target
  //    0,                // level
  //    0,                // xoffset
  //    0,                // yoffset
  //    texSize,          // width
  //    texSize,          // height
  //    GL_RGBA,          // format
  //    GL_UNSIGNED_BYTE, // type
  //    data              // data
  //    );

  //glClearColor(0.3f, 0.3f, 1.0f, 0.5f);
  //glDisable(GL_LIGHTING);
  //glDisable(GL_DEPTH_TEST);

  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* １枚の４角形を描く */
  glNormal3d(0.0, 0.0, 1.0);
  glBegin(GL_QUADS);
  glTexCoord2d(0.0, 1.0);
  glVertex3d(-1.0, -1.0,  0.0);
  glTexCoord2d(1.0, 1.0);
  glVertex3d( 1.0, -1.0,  0.0);
  glTexCoord2d(1.0, 0.0);
  glVertex3d( 1.0,  1.0,  0.0);
  glTexCoord2d(0.0, 0.0);
  glVertex3d(-1.0,  1.0,  0.0);
  glEnd();

  glFlush();

  // and read back
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glReadPixels(0, 0, texSize, texSize,GL_RGBA,GL_UNSIGNED_BYTE,result);

  // // print out results
  // printf("Data before roundtrip:\n");
  // for (int y=0; y<texSize; y++) {
  //   for (int x=0; x<texSize; x++) {
  //     for (int i=0; i<4; i++)
  //       printf("%02x", data[texSize*4*y+4*x+i]);
  //     printf(" ");
  //   }
  //   printf("\n");
  // }
  // printf("Data after roundtrip:\n");
  // for (int i=0; i<texSize*texSize*4; i++) printf("%d\n",result[i]);

  //write_ppm("out.ppm", texSize, texSize, result);
  write_ppm("out.ppm", texSize, texSize, data);

  // clean up
  delete data;
  delete result;
  glDeleteFramebuffers(1,&fb);
  glDeleteTextures (1,&tex);

  return 0;
}
