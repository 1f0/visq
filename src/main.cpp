#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <Eigen/Dense>
#include <math.h>
#include <AntTweakBar.h>
#include <stdlib.h>

using namespace std;
using namespace Eigen;
using Vec3 = Vector3f;
using Vec4 = Vector4f;

float max0(float a) {
  if (a > 0)return a;
  return 0;
}

float max1(float a) {
  if (a < 1)return a;
  return 1;
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


// Shapes scale
float g_Zoom = 1.0f;
// Shape orientation (stored as a quaternion)
float g_Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };

// Routine to convert a quaternion to a 4x4 matrix
// ( input: quat = float[4]  output: mat = float[4*4] )
Matrix4f QuaternionToMatrix(const float *quat)
{
  float yy2 = 2.0f * quat[1] * quat[1];
  float xy2 = 2.0f * quat[0] * quat[1];
  float xz2 = 2.0f * quat[0] * quat[2];
  float yz2 = 2.0f * quat[1] * quat[2];
  float zz2 = 2.0f * quat[2] * quat[2];
  float wz2 = 2.0f * quat[3] * quat[2];
  float wy2 = 2.0f * quat[3] * quat[1];
  float wx2 = 2.0f * quat[3] * quat[0];
  float xx2 = 2.0f * quat[0] * quat[0];

  Matrix4f mat;
  mat(0, 0) = -yy2 - zz2 + 1.0f;
  mat(0, 1) = xy2 + wz2;
  mat(0, 2) = xz2 - wy2;
  mat(0, 3) = 0;
  mat(1, 0) = xy2 - wz2;
  mat(1, 1) = -xx2 - zz2 + 1.0f;
  mat(1, 2) = yz2 + wx2;
  mat(1, 3) = 0;
  mat(2, 0) = xz2 + wy2;
  mat(2, 1) = yz2 - wx2;
  mat(2, 2) = -xx2 - yy2 + 1.0f;
  mat(2, 3) = 0;
  mat(3, 0) = mat(3, 1) = mat(3, 2) = 0;
  mat(3, 3) = 1;

  return mat;
}

// Return elapsed time in milliseconds
int GetTimeMs()
{
#if !defined(_WIN32)
  return glutGet(GLUT_ELAPSED_TIME);
#else
  return (int)GetTickCount();
#endif
}

float para[10] = {3, 0, 0, 0,  //ABCD
                  4, 0, 0,     //EFG
                  5, 0, -1     //HIJ
                 };
Vec3 light(0.57735f, 0.57735f, 0.57735f);

Vec3 raycast(float x, float y) {
  Matrix4f Q;
  Q <<  para[0], para[1], para[2], para[3],
  para[1], para[4], para[5], para[6],
  para[2], para[5], para[7], para[8],
  para[3], para[6], para[8], para[9];

  Matrix4f R = QuaternionToMatrix(g_Rotation);
  Matrix4f Z = Matrix4f::Identity() / g_Zoom;
  Z(3,3) = 1;

  Vec4 C(0, 0, 1, 0);
  Vec4 X(x, y, 0, 1);

  Matrix4f QRZ = Z.transpose() * R.transpose() * Q * R * Z;

  float a = C.transpose() * QRZ * C;
  float b = 2 * X.transpose() * QRZ * C;
  float c = X.transpose() * QRZ * X;

  float delta = b * b - 4 * a * c;
  if (delta < 0)
    return Vec3(0, 0, 0);
  float z, z2;

  if (a == 0) {//highlight z=[-\inf, \inf]
    if (fabs(c) < 0.005)
      z2 = z = 0;
    else
      return Vec3(0, 0, 0);
  } else {
    z = (-b + sqrt(delta)) / (2 * a);
    z2 = (-b - sqrt(delta)) / (2 * a);
  }

  Vec4 RZX = R * Z * Vec4(x, y, z, 1);
  Vec4 RZX2 = R * Z * Vec4(x, y, z2, 1);
  bool back = false;
  const float limit = 0.8;


  if(fabs(RZX[0]) > limit || fabs(RZX[1]) > limit || fabs(RZX[2]) > limit){
    RZX = RZX2;
    back = true;
  }

  if (fabs(RZX[0]) > limit || fabs(RZX[1]) > limit || fabs(RZX[2]) > limit)
    return Vec3(0,0,0);

  float ambient = 0.2;

  // if(back)
  //   return ambient * Vec3(1, 1, 0);

  Vec3 n =  (Q * R * Z * Vec4(x, y, z, 1)).head(3).normalized();

  Vec3 light_dir = (light * 2 - RZX.head(3)).normalized();

  float diffuse = max0(n.dot(light_dir));

  if(diffuse == 0)
    return ambient * Vec3(1, 1, 0);

  Vec3 reflect = (2 * n.dot(light_dir) * n - light_dir);
  Vec3 view(0, 0, 1);

  float specular = pow(max0(view.dot(reflect)), 8) * 0.4;

  float intensity = max1(ambient + 0.5 * diffuse);
  Vec3 color = intensity * Vec3(1, 1, 0) + specular * Vec3(1, 1, 1);
  for (size_t i = 0; i < 3; ++i) {
    color[i] = max1(color[i]);
  }

  return color;
}

void draw_quadric(Buffer& buf) {
  light.normalize();
  for (size_t i = 0; i < buf.w; ++i) {
    for (size_t j = 0; j < buf.h; ++j) {
      float cx = (i * 2.0 / buf.w - 1);
      float cy = (j * 2.0 / buf.h - 1);
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

  // Draw tweak bars
  TwDraw();

  glutSwapBuffers();
  glutPostRedisplay();
}

void change_size(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  glOrtho(-1, 1, -1, 1, -2, 2);
  glMatrixMode(GL_MODELVIEW);
  // Send the new window size to AntTweakBar
  TwWindowSize(w, h);
}

void my_terminate(void) {
  TwTerminate();
}

// Routine to set a quaternion from a rotation axis and angle
// ( input axis = float[3] angle = float  output: quat = float[4] )
void SetQuaternionFromAxisAngle(const float *axis, float angle, float *quat)
{
  float sina2, norm;
  sina2 = (float)sin(0.5f * angle);
  norm = (float)sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
  quat[0] = sina2 * axis[0] / norm;
  quat[1] = sina2 * axis[1] / norm;
  quat[2] = sina2 * axis[2] / norm;
  quat[3] = (float)cos(0.5f * angle);
}

// Routine to multiply 2 quaternions (ie, compose rotations)
// ( input q1 = float[4] q2 = float[4]  output: qout = float[4] )
void MultiplyQuaternions(const float *q1, const float *q2, float *qout)
{
  float qr[4];
  qr[0] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
  qr[1] = q1[3]*q2[1] + q1[1]*q2[3] + q1[2]*q2[0] - q1[0]*q2[2];
  qr[2] = q1[3]*q2[2] + q1[2]*q2[3] + q1[0]*q2[1] - q1[1]*q2[0];
  qr[3]  = q1[3]*q2[3] - (q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2]);
  qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}

void rotate_around_axis(float* axis, float angle){
  float quat[4];
  SetQuaternionFromAxisAngle(axis, angle, quat);
  float g_RotateTemp[4];
  MultiplyQuaternions(quat, g_Rotation, g_RotateTemp);
  memcpy(g_Rotation, g_RotateTemp, sizeof(g_RotateTemp));
}

void inc_Lng(void *p){
  float axis[3] = { 0, 1, 0 };
  float angle = M_PI * 1/30.0f;
  rotate_around_axis(axis, angle);
}

void inc_Lat(void *p){
  float axis[3] = { 1, 0, 0 };
  float angle = M_PI * 1/30.0f;
  rotate_around_axis(axis, angle);
}

void dec_Lng(void *p){
  float axis[3] = { 0, 1, 0 };
  float angle = M_PI * -1/30.0f;
  rotate_around_axis(axis, angle);
}

void dec_Lat(void *p){
  float axis[3] = { 1, 0, 0 };
  float angle = M_PI * -1/30.0f;
  rotate_around_axis(axis, angle);
}

void reset_ang(void *p){
  fill(g_Rotation, g_Rotation+3, 0);
  g_Rotation[3] = 1;
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
  atexit(my_terminate);

  TwInit(TW_OPENGL, NULL);
  // Set GLUT event callbacks
  glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
  glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
  glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
  glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
  glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
  // - Send 'glutGetModifers' function pointer to AntTweakBar;
  //   required because the GLUT key event functions do not report key modifiers states.
  TwGLUTModifiersFunc(glutGetModifiers);

  TwBar *bar = TwNewBar("TweakBar"); // Pointer to the tweak bar
  TwDefine(" TweakBar size='180 360' color='96 216 224' "); // change default tweak bar size and color

  // Add 'g_Zoom' to 'bar': this is a modifable (RW) variable of type TW_TYPE_FLOAT. Its key shortcuts are [z] and [Z].
  TwAddVarRW(bar, "Zoom", TW_TYPE_FLOAT, &g_Zoom,
             " min=0.01 max=2.5 step=0.01 keyIncr=Z keyDecr=z help='Scale the object (1=original size).' ");

  // Add 'g_Rotation' to 'bar': this is a variable of type TW_TYPE_QUAT4F which defines the object's orientation
  TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &g_Rotation,
             " label='Rotation' opened=true help='Change the object orientation.' ");

  TwDefine(" GLOBAL help='This example shows Ax^2+2Bxy+2Cxz+2Dx+Ey^2+2Fyz+Gy+Hz^2+2Iz+J=0.' ");

  TwAddVarRW(bar, "A", TW_TYPE_FLOAT, para,   " group='Equation' ");
  TwAddVarRW(bar, "B", TW_TYPE_FLOAT, para+1, " group='Equation' ");
  TwAddVarRW(bar, "C", TW_TYPE_FLOAT, para+2, " group='Equation' ");
  TwAddVarRW(bar, "D", TW_TYPE_FLOAT, para+3, " group='Equation' ");
  TwAddVarRW(bar, "E", TW_TYPE_FLOAT, para+4, " group='Equation' ");
  TwAddVarRW(bar, "F", TW_TYPE_FLOAT, para+5, " group='Equation' ");
  TwAddVarRW(bar, "G", TW_TYPE_FLOAT, para+6, " group='Equation' ");
  TwAddVarRW(bar, "H", TW_TYPE_FLOAT, para+7, " group='Equation' ");
  TwAddVarRW(bar, "I", TW_TYPE_FLOAT, para+8, " group='Equation' ");
  TwAddVarRW(bar, "J", TW_TYPE_FLOAT, para+9, " group='Equation' ");

  TwAddVarRW(bar, "Light", TW_TYPE_DIR3F, light.data(), 
               " label='Inv light dir' help='Change the light direction.' ");

  TwAddButton(bar, "inc_Lng", inc_Lng, NULL, "key=d group='angle'");
  TwAddButton(bar, "dec_Lng", dec_Lng, NULL, "key=a group='angle'");
  TwAddButton(bar, "inc_Lat", inc_Lat, NULL, "key=s group='angle'");
  TwAddButton(bar, "dec_Lat", dec_Lat, NULL, "key=w group='angle'");
  TwAddButton(bar, "reset", reset_ang, NULL, "key=r group='angle'");

  glutMainLoop();
  return 0;
}
