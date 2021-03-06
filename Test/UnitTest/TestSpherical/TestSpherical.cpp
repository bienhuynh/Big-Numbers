#include "stdafx.h"
#include "CppUnitTest.h"
#include <Math/Spherical.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#ifdef verify
#undef verify
#endif
#define verify(expr) Assert::IsTrue(expr, _T(#expr))

namespace TestSpherical {		

  void OUTPUT(const TCHAR *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    const String msg = vformat(format, argptr);
    va_end(argptr);
    Logger::WriteMessage(msg.cstr());
  }


	TEST_CLASS(TestSpherical)	{
    public:

    TEST_METHOD(SphericalTest) {
      Point3D v;
      for (v.x = -2; v.x < 2; v.x += 0.1f) {
        for (v.y = -2; v.y < 2; v.y += 0.1f) {
          for (v.z = -2; v.z < 2; v.z += 0.1f) {
            Spherical sph(v);
            Point3D v1 = sph;
            Point3D diff = v - v1;
            const double dist = diff.length();
            verify(dist < 1e-5);
            /*
            printf("dist:%le, v:(%f,%f,%f) -> sph:(%f,%f,%f) -> v1:(%f,%f,%f)\n"
            ,dist
            ,v.x,v.y,v.z
            ,sph.r, RAD2GRAD(sph.fi), RAD2GRAD(sph.theta)
            ,v1.x,v1.y,v1.z);
            pause();
            }
            */
          }
        }
      }
    }
  };
}
