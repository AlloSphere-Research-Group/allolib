
#include <math.h>

#include "gtest/gtest.h"

#include "al/math/al_Functions.hpp"
#include "al/math/al_Mat.hpp"
#include "al/math/al_Quat.hpp"
#include "al/math/al_Vec.hpp"
// #include "al/math/al_Random.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Frustum.hpp"
#include "al/math/al_Interval.hpp"

//  Synchronized to AlloSystem commit:
//  0ddb8ec6594ca66d34dc18849bc2b433e5f67016

using namespace al;

template <class T> inline bool eqVal(T x, T y, T eps = 0.000001) {
  return std::abs(x - y) < eps;
}

template <class T>
inline bool eqPtr(const T *x, const T *y, int n, T eps = 0.0000001) {
  for (int i = 0; i < n; ++i) {
    if (!eqVal(x[i], y[i], eps))
      return false;
  }
  return true;
}

template <class T>
inline bool eq(const Quat<T> &a, const Quat<T> &b, T eps = 0.000001) {
  return eqPtr(&a[0], &b[0], 4, eps);
}

template <int N, class T>
inline bool eq(const Vec<N, T> &a, const Vec<N, T> &b, T eps = 0.000001) {
  return eqPtr(&a[0], &b[0], N, eps);
}

template <int N, class T>
inline bool eq(const Mat<N, T> &a, const Mat<N, T> &b, T eps = 0.000001) {
  return eqPtr(&a[0], &b[0], N * N, eps);
}

// TODO break test into sections or separate tests as needed. Look at:
// https://github.com/philsquared/Catch/blob/master/docs/tutorial.md
TEST(Math, Math) {
  // Vec
  {

    // Should be able to hold objects with constructors
    { Vec<1, Vec<1, int>> t; }
    { Vec<4, Complex<float>> t; }
    { Vec<5, char> t; }
    { Vec<0, int> t; }

    const int N = 4;
    Vec<N, double> a, b, c;
    EXPECT_TRUE(a.size() == N);
    // constructors
    {
      decltype(a) t(a);
      assert(t == a);
    }
    {
      Vec<3, float> t(1);
      assert(t[0] == 1 && t[1] == 1 && t[2] == 1);
    }
    {
      Vec<3, float> t(1, 2, 3);
      assert(t[0] == 1 && t[1] == 2 && t[2] == 3);
    }
    {
      Vec<3, float> t({1, 2, 3});
      assert(t[0] == 1 && t[1] == 2 && t[2] == 3);
    }
    {
      Vec<3, float> t(Vec<2, float>(1, 2), 3);
      assert(t[0] == 1 && t[1] == 2 && t[2] == 3);
    }
    {
      Vec<3, float> t(Vec<2, float>(1, 2));
      assert(t[0] == 1 && t[1] == 2 && t[2] == 0);
    }
    {
      float s[] = {1, 10, 2, 20, 3, 30};
      assert(Vec3f(s) == Vec3f(1, 10, 2));
      assert(Vec3f(s, 2) == Vec3f(1, 2, 3));
    }

    // factories
    assert(Vec3f::aa(0, 1.f) == Vec3f(1, 0, 0));
    assert(Vec4i::iota(0) == Vec4i(0, 1, 2, 3));
    assert(Vec4i::iota(1) == Vec4i(1, 2, 3, 4));
    assert(Vec4i::iota(0, 2) == Vec4i(0, 2, 4, 6));
    assert(Vec4d::line(0, 3) == Vec4d(0, 1, 2, 3));
    assert(Vec4d::line<false>(0, 4) == Vec4d(0, 1, 2, 3));
    assert(toVec(1, 2, 3, 4).size() == 4);
    assert(toVec(1, 2, 3, 4) == Vec4i(1, 2, 3, 4));

    // access
    for (int i = 0; i < a.size(); ++i)
      a[i] = i;
    assert(a.at<1>() == 1);
    assert(a.get(0, 1) == Vec2d(0, 1));
    assert(a.get(2, 2) == Vec2d(2, 2));
    assert(a.get(2, 1, 0) == Vec3d(2, 1, 0));
    {
      bool compileTimeVec_get = a.get<2, 1, 0>() == Vec3d(2, 1, 0);
      assert(compileTimeVec_get);
    }

    {
      for (int i = 0; i < a.size(); ++i)
        a[i] = i;

      Vec<2, double> t;
      t = a.sub<2, 0>();
      assert(t[0] == 0 && t[1] == 1);
      t = a.sub<2, 2>();
      assert(t[0] == 2 && t[1] == 3);
      // verify in-place operations
      a.sub<2>(0) += 10;
      assert(a[0] == 10 && a[1] == 11);
    }

    a[0] = 0;
    EXPECT_TRUE(a[0] == 0);
    EXPECT_TRUE(a.elems()[0] == 0);

    a.x = 1;
    EXPECT_TRUE(a[0] == 1);
    a.y = 2;
    EXPECT_TRUE(a[1] == 2);
    a.z = 3;
    EXPECT_TRUE(a[2] == 3);
    a.w = 4;
    EXPECT_TRUE(a[3] == 4);

    a.set(1);
    EXPECT_TRUE(a == 1);
    b.set(a);
    EXPECT_TRUE(b == 1);

    {
      a.set(1);
      EXPECT_EQ(a, 1);
      b.set(a);
      EXPECT_EQ(b, 1);

      a = 1;
      assert(a == 1);
      b = a;
      assert(b == 1);
      assert(b == a);
      b = 2;
      assert(b != a);

      a = 1;
      b = 0;

      double *p = a.elems();
      EXPECT_TRUE(p[0] == a[0]);
      b.set(p);
      EXPECT_TRUE(a == b);

      char c1[] = {4, 4, 4, 4};
      a.set(c1);
      EXPECT_TRUE(a == 4);

      char c2[] = {1, 0, 1, 0, 1, 0, 1, 0};
      a.set(c2, 2);
      EXPECT_TRUE(a == 1);

      //      a.zero();
      a.set(Vec<N - 1, int>(1, 2, 3), 4);
      EXPECT_TRUE(a[0] == 1);
      EXPECT_TRUE(a[1] == 2);
      EXPECT_TRUE(a[2] == 3);
      EXPECT_TRUE(a[3] == 4);
    }

    a = 0;
    EXPECT_TRUE(a == 0);

    a = 1;
    EXPECT_TRUE(a == 1);
    EXPECT_TRUE(!(a != 1));
    b = a;
    EXPECT_TRUE(b == a);
    EXPECT_TRUE(!(b != a));

    a = 3;
    b = 3;
    a -= b;
    EXPECT_TRUE(a == 0);
    a += b;
    EXPECT_TRUE(a == 3);
    a -= 3;
    EXPECT_TRUE(a == 0);
    a += 3;
    EXPECT_TRUE(a == 3);

    a *= b;
    EXPECT_TRUE(a == 9);
    a /= b;
    EXPECT_TRUE(a == 3);
    a *= 3;
    EXPECT_TRUE(a == 9);
    a /= 3;
    EXPECT_TRUE(a == 3);

    a = b + b;
    EXPECT_TRUE(a == 6);
    a = b - b;
    EXPECT_TRUE(a == 0);
    a = b * b;
    EXPECT_TRUE(a == 9);
    a = b / b;
    EXPECT_TRUE(a == 1);

    a = 2. + a;
    EXPECT_TRUE(a == 3);
    a = 6. - a;
    EXPECT_TRUE(a == 3);
    a = 2. * a;
    EXPECT_TRUE(a == 6);
    //		a = 1. / a;				EXPECT_TRUE(a == 1./6);

    a = +1;
    b = -1;
    EXPECT_TRUE(a == -b);

    a = -1;
    b = +1;
    EXPECT_TRUE(a.absVec() == b);
    EXPECT_TRUE(a.dot(b) == -N);
    EXPECT_TRUE(a.dot(a) == N);
    EXPECT_TRUE(Vec3i(1, 2, -3).product() == -6);
    EXPECT_TRUE(a.sum() == -N);
    EXPECT_TRUE(a.sumAbs() == N);
    EXPECT_TRUE(Vec3f(1, 2, 3).mean() == 2);
    EXPECT_TRUE(a.mag() == sqrt(N));
    EXPECT_TRUE(b.mag() == sqrt(N));
    EXPECT_TRUE(a.mag() == abs(a));
    EXPECT_TRUE(a.magSqr() == N);
    EXPECT_TRUE(b.magSqr() == N);
    EXPECT_TRUE(a.norm1() == N);
    EXPECT_TRUE(a.norm2() == sqrt(N));
    EXPECT_TRUE(a.norm(1) == a.norm1());
    EXPECT_TRUE(a.norm(2) == a.norm2());

    (a = 1).negate();
    EXPECT_TRUE(a == -1);
    (a = 1).normalize();
    EXPECT_TRUE(a == 1. / sqrt(N));
    EXPECT_TRUE(a == (b = 10).normalized());
    EXPECT_TRUE(Vec4i(-10, 1, 4, 100).indexOfMax() == 3);
    EXPECT_TRUE(Vec4i(-10, 1, 4, 100).indexOfMin() == 0);

    a.set(1).negate();
    EXPECT_TRUE(a == -1);
    a.set(1).normalize();
    EXPECT_TRUE(a == 1. / sqrt(N));
    EXPECT_TRUE(a == b.set(10).normalized());

    // Vec-Vec ops
    b = a = 1;
    EXPECT_TRUE(concat(a, b) == 1);

    {
      a = 0;
      Vec<N + 1, double> t = concat(a, Vec<1, char>(1));
      EXPECT_TRUE(t.size() == a.size() + 1);
    }

    for (int i = 0; i < a.size(); ++i)
      a[i] = i;
    EXPECT_TRUE(a.get(0, 1) == Vec2d(0, 1));
    EXPECT_TRUE(a.get(2, 2) == Vec2d(2, 2));
    EXPECT_TRUE(a.get(2, 1, 0) == Vec3d(2, 1, 0));

    {
      for (int i = 0; i < a.size(); ++i)
        a[i] = i;

      Vec<2, double> t;
      t = a.sub<2, 0>();
      EXPECT_TRUE(t[0] == 0);
      EXPECT_TRUE(t[1] == 1);
      t = a.sub<2>(2);
      EXPECT_TRUE(t[0] == 2);
      EXPECT_TRUE(t[1] == 3);
    }

    // geometry and other math ops
    EXPECT_TRUE(eqVal(angle(Vec3d(1, 0, 0), Vec3d(1, 0, 0)), 0.));
    EXPECT_TRUE(eqVal(angle(Vec3d(1, 0, 0), Vec3d(0, 1, 0)), M_PI_2));
    EXPECT_TRUE(eqVal(angle(Vec3d(1, 0, 0), Vec3d(0, -1, 0)), M_PI_2));

    {
      Vec3d r = centroid(Vec3d(1, 0, 0), Vec3d(0, 1, 0), Vec3d(0, 0, 1));
      EXPECT_TRUE(eq(r, Vec3d(1 / 3.)));

      normal(r, Vec3d(1, 0, 0), Vec3d(0, 1, 0), Vec3d(-1, 0, 0));
      EXPECT_TRUE(eq(r, Vec3d(0, 0, 1)));

      Vec3d pos(1, 2, 3);
      Vec3d to(4, 5, 6);
      Vec3d rel = to - pos;

      EXPECT_TRUE(rel[0] == 3);
      EXPECT_TRUE(rel[1] == 3);
      EXPECT_TRUE(rel[2] == 3);
    }

    // analysis
    EXPECT_TRUE(toVec(1, -10, 7, 2).find(7) == 2);
    EXPECT_TRUE(toVec(1, -10, 7, 2).find(3) < 0);
    EXPECT_TRUE(toVec(4, 1, 1, 7).find(1) == 1);
    EXPECT_TRUE(toVec(1, -10, 7, 2).indexOfMin() == 1);
    EXPECT_TRUE(toVec(1, -10, 7, 2).indexOfMax() == 2);

    a = 0;
    b = 1;
    EXPECT_TRUE(min(a, b) == 0);
    EXPECT_TRUE(max(a, b) == 1);
  }

  // Vec3
  {
    Vec3d a, b, c;

    a.set(1, 0, 0);
    b.set(0, 1, 0);
    c.set(0, 0, 1);
    EXPECT_TRUE(c == cross(a, b));
    EXPECT_TRUE(c == a.cross(b));

    a = b;
    EXPECT_TRUE(a == b);
  }

  // Mat
  {
/*#define CHECK2(mat, a,b,c, d,e,f, g,h,i){\
        const auto& m = mat;\
        EXPECT_TRUE(eq(m(0,0),a)); EXPECT_TRUE(eq(m(0,1),b));
EXPECT_TRUE(eq(m(0,2),c));\
        EXPECT_TRUE(eq(m(1,0),d)); EXPECT_TRUE(eq(m(1,1),e));
EXPECT_TRUE(eq(m(1,2),f));\
        EXPECT_TRUE(eq(m(2,0),g)); EXPECT_TRUE(eq(m(2,1),h));
EXPECT_TRUE(eq(m(2,2),i));\
}*/
#define CHECK2(mat, a, b, c, d, e, f, g, h, i)                                 \
  EXPECT_TRUE(eq(mat, Mat3d(a, b, c, d, e, f, g, h, i)))

    // factory functions
    CHECK2(Mat3d::identity(), 1, 0, 0, 0, 1, 0, 0, 0, 1);
    CHECK2(Mat3d::scaling(2), 2, 0, 0, 0, 2, 0, 0, 0, 1);
    CHECK2(Mat3d::scaling(2, 3), 2, 0, 0, 0, 3, 0, 0, 0, 1);
    CHECK2(Mat3d::scaling(Vec2d(2, 3)), 2, 0, 0, 0, 3, 0, 0, 0, 1);
    CHECK2(Mat3d::translation(2, 3), 1, 0, 2, 0, 1, 3, 0, 0, 1);
    CHECK2(Mat3d::translation(Vec2d(2, 3)), 1, 0, 2, 0, 1, 3, 0, 0, 1);
    CHECK2(Mat3d::rotation(M_PI / 2, 0, 1), 0, -1, 0, 1, 0, 0, 0, 0, 1);

    Mat3d a; //, b;

    a.setIdentity();
    CHECK2(a, 1, 0, 0, 0, 1, 0, 0, 0, 1);

    EXPECT_TRUE(a.trace() == 3);

    a += 2;
    CHECK2(a, 3, 2, 2, 2, 3, 2, 2, 2, 3);
    a -= 1;
    CHECK2(a, 2, 1, 1, 1, 2, 1, 1, 1, 2);
    a *= 2;
    CHECK2(a, 4, 2, 2, 2, 4, 2, 2, 2, 4);
    a /= 2;
    CHECK2(a, 2, 1, 1, 1, 2, 1, 1, 1, 2);

    a.setIdentity();
    a = a + 2;
    CHECK2(a, 3, 2, 2, 2, 3, 2, 2, 2, 3);
    a = a - 1;
    CHECK2(a, 2, 1, 1, 1, 2, 1, 1, 1, 2);
    a = a * 2;
    CHECK2(a, 4, 2, 2, 2, 4, 2, 2, 2, 4);
    a = a / 2;
    CHECK2(a, 2, 1, 1, 1, 2, 1, 1, 1, 2);

    a.setIdentity();
    a = 2. + a;
    CHECK2(a, 3, 2, 2, 2, 3, 2, 2, 2, 3);
    a = 4. - a;
    CHECK2(a, 1, 2, 2, 2, 1, 2, 2, 2, 1);
    a = 2. * a;
    CHECK2(a, 2, 4, 4, 4, 2, 4, 4, 4, 2);

    a.set(1, 2, 3, 4, 5, 6, 7, 8, 9);

    EXPECT_TRUE(a.col(0) == Vec3d(1, 4, 7));
    EXPECT_TRUE(a.col(1) == Vec3d(2, 5, 8));
    EXPECT_TRUE(a.col(2) == Vec3d(3, 6, 9));
    EXPECT_TRUE(a.row(0) == Vec3d(1, 2, 3));
    EXPECT_TRUE(a.row(1) == Vec3d(4, 5, 6));
    EXPECT_TRUE(a.row(2) == Vec3d(7, 8, 9));

    a.transpose();

    EXPECT_TRUE(a.col(0) == Vec3d(1, 2, 3));
    EXPECT_TRUE(a.col(1) == Vec3d(4, 5, 6));
    EXPECT_TRUE(a.col(2) == Vec3d(7, 8, 9));
    EXPECT_TRUE(a.row(0) == Vec3d(1, 4, 7));
    EXPECT_TRUE(a.row(1) == Vec3d(2, 5, 8));
    EXPECT_TRUE(a.row(2) == Vec3d(3, 6, 9));

    // test special operations
    {
      Mat<2, double> m(2, 4, 0, 3);

      EXPECT_TRUE(eqVal(determinant(m), 6.));

      Mat<2, double> inv = m;
      EXPECT_TRUE(invert(inv));
      EXPECT_TRUE(eq(m * inv, Mat<2, double>::identity()));
    }

    {
      Mat<3, double> m(2, 5, 7, 0, 3, 6, 0, 0, 4);

      EXPECT_TRUE(eqVal(determinant(m), 24.));

      Mat<3, double> inv = m;
      EXPECT_TRUE(invert(inv));
      EXPECT_TRUE(eq(m * inv, Mat<3, double>::identity()));
    }

#undef CHECK2
  }

  {
    Complexd c(0, 0);
#define T(x, y) EXPECT_TRUE(x == y);
    T(c, Complexd(0, 0))
    c.fromPolar(1, 0.2);
    T(c, Polard(0.2))
    c.fromPolar(2.3);
    T(c, Polard(2.3))
    EXPECT_TRUE(c != Complexd(0, 0));
    T(c.conj(), Complexd(c.r, -c.i))
#undef T

    //		#define T(x, y) EXPECT_TRUE(almostEqual(x,y,2));
    //		c.normalize();			T(c.norm(), 1)
    //		double p=0.1; c(1,0); c *= Polard(1, p); T(c.arg(), p)
    //
    //		c.fromPolar(4,0.2);
    //		T(c.sqrt().norm(), 2)
    //		T(c.sqrt().arg(), 0.1)
    //		#undef T
  }

  // Quat
  {
    struct printQuat {
      void operator()(const Quatd &v) {
        printf("%g %g %g %g\n", v[0], v[1], v[2], v[3]);
      }
    };

    // Test factory functions
    EXPECT_TRUE(Quatd::identity() == Quatd(1, 0, 0, 0));

    // Test basic mathematical operations
    Quatd q(0, 0, 0, 0);

    EXPECT_TRUE(q == Quatd(0, 0, 0, 0));

    q.setIdentity();

    EXPECT_TRUE(q == Quatd(1, 0, 0, 0));
    EXPECT_TRUE(q != Quatd(1, 0, 0, 1));

    q.set(1, 2, 4, 10);

    EXPECT_TRUE(-q == Quatd(-1, -2, -4, -10));
    EXPECT_TRUE(q.conj() == Quatd(q.w, -q.x, -q.y, -q.z));
    EXPECT_TRUE(q.dot(q) == 121);
    EXPECT_TRUE(q.mag() == 11);
    EXPECT_TRUE(q.magSqr() == 121);
    EXPECT_TRUE(eq(q.sgn(), Quatd(1. / 11, 2. / 11, 4. / 11, 10. / 11)));

    // Test rotation of vectors by quaternion
    q.fromAxisAngle(M_2PI / 4, 1, 0, 0);
    EXPECT_TRUE(eq(q, Quatd(sqrt(2) / 2, sqrt(2) / 2, 0, 0)));
    {
      Vec3d v(0, 1, 0);
      v = q.rotate(v);
      // printf("%g %g %g\n", v[0], v[1], v[2]);
      EXPECT_TRUE(eq(v, Vec3d(0, 0, 1)));
    }

    q.fromAxisAngle(M_2PI / 4, 0, 1, 0);
    EXPECT_TRUE(eq(q, Quatd(sqrt(2) / 2, 0, sqrt(2) / 2, 0)));
    {
      Vec3d v(0, 0, 1);
      v = q.rotate(v);
      // printf("%g %g %g\n", v[0], v[1], v[2]);
      EXPECT_TRUE(eq(v, Vec3d(1, 0, 0)));
    }

    q.fromAxisAngle(M_2PI / 4, 0, 0, 1);
    {
      Vec3d v(1, 0, 0);
      v = q.rotate(v);
      // printf("%g %g %g\n", v[0], v[1], v[2]);
      EXPECT_TRUE(eq(v, Vec3d(0, 1, 0)));
    }

    // Test fromAxis* consistency
    EXPECT_TRUE(q.fromAxisAngle(M_2PI / 8, 1, 0, 0) ==
                Quatd().fromAxisX(M_2PI / 8));
    EXPECT_TRUE(q.fromAxisAngle(M_2PI / 8, 0, 1, 0) ==
                Quatd().fromAxisY(M_2PI / 8));
    EXPECT_TRUE(q.fromAxisAngle(M_2PI / 8, 0, 0, 1) ==
                Quatd().fromAxisZ(M_2PI / 8));

    // Test AxisAngle<->Quat conversion
    {
      q.fromEuler(M_2PI / 7, M_2PI / 8,
                  M_2PI / 9); // set to something non-trival...
      double angle, ax, ay, az;
      q.toAxisAngle(angle, ax, ay, az);
      Quatd b = q.fromAxisAngle(angle, ax, ay, az);
      EXPECT_TRUE(((q == b) || (q == b.conj())));
    }

    // Test consistency between conversions from Euler angles and axes
    EXPECT_TRUE(
        eq(q.fromEuler(M_2PI / 8, M_2PI / 8, M_2PI / 8),
           (Quatd().fromAxisY(M_2PI / 8) * Quatd().fromAxisX(M_2PI / 8)) *
               Quatd().fromAxisZ(M_2PI / 8)));

    // Test roundtrip Euler/quat conversion
    {
      float eps = 0.00001;
      int N = 16;
      for (int k = 0; k < N; ++k) {
        for (int j = 0; j < N; ++j) {
          for (int i = 0; i < N; ++i) {
            float kludge = 0.1;
            float az = (float(k) / N - 0.5) * 2 * M_PI + kludge;
            float el = (float(j) / N - 0.5) * 2 * M_PI + kludge;
            float ba = (float(i) / N - 0.5) * 2 * M_PI + kludge;

            Quatf a, b;
            a.fromEuler(az, el, ba);
            a.toEuler(az, el, ba);
            b.fromEuler(az, el, ba);
            // a.print(); b.print(); printf("\n");

            EXPECT_TRUE((eq(a, b, eps) || eq(a, -b, eps)));
          }
        }
      }
    }

    // Test roundtrip matrix/quat conversions
    {
      double mat4[16];
      Quatd b;
      q.toMatrix(mat4);
      b = q.fromMatrix(mat4);
      EXPECT_TRUE((q == b || q == b.conj()));

      q.toMatrixTransposed(mat4);
      b = q.fromMatrixTransposed(mat4);
      EXPECT_TRUE((q == b || q == b.conj()));
    }
    // Test Quat to matrix
    {

      Quatd q;
      q.set(1, 0, 0, 0);
      Mat4d m;
      q.toMatrix(&m[0]);
      EXPECT_TRUE(eq(m, Mat4d::identity()));

      q.fromAxisAngle(M_2PI / 4, 0, 0, 1);
      q.toMatrix(&m[0]);
      // For a right-handed coordinate system
      EXPECT_TRUE(
          eq(m, Mat4d(0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)));
    }

    // Test Quat to component coordinate frame
    {
      Quatd q;

      Vec3d vx, vy, vz;

      q.fromAxisAngle(M_2PI / 4, 1, 0, 0);
      q.toVectorX(vx);
      q.toVectorY(vy);
      q.toVectorZ(vz);
      EXPECT_TRUE(eq(vx, Vec3d(1, 0, 0)));
      EXPECT_TRUE(eq(vy, Vec3d(0, 0, 1)));
      EXPECT_TRUE(eq(vz, Vec3d(0, -1, 0)));

      q.fromAxisAngle(M_2PI / 4, 0, 1, 0);
      q.toVectorX(vx);
      q.toVectorY(vy);
      q.toVectorZ(vz);
      EXPECT_TRUE(eq(vx, Vec3d(0, 0, -1)));
      EXPECT_TRUE(eq(vy, Vec3d(0, 1, 0)));
      EXPECT_TRUE(eq(vz, Vec3d(1, 0, 0)));

      q.fromAxisAngle(M_2PI / 4, 0, 0, 1);
      q.toVectorX(vx);
      q.toVectorY(vy);
      q.toVectorZ(vz);
      EXPECT_TRUE(eq(vx, Vec3d(0, 1, 0)));
      EXPECT_TRUE(eq(vy, Vec3d(-1, 0, 0)));
      EXPECT_TRUE(eq(vz, Vec3d(0, 0, 1)));
    }

    //		int smps = 100;
    //		Quatd q1 = Quatd::fromAxisAngle(10, .707, .707, 0);
    //		Quatd q2 = Quatd::fromAxisAngle(60, .707, 0, .707);
    //		Quatd buf[smps];
    //		Quatd::slerp_buffer(q1, q2, buf, smps);
    //		for (int i=0; i<smps; i++) {
    //			double t, x, y, z;
    //			buf[i].toAxisAngle(&t, &x, &y, &z);
    //			//printf("%f %f %f %f\n", t, x, y, z);
    //		}
  }

  // Simple Functions
  {
    const double pinf = INFINITY;  // + infinity
    const double ninf = -INFINITY; // - infinity

#define T(x, y, r) EXPECT_TRUE(al::atLeast(x, y) == r);
    T(0., 1., 1.)
    T(+0.1, 1., 1.)
    T(-0.1, 1., -1.)
#undef T

#define T(x, y) EXPECT_TRUE(al::ceilEven(x) == y);
    T(0, 0)
    T(1, 2)
    T(2, 2)
    T(3, 4)
    T(1001, 1002)
#undef T

#define T(x, y) EXPECT_TRUE(al::ceilPow2(x) == y);

    T(0, 0)
    T(1, 1)
    T(2, 2)
    T(3, 4)
    T(500, 512)
    T(999, 1024)

#undef T

#define T(x, y) EXPECT_TRUE(al::clip(x) == y);

    T(0., 0.)
    T(0.5, 0.5)
    T(1., 1.)
    T(1.2, 1.)
    T(-0.5, 0.)
    T(pinf, 1.)
    T(ninf, 0.)
#undef T

#define T(x, y) EXPECT_TRUE(al::clipS(x) == y);

    T(0., 0.)
    T(0.5, 0.5)
    T(1., 1.)
    T(1.2, 1.)
    T(-0.5, -0.5)
    T(-1., -1)
    T(-1.2, -1.)
#undef T

#define T(x, r) EXPECT_TRUE(al::even(x) == r);

    T(0, true)
    T(1, false)
    T(-2, true)
#undef T

#define T(x, y) EXPECT_TRUE(al::factorial(x) == y);

        T(0, 1) T(1, 1) T(2, 2 * 1) T(3, 3 * 2 * 1) T(4, 4 * 3 * 2 * 1) T(
            5, 5 * 4 * 3 * 2 * 1) T(6, 6 * 5 * 4 * 3 * 2 * 1)
            T(7, 7 * 6 * 5 * 4 * 3 * 2 * 1) T(8, 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1)
                T(9, 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 *
                         1) T(10, 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1)
                    T(11, 11 * 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1)
                        T(12, 12 * 11 * 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1)
#undef T

                            for (int i = 0; i <= 12; ++i) {
      EXPECT_TRUE(al::aeq(al::factorialSqrt(i), sqrt(al::factorial(i))));
    }

#define T(x, y) EXPECT_TRUE(al::floorPow2(x) == y);
    T(0, 1)
    T(1, 1)
    T(2, 2)
    T(3, 2)
    T(513, 512)
    T(1090, 1024)
#undef T

#define T(x, y) EXPECT_TRUE(eqVal(al::fold(x), y));
    T(0., 0.)
    T(0.5, 0.5)
    T(1., 1.)
    T(1.2, 0.8)
    T(-0.2, 0.2)
    T(2.2, 0.2)
    T(3.2, 0.8)
    T(4.2, 0.2)
    T(5.2, 0.8)
#undef T

#define T(x, y, r) EXPECT_TRUE(al::gcd(x, y) == r);
    T(7, 7, 7)
    T(7, 4, 1)
    T(8, 4, 4)
#undef T

#define T(x, y, r) EXPECT_TRUE(al::lcm(x, y) == r);
    T(7, 3, 21)
    T(8, 4, 8)
    T(3, 1, 3)
#undef T

#define T(x, y, r) EXPECT_TRUE(al::lessAbs(x, y) == r);
    T(0.1, 1., true)
    T(-0.1, 1., true)
    T(1., 1., false)
    T(-1., 1., false)
#undef T

#define T(x, r) EXPECT_TRUE(al::odd(x) == r);
    T(0, false)
    T(1, true)
    T(-2, false)
#undef T

#define T(x) EXPECT_TRUE(al::pow2(x) == x * x);
    T(0)
    T(1)
    T(2)
    T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow2S(x) == x * std::abs(x));
        T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow3(x) == x * x * x);
            T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow3Abs(x) == std::abs(x * x * x));
                T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow4(x) == x * x * x * x);
                    T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow5(x) == x * x * x * x * x);
                        T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow6(x) == x * x * x * x * x * x);
                            T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x) EXPECT_TRUE(al::pow8(x) == x * x * x * x * x * x * x * x);
                                T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x)                                                                   \
  EXPECT_TRUE(al::pow16(x) ==                                                  \
              x * x * x * x * x * x * x * x * x * x * x * x * x * x * x * x);
                                    T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
#undef T

#define T(x)                                                                   \
  EXPECT_TRUE(eqVal(                                                           \
      al::pow64(x),                                                            \
      x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x   \
          *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x \
              *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x *x));
                                        T(0.) T(1.) T(1.01) T(1.02) T(-1.) T(
                                            -1.01) T(-1.02)
#undef T

    //	#define T(x,r) EXPECT_TRUE(al::powerOf2(x) == r);
    //	T(0, false) T(1, true) T(2, true) T(3, false) T(4, true)
    //	#undef T

#define T(x, y, r) EXPECT_TRUE(al::round(x, y) == r);
                                            T(0.0, 0.1, 0.0) T(0.1, 0.1, 0.1) T(
                                                0.15, 0.1, 0.1) T(-0.15, 0.1,
                                                                  -0.1)
#undef T

#define T(x, y, r) EXPECT_TRUE(al::roundAway(x, y) == r);
                                                T(0.0, 0.1,
                                                  0.0) T(0.1, 0.1,
                                                         0.1) T(0.15, 0.1, 0.2)
                                                    T(-0.15, 0.1, -0.2)
#undef T

#define T(x, r) EXPECT_TRUE(al::sgn(x) == r);
                                                        T(-0.1, -1.) T(
                                                            0.1, 1.) T(0., 0.)
#undef T

#define T(x1, y1, x2, y2, r) EXPECT_TRUE(al::slope(x1, y1, x2, y2) == r);
                                                            T(3., 3., 4., 4.,
                                                              1.) T(3., -3., 4.,
                                                                    -4., -1.)
#undef T

    {
      double x = 1, y = 0;
      sort(x, y);
      EXPECT_TRUE(x == 0);
      EXPECT_TRUE(y == 1);
      sort(x, y);
      EXPECT_TRUE(x == 0);
      EXPECT_TRUE(y == 1);
    }

#define T(x, y) EXPECT_TRUE(al::sumOfSquares(x) == y);
    T(1., 1.)
    T(2., 1 * 1 + 2 * 2)
    T(3., 1 * 1 + 2 * 2 + 3 * 3)
    T(4., 1 * 1 + 2 * 2 + 3 * 3 + 4 * 4)
    T(5., 1 * 1 + 2 * 2 + 3 * 3 + 4 * 4 + 5 * 5)
#undef T

#define T(x, r) EXPECT_TRUE(al::trailingZeroes(x) == r);
    T(0, 0)
    T(1, 0)
    T(2, 1)
    T(3, 0)
    T(4, 2)
    T(8, 3)
    T(9, 0)
#undef T

#define T(x, l, h, r) EXPECT_TRUE(al::within(x, l, h) == r);
    T(0, 0, 1, true)
    T(1, 0, 1, true)
#undef T

    //	printf("%.20g\n", wrap<double>(-32.0, 32.0, 0.));  // should be 0.0
    //	printf("%.20g\n", wrap<double>(-64.0, 32.0, 0.));  // should be 0.0
    //	printf("%.20g\n", wrap<double>(-1e-16, 32., 0.));  // should
    // be 31.999999999999996447

#define T(x, y) EXPECT_TRUE(eqVal(al::wrap(x, 1., -1.), y));
    T(0., 0.)
    T(0.5, 0.5)
    T(1., -1.)
    T(1.2, -0.8)
    T(2.2, 0.2) T(-0.5, -0.5) T(-1., -1.) T(-1.2, 0.8) T(-2.2, -0.2)
#undef T
  }

#define T(x, y) EXPECT_TRUE(eqVal(al::wrapPhase(x), y));
  T(0., 0.)
  T(1., 1.)
  T(M_PI, -M_PI)
  T(M_PI + 1, -M_PI + 1)
  T(7 * M_PI + 1, -M_PI + 1)
  T(-1., -1.)
  T(-M_PI, -M_PI)
  T(-M_PI - 1, M_PI - 1)
  T(-7 * M_PI + 1, -M_PI + 1)
#undef T

  // Special Functions
  {
    struct F {

      // Pl,-m(x) = (-1)^m (l-m)! / (l+m)! Pl,m(x)
      static double testLegendreP(int l, int m, double x) {
        switch (l) {
        case 0:
          return 1;
        case 1:
          switch (m) {
          case -1:
            return -1. / (2) * testLegendreP(l, -m, x);
          case 0:
            return x;
          case 1:
            return -sqrt(1 - x * x);
          }
          break;
        case 2:
          switch (m) {
          case -2:
            return +1. / (4 * 3 * 2 * 1) * testLegendreP(l, -m, x);
          case -1:
            return -1. / (3 * 2) * testLegendreP(l, -m, x);
          case 0:
            return 0.5 * (3 * x * x - 1);
          case 1:
            return -3 * x * sqrt(1 - x * x);
          case 2:
            return 3 * (1 - x * x);
          }
          break;
        case 3:
          switch (m) {
          case -3:
            return -1. / (6 * 5 * 4 * 3 * 2 * 1) * testLegendreP(l, -m, x);
          case -2:
            return +1. / (5 * 4 * 3 * 2) * testLegendreP(l, -m, x);
          case -1:
            return -1. / (4 * 3) * testLegendreP(l, -m, x);
          case 0:
            return 0.5 * x * (5 * x * x - 3);
          case 1:
            return 1.5 * (1 - 5 * x * x) * sqrt(1 - x * x);
          case 2:
            return 15 * x * (1 - x * x);
          case 3:
            return -15 * al::pow3(sqrt(1 - x * x));
          }
          break;
        }
        return 0; // undefined
      }

      static double testLaguerre(int n, int k, double x) {
        switch (n) {
        case 0:
          return 1;
        case 1:
          return -x + k + 1;
        case 2:
          return (1. / 2) * (x * x - 2 * (k + 2) * x + (k + 2) * (k + 1));
        case 3:
          return (1. / 6) *
                 (-x * x * x + 3 * (k + 3) * x * x - 3 * (k + 2) * (k + 3) * x +
                  (k + 1) * (k + 2) * (k + 3));
        default:
          return 0;
        }
      }
    };

    const int M = 2000; // granularity of domain

    // test associated legendre
    for (int l = 0; l <= 3; ++l) {
      for (int m = 0; m <= l; ++m) {
        for (int i = 0; i < M; ++i) {
          double theta = double(i) * M_PI / M;
          double a = al::legendreP(l, m, theta);
          double b = F::testLegendreP(l, m, cos(theta));
          //		if(!al::aeq(a, b, 1<<16)){
          if (!(std::abs(a - b) < 1e-10)) {
            printf("\nP(%d, %d, %g) = %.16g (actual = %.16g)\n", l, m,
                   cos(theta), a, b);
            EXPECT_TRUE(false);
          }
        }
      }
    }

    // test laguerre
    for (int n = 0; n <= 3; ++n) {
      for (int i = 0; i < M; ++i) {
        double x = double(i) / M * 4;
        double a = al::laguerreL(n, 1, x);
        double b = F::testLaguerre(n, 1, x);

        if (!(std::abs(a - b) < 1e-10)) {
          printf("\nL(%d, %g) = %.16g (actual = %.16g)\n", n, x, a, b);
          EXPECT_TRUE(false);
        }
      }
    }

    // TODO: spherical harmonics
    //		for(int l=0; l<=SphericalHarmonic::L_MAX; ++l){
    //		for(int m=-l; m<=l; ++m){
    //	//		double c = SphericalHarmonic::coef(l,m);
    //	//		double t = computeCoef(l,m);
    //	//		EXPECT_TRUE(c == t);
    //		}}
    //
    //		for(int j=0; j<M; ++j){	double ph = double(j)/M * M_PI;
    //		for(int i=0; i<M; ++i){ double th = double(i)/M * M_2PI;
    //
    //		}}
  }

  // Interval
  {
    Interval<double> i(0, 1);

    EXPECT_TRUE(i.min() == 0);
    EXPECT_TRUE(i.max() == 1);

    i.min(2);
    EXPECT_TRUE(i.min() == 1);
    EXPECT_TRUE(i.max() == 2);

    i.max(0);
    EXPECT_TRUE(i.min() == 0);
    EXPECT_TRUE(i.max() == 1);

    i.endpoints(-1, 1);
    EXPECT_TRUE(i.min() == -1);
    EXPECT_TRUE(i.max() == 1);

    EXPECT_TRUE(i.center() == 0);
    EXPECT_TRUE(i.diameter() == 2);
    EXPECT_TRUE(i.radius() == 1);

    EXPECT_TRUE(i.proper());

    i.endpoints(0, 0);
    EXPECT_TRUE(i.degenerate());

    i.centerDiameter(1, 4);
    EXPECT_TRUE(i.center() == 1);
    EXPECT_TRUE(i.diameter() == 4);
    EXPECT_TRUE(i.min() == -1);
    EXPECT_TRUE(i.max() == 3);

    i.center(2);
    EXPECT_TRUE(i.min() == 0);
    EXPECT_TRUE(i.max() == 4);

    i.diameter(6);
    EXPECT_TRUE(i.min() == -1);
    EXPECT_TRUE(i.max() == 5);

    i.endpoints(-1, 1);
    EXPECT_TRUE(i.toUnit(0) == 0.5);

    EXPECT_TRUE(Interval<int>(0, 1) == Interval<int>(0, 1));
    EXPECT_TRUE(Interval<int>(0, 2) != Interval<int>(0, 1));
    EXPECT_TRUE((Interval<int>(0, 2) += Interval<int>(-1, 2)) ==
                Interval<int>(-1, 4));
    EXPECT_TRUE((Interval<int>(0, 2) -= Interval<int>(-1, 2)) ==
                Interval<int>(-2, 3));
  }

// Random
#if 0
	{
		using namespace al::rnd;

		// Ensure uniqueness of sequences
        EXPECT_TRUE(seed() != seed());

		{	LinCon a,b;
            EXPECT_TRUE(a() != b()); // sequences are unique
            EXPECT_TRUE(a() != a());	// successive values are unique
		}

		{	MulLinCon a,b;
            EXPECT_TRUE(a() != b()); // sequences are unique
            EXPECT_TRUE(a() != a()); // successive values are unique
		}

		{	Tausworthe a,b;
            EXPECT_TRUE(a() != b()); // sequences are unique
            EXPECT_TRUE(a() != a()); // successive values are unique
		}


		Random<> r;
		int N = 1000000;
        for(int i=0; i<N; ++i){ float v=r.uniform(); EXPECT_TRUE(  0 <= v); EXPECT_TRUE(v < 1); }
        for(int i=0; i<N; ++i){ int v=r.uniform(20,  0); EXPECT_TRUE(  0 <= v); EXPECT_TRUE(v < 20); }
        for(int i=0; i<N; ++i){ int v=r.uniform(20, 10); EXPECT_TRUE( 10 <= v); EXPECT_TRUE(v < 20); }
        for(int i=0; i<N; ++i){ int v=r.uniform(20,-10); EXPECT_TRUE(-10 <= v); EXPECT_TRUE(v < 20); }

        for(int i=0; i<N; ++i){ float v=r.uniformS(); EXPECT_TRUE(  -1 <= v); EXPECT_TRUE(v <  1); }
        for(int i=0; i<N; ++i){ int v=r.uniformS(20); EXPECT_TRUE( -20 <= v); EXPECT_TRUE(v < 20); }

		//for(int i=0; i<32; ++i) printf("% g ", r.uniformS());
		//for(int i=0; i<32; ++i) printf("%d ", r.prob(0.1));
		//for(int i=0; i<128; ++i) printf("% g\n", r.gaussian());

		int arr[] = {0,1,2,3,4,5,6,7};
		r.shuffle(arr, 8);
		//for(int i=0; i<8; ++i) printf("%d\n", arr[i]);
		//printf("\n");

		// Test uniformity of random sequence
		{
			Random<> r;
			const int N=64;
			const int M=1000;
			const int eps=M*0.2;
			int histo[N] = {0};

			for(int i=0; i<M*N; ++i){
				int idx = r.uniform(N);
				++histo[idx];
			}

			for(int i=0; i<N; ++i){
				int cnt = histo[i];
				//printf("%d\n", cnt);
                EXPECT_TRUE(M-eps < cnt); EXPECT_TRUE(cnt < M+eps);
			}
		}
	}
#endif

  {
    al::Plane<double> p;
    p.fromNormalAndPoint(Vec3d(1, 0, 0), Vec3d(1, 1, 1));

    EXPECT_TRUE(p.distance(Vec3d(1, 0, 0)) == 0);
    EXPECT_TRUE(p.distance(Vec3d(0, 1, 0)) == -1);
    EXPECT_TRUE(p.distance(Vec3d(0, 0, 1)) == -1);
    EXPECT_TRUE(p.distance(Vec3d(2, 0, 0)) == 1);
  }

  {
    Frustumd f;
    f.fbl = Vec3d(-1, -1, -1);
    f.fbr = Vec3d(1, -1, -1);
    f.ftl = Vec3d(-1, 1, -1);
    f.ftr = Vec3d(1, 1, -1);
    f.nbl = Vec3d(-1, -1, 1);
    f.nbr = Vec3d(1, -1, 1);
    f.ntl = Vec3d(-1, 1, 1);
    f.ntr = Vec3d(1, 1, 1);
    f.computePlanes();

    //		Plane<double>& p = f.pl[Frustumd::LEFT];
    //		printf("%g %g %g\n", p.normal()[0], p.normal()[1],
    // p.normal()[2]);
    //
    //		Vec3d nrm = cross(f.fbl-f.nbl, f.ntl-f.nbl);
    //		printf("%g %g %g\n", nrm[0], nrm[1], nrm[2]);
    //
    //		printf("%g %g %g\n", (f.fbl-f.nbl)[0], (f.fbl-f.nbl)[1],
    //(f.fbl-f.nbl)[2]); 		printf("%g %g %g\n", (f.ntl-f.nbl)[0],
    //(f.ntl-f.nbl)[1], (f.ntl-f.nbl)[2]);

    EXPECT_TRUE(f.testPoint(Vec3d(0, 0, 0)) == Frustumd::INSIDE);
    EXPECT_TRUE(f.testPoint(Vec3d(2, 1, 1)) == Frustumd::OUTSIDE);

    EXPECT_TRUE(f.testSphere(Vec3d(0, 0, 0), 0.9) == Frustumd::INSIDE);
    EXPECT_TRUE(f.testSphere(Vec3d(0, 0, 0), 1.1) == Frustumd::INTERSECT);
    EXPECT_TRUE(f.testSphere(Vec3d(2, 2, 2), 0.5) == Frustumd::OUTSIDE);
  }
}
