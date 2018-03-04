#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <Eigen/Dense>
#include <math.h>

using namespace std;
using namespace Eigen;
using Vec3 = Vector3f;
using Vec4 = Vector4f;

float max0(float a) {
  if(a > 0)return a;
  return 0;
}

class Buffer {
  vector<float>data;
public:
  size_t w, h;
  Buffer(size_t w, size_t h): w(w), h(h), data(w * h * 3) {}
  void set(size_t i, size_t j, const Vec3& color) {
    if (i >= 0 && i < w && j >= 0 && j < h)
      for (size_t k = 0; k < 3; ++k)
        data[3 * (j * w + i) + k] = color[k];
  }
  void clear() {fill(data.begin(), data.end(), 0);}
  void resize(size_t w, size_t h) {
    data.resize(w, h);
    this->w = w;
    this->h = h;
  }
  float* raw_data() {return data.data();}
};

const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 800;

float para[10] = {1, 0, 0, 0, //ABCD
                  1, 0, 0, //EFG
                  1, 0, -1     //HIJ
                 };
Vec3 light(1, 1, 1);

Vec3 raycast(float x, float y) {
  Matrix4f Q;
  Q <<  para[0], para[1], para[2], para[3],
  para[1], para[4], para[5], para[6],
  para[2], para[5], para[7], para[8],
  para[3], para[6], para[8], para[9];
  Vec4 C(0, 0, 1, 0);
  Vec4 X(x, y, 0, 1);

  float a = C.transpose() * Q * C;
  float b = 2 * X.transpose() * Q * C;
  float c = X.transpose() * Q * X;

  float delta = b * b - 4 * a * c;
  if (delta < 0)
    return Vec3(0, 0, 0);

  float z = (-b + sqrt(delta)) / (2 * a);
  Vec3 n =  (Q * Vec4(x, y, z, 1)).normalized().head(3);

  light.normalize();
  float diffuse = max0(n.dot(light));

  Vec3 reflect = (2 * n.dot(light) * n - light);
  cout << n << endl;
  cout << n.dot(light) << endl;
  cout << reflect.norm() <<endl;;
  exit(0);
  Vec3 view(0, 0, 1);

  float ambient = 0.2;
  float specular = pow(max0(view.dot(reflect)), 4) * 2;

  cout << specular << endl;

  return (ambient + 0.5*diffuse) * Vec3(0, 0, 1) + specular * Vec3(1,1,1);
}

void draw_quadric(Buffer& buf) {
  for (size_t i = 0; i < buf.w; ++i) {
    for (size_t j = 0; j < buf.h; ++j) {
      float cx = (i * 2.0 / buf.w - 1) * 1.1;
      float cy = (j * 2.0 / buf.h - 1) * 1.1;
      if (fabs(cx) > 1 || fabs(cy) > 1)continue;
      // intesect with quardric surface
      buf.set(i, j, raycast(cx, cy));
    }
  }
}

void render_scene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(.8, 0., 0.);
  GLfloat viewport[4];
  glGetFloatv(GL_VIEWPORT, viewport);
  float w = viewport[2];
  float h = viewport[3];

  Buffer buf(w, h);
  draw_quadric(buf);
  glDrawPixels(w, h, GL_RGB, GL_FLOAT, buf.raw_data());

  glutSwapBuffers();
  glutPostRedisplay();
}

void change_size(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  glOrtho(-1, 1, -1, 1, -2, 2);
  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(10, 10);
  glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  glutCreateWindow("Quardric Surfaces");
  glClearColor(0.0, 0.0, 1.0, 1.0);

  glutDisplayFunc(render_scene);
  glutReshapeFunc(change_size);
  glutMainLoop();
  return 0;
}