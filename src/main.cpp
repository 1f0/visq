#include <GL/freeglut.h>
#include <iostream>
using namespace std;
using namespace Eigen;
using Mat = MatXXd;

int main(){
  string equation = 'x^2+y^2+z^2=1';
  Mat tris(3, 100);
  Mat pts(3, 100);

  //partition = >cells

  //fit polygon in cells
}