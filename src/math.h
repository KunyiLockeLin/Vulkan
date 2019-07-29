﻿#pragma once
// Right-handed Coordinate System
#include "header.h"

struct QeVector2i {
    int x, y;

    QeVector2i();
    QeVector2i(int _x, int _y);
};

struct QeVector2f {
    float x, y;

    QeVector2f();
    QeVector2f(float _x, float _y);

    bool operator==(const QeVector2f &other) const;
    QeVector2f &operator+=(const QeVector2f &other);
    QeVector2f &operator-=(const QeVector2f &other);
    QeVector2f &operator/=(const float &other);
    QeVector2f &operator*=(const float &other);
    QeVector2f operator/(const float &other);
    QeVector2f operator*(const float &other);
};

struct QeVector3i {
    int x, y, z;

    QeVector3i();
    QeVector3i(int _x, int _y, int _z);

    bool operator==(const QeVector3i &other) const;
    QeVector3i &operator+=(const QeVector3i &other);
    QeVector3i &operator-=(const QeVector3i &other);
    QeVector3i &operator-=(const int &other);
    QeVector3i &operator/=(const int &other);
    QeVector3i operator-(const QeVector3i &other);
    QeVector3i operator/(const int &other);
    QeVector3i operator*(const int &other);
};

struct QeVector3f {
    float x, y, z;

    QeVector3f();
    QeVector3f(float _x, float _y, float _z);
    QeVector3f(int _x, int _y, int _z);
    QeVector3f(const QeVector4f &other);

    bool operator==(const QeVector3f &other) const;
    bool operator!=(const QeVector3f &other) const;

    QeVector3f &operator=(const QeVector4f &other);
    QeVector3f &operator+=(const QeVector3f &other);
    QeVector3f &operator-=(const QeVector3f &other);
    QeVector3f &operator*=(const QeVector3f &other);
    QeVector3f &operator/=(const QeVector3f &other);
    QeVector3f &operator-=(const float &other);
    QeVector3f &operator+=(const float &other);
    QeVector3f &operator/=(const float &other);
    QeVector3f &operator*=(const float &other);
    QeVector3f operator+(const QeVector3f &other);
    QeVector3f operator-(const QeVector3f &other);
    QeVector3f operator*(const QeVector3f &other);
    QeVector3f operator+(const float &other);
    QeVector3f operator/(const float &other);
    QeVector3f operator*(const float &other);
};

struct QeVector4s {
    short int x, y, z, w;
    QeVector4s();
};

struct QeVector4i {
    int x, y, z, w;
    QeVector4i();
    QeVector4i &operator=(const QeVector4s &other);
};

struct QeVector4f {
    float x, y, z, w;

    QeVector4f();
    QeVector4f(float _x, float _y, float _z, float _w);
    QeVector4f(int _x, int _y, int _z, int _w);
    QeVector4f(const QeVector3f &other, float _w);

    bool operator==(const QeVector4f &other) const;
    bool operator!=(const QeVector4f &other) const;

    QeVector4f &operator=(const QeVector3f &other);
    QeVector4f &operator=(const QeVector2f &other);
    QeVector4f &operator=(const QeVector4s &other);
    QeVector4f &operator+=(const QeVector3f &other);
    QeVector4f operator+(const QeVector3f &other);
    QeVector4f operator-(const QeVector3f &other);
    QeVector4f operator/(const float &other);
    QeVector4f &operator*=(const float &other);
};

struct QeMatrix4x4f {
    float _00, _01, _02, _03;
    float _10, _11, _12, _13;
    float _20, _21, _22, _23;
    float _30, _31, _32, _33;

    QeMatrix4x4f();
    QeMatrix4x4f(float _num);
    QeMatrix4x4f(float __00, float __01, float __02, float __03, float __10, float __11, float __12, float __13, float __20,
                 float __21, float __22, float __23, float __30, float __31, float __32, float __33);
    QeMatrix4x4f &operator*=(const QeMatrix4x4f &other);
    QeMatrix4x4f operator*(const QeMatrix4x4f &other);
    QeVector4f operator*(const QeVector4f &other);
    QeMatrix4x4f &operator/=(const float &other);
};

struct QeRay {
    QeVector3f origin;
    QeVector3f direction;
    QeVector3f positionByTime(float t);
};

struct QeRayHitRecord {
    float t;
    QeVector3f position;
    QeVector3f normal;
};

struct QeBoundingSphere {
    QeVector3f center;
    float radius;
};

struct QeBoundingBox {
    QeVector3f min;
    QeVector3f max;
};

class QeBinaryTree {
    void *data;
    int key;
    QeBinaryTree *left = nullptr;
    QeBinaryTree *right = nullptr;
    void insertNode(QeBinaryTree &node) {}
    void removeNode(QeBinaryTree &node) {}
    QeBinaryTree *getNode(int &key) { return nullptr; }
};

class QeMath {
   public:
    QeMath(QeGlobalKey &_key) {}
    ~QeMath() {}

    const float PI = 3.1415927f;
    const float RADIANS_TO_DEGREES = 180.0f / PI;
    const float DEGREES_TO_RADIANS = PI / 180;

    int iRandom(int start, int range);
    void iRandoms(int start, int range, int size, int *ret);
    float fRandom(float start, float range);
    void fRandoms(float start, float range, int size, float *ret);
    QeMatrix4x4f lookAt(QeVector3f &_pos, QeVector3f &_center, QeVector3f &_up);
    QeMatrix4x4f perspective(float _fov, float _aspect, float _near, float _far);
    QeMatrix4x4f translate(QeVector3f &_pos);
    QeVector3f move(QeVector3f &_position, QeVector3f &_addMove, QeVector3f &_face, QeVector3f &_up);
    QeMatrix4x4f rotate_quaternion(QeVector3f &_eulerAngles);
    QeMatrix4x4f rotate_quaternion(QeVector4f &quaternion);
    QeMatrix4x4f rotate_quaternion(float _angle, QeVector3f &_axis);
    QeVector4f eulerAngles_to_quaternion(QeVector3f &_eulerAngles);
    QeVector4f axis_to_quaternion(float _angle, QeVector3f &_axis);
    QeMatrix4x4f rotate_eularAngles(QeVector3f &_eulerAngles);  // (roll, pitch, yaw) or (bank, attitude, heading)
    QeMatrix4x4f rotate_axis(float _angle, QeVector3f &_axis);
    QeMatrix4x4f rotateX(float _angle);
    QeMatrix4x4f rotateY(float _angle);
    QeMatrix4x4f rotateZ(float _angle);
    QeVector4f matrix_to_quaternion(QeMatrix4x4f matrix);
    QeMatrix4x4f scale(QeVector3f &_size);
    QeMatrix4x4f transform(QeVector3f &_tanslation, QeVector4f &_rotation_quaternion, QeVector3f &_scale);
    QeMatrix4x4f getTransformMatrix(QeVector3f &_translate, QeVector3f &_rotateEuler, QeVector3f &_scale, bool bRotate = true,
                                    bool bFixSize = false);
    QeVector3f normalize(QeVector3f &_vec);
    QeVector4f normalize(QeVector4f &_vec);
    QeVector3f eulerAnglesToVector(QeVector3f &_eulerAngles);
    QeVector3f vectorToEulerAngles(QeVector3f &_vector);
    float length(QeVector2f &_vec);
    float length(QeVector3f &_vec);
    float length(QeVector4f &_vec);
    float dot(QeVector2f &_vec1, QeVector2f &_vec2);
    float dot(QeVector3f &_vec1, QeVector3f &_vec2);
    float dot(QeVector4f &_vec1, QeVector4f &_vec2);
    QeVector3f cross(QeVector3f &_vec1, QeVector3f &_vec2);
    float fastSqrt(float _number);
    bool inverse(QeMatrix4x4f &_inMat, QeMatrix4x4f &_outMat);
    QeMatrix4x4f transpose(QeMatrix4x4f &_mat);
    int clamp(int in, int low, int high);
    float clamp(float in, float low, float high);
    QeVector4f interpolateDir(QeVector4f &a, QeVector4f &b, float blend);
    QeVector3f interpolatePos(QeVector3f &start, QeVector3f &end, float progression);
    float getAnglefromVectors(QeVector3f &v1, QeVector3f &v2);
    QeVector3f revolute_axis(QeVector3f &_position, QeVector3f &_addRevolute, QeVector3f &_centerPosition, bool bFixX = false,
                        bool bFixY = false, bool bFixZ = false);
    QeVector3f revolute_eularAngles(QeVector3f &_position, QeVector3f &_addRevolute, QeVector3f &_centerPosition, bool bFixX,
                                    bool bFixY, bool bFixZ);

    // void getAnglefromVector(QeVector3f& inV, float & outPolarAngle, float & outAzimuthalAngle);
    // void rotatefromCenter(QeVector3f& center, QeVector3f& pos, float polarAngle, float azimuthalAngle);
    // void rotatefromCenter(QeVector3f& center, QeVector3f& pos, QeVector2f & axis, float angle, bool bStopTop);
    bool hit_test_raycast_sphere(QeRay &ray, QeBoundingSphere &sphere, float maxDistance = 0.f, QeRayHitRecord *hit = nullptr);
    void quicksort(float *data, int count);
};