// http://www.seas.upenn.edu/~cis565/fbo.htm
// http://marina.sys.wakayama-u.ac.jp/~tokoi/?date=20051008

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include "glsl.h"

using namespace std;

vector<unsigned char> read_ppm(const char* filename,
    int* width, int* height) {
  FILE* fp = fopen(filename, "r");
  int type, max, p[3];
  char buf[128];
  if (fp == NULL) {
    fprintf(stderr, "error: can't open '%s'\n", filename);
    exit(1);
  }
  fscanf(fp, "P%d\n", &type);
  fgets(buf, 128, fp); // skip comment
  fscanf(fp, "%d %d", width, height);
  fscanf(fp, "%d", &max);
  const int n = (*width)*(*height)*4;
  vector<unsigned char> dat;
  dat.reserve(n);
  for (int i=0; i<n; i+=4) {
    fscanf(fp, "%d %d %d", p, p+1, p+2);
    dat[i+0] = p[0];
    dat[i+1] = p[1];
    dat[i+2] = p[2];
    dat[i+3] = 0;
  }
  fclose(fp);
  return dat;
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
        w,                // width
        h,                // height
        0,                // border
        GL_RGBA,          // foramt
        GL_UNSIGNED_BYTE, // type
        dat               // data
        );
  }

  ~texture() {
    glDeleteTextures(1, &name);
  }
};

struct frame_buffer_object {
  GLuint name;
  frame_buffer_object() {
    glGenFramebuffers(1, &name);
    glBindFramebuffer(GL_FRAMEBUFFER, name);
  }
  ~frame_buffer_object() {
    glDeleteFramebuffers(1, &name);
  }

  void attach(const texture& tex) {
    // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glFramebufferTexture2D.xml
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        tex.name,
        0
        );
  }
};

struct shader {
  shader() {
    GLint compiled, linked;

    /* シェーダオブジェクトの作成 */
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    /* シェーダのソースプログラムの読み込み */
    if (readShaderSource(vertShader, "texture.vert")) exit(1);
    if (readShaderSource(fragShader, "texture.frag")) exit(1);

    /* バーテックスシェーダのソースプログラムのコンパイル */
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
    printShaderInfoLog(vertShader);
    if (compiled == GL_FALSE) {
      fprintf(stderr, "Compile error in vertex shader.¥n");
      exit(1);
    }

    /* フラグメントシェーダのソースプログラムのコンパイル */
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
    printShaderInfoLog(fragShader);
    if (compiled == GL_FALSE) {
      fprintf(stderr, "Compile error in fragment shader.¥n");
      exit(1);
    }

    /* プログラムオブジェクトの作成 */
    GLuint gl2Program = glCreateProgram();

    /* シェーダオブジェクトのシェーダプログラムへの登録 */
    glAttachShader(gl2Program, vertShader);
    glAttachShader(gl2Program, fragShader);

    /* シェーダオブジェクトの削除 */
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    /* シェーダプログラムのリンク */
    glLinkProgram(gl2Program);
    glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
    printProgramInfoLog(gl2Program);
    if (linked == GL_FALSE) {
      fprintf(stderr, "Link error.¥n");
      exit(1);
    }

    /* シェーダプログラムの適用 */
    glUseProgram(gl2Program);

    /* テクスチャユニット０を指定する */
    glUniform1i(glGetUniformLocation(gl2Program, "texture"), 0);
    glActiveTexture(GL_TEXTURE0);
  }
};

void initGL(int* argc, char** argv) {
  // set up glut to get valid GL context and
  // get extension entry points
  glutInit(argc, argv);
  glutCreateWindow("TEST1");

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

  glslInit();
}

void resize(int w, int h) {
  // viewport transform for 1:1 pixel=texel=data mapping
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //gluOrtho2D(0.0,texSize,0.0,texSize);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0,0,w,h);
}

void execute() {
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
}

int main(int argc, char **argv) {
  initGL(&argc, argv);

  if (argc != 2) {
    fprintf(stderr, "usage: %s IN_PPM\n", argv[0]);
    exit(1);
  }

  int w, h;
  vector<unsigned char> data = read_ppm(argv[1], &w, &h);
  vector<unsigned char> result;
  result.reserve(w*h*4);

  resize(w, h);
  shader sh;

  // create FBO and bind it (that is, use offscreen render target)
  frame_buffer_object fbo;
  // create output texture
  texture tex_out(w, h);
  // attach texture
  fbo.attach(tex_out);
  // create input texture
  texture tex_in(w, h, &data[0]);
  // execute caclulation
  execute();
  // and read back
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &result[0]);

  write_ppm("out.ppm", w, h, &result[0]);

  return 0;
}
