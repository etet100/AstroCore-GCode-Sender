#ifndef CUBE_H
#define CUBE_H

static const QVector<CubeVertexData> cube = {
  // f 0 Default_Generic
  CubeVertexData(QVector3D(5.500000, 5.500000, 7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, 5.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 7.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 1 Default_Generic
  CubeVertexData(QVector3D(5.500000, 5.500000, -7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 7.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 2 BackLeft
  CubeVertexData(QVector3D(5.500000, 7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 3 BackLeft
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, 5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 4 Default_Generic
  CubeVertexData(QVector3D(5.500000, -5.500000, 7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -7.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 5 Default_Generic
  CubeVertexData(QVector3D(-5.500000, 5.500000, 7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 6 Default_Generic
  CubeVertexData(QVector3D(5.500000, -5.500000, -7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, -5.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -7.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 7 BackRight
  CubeVertexData(QVector3D(5.500000, -7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 8 BackRight
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, -5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 9 Default_Generic
  CubeVertexData(QVector3D(-5.500000, 5.500000, -7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 10 FrontLeft
  CubeVertexData(QVector3D(-5.500000, 7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 11 FrontLeft
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 12 LeftBottom
  CubeVertexData(QVector3D(5.500000, 5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 13 LeftBottom
  CubeVertexData(QVector3D(5.500000, 7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 14 Default_Generic
  CubeVertexData(QVector3D(-5.500000, -5.500000, 7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, -5.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, 5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 15 Default_Generic
  CubeVertexData(QVector3D(-5.500000, -5.500000, -7.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 0, QVector3D(0.0, 0.0, 0.0)),
  // f 16 FrontRight
  CubeVertexData(QVector3D(-7.500000, -5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 17 FrontRight
  CubeVertexData(QVector3D(-5.500000, -7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 18 FrontBottom
  CubeVertexData(QVector3D(-7.500000, 5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 19 FrontBottom
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 20 RightBottom
  CubeVertexData(QVector3D(-5.500000, -5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 21 RightBottom
  CubeVertexData(QVector3D(-5.500000, -7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -7.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 22 RightTop
  CubeVertexData(QVector3D(5.500000, -5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 23 RightTop
  CubeVertexData(QVector3D(5.500000, -7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 24 FrontTop
  CubeVertexData(QVector3D(-7.500000, -5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 25 FrontTop
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, -5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 26 BackBottom
  CubeVertexData(QVector3D(7.500000, -5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 27 BackBottom
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 5.500000, -7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 28 LeftTop
  CubeVertexData(QVector3D(-5.500000, 5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 29 LeftTop
  CubeVertexData(QVector3D(-5.500000, 7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 7.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 30 BackTop
  CubeVertexData(QVector3D(7.500000, 5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  // f 31 BackTop
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, 5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  CubeVertexData(QVector3D(5.500000, -5.500000, 7.500000), 1, QVector3D(0.0, 0.0, 0.0)),
  ////////////////
  // f 32 Back
  CubeVertexData(QVector3D(7.500000, -5.500000, -5.500000), 7, QVector3D(0.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 7, QVector3D(1.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 7, QVector3D(0.000000, 1.000000, 0.000000)),
  // f 33 Back
  CubeVertexData(QVector3D(7.500000, -5.500000, 5.500000), 7, QVector3D(0.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(7.500000, 5.500000, -5.500000), 7, QVector3D(1.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(7.500000, 5.500000, 5.500000), 7, QVector3D(1.000000, 1.000000, 0.000000)),
  // f 34 Left
  CubeVertexData(QVector3D(-5.500000, 7.500000, 5.500000), 4, QVector3D(0.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, 7.500000, 5.500000), 4, QVector3D(1.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(-5.500000, 7.500000, -5.500000), 4, QVector3D(0.000000, 0.000000, 0.000000)),
  // f 35 Left
  CubeVertexData(QVector3D(-5.500000, 7.500000, -5.500000), 4, QVector3D(0.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, 7.500000, 5.500000), 4, QVector3D(1.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, 7.500000, -5.500000), 4, QVector3D(1.000000, 0.000000, 0.000000)),
  // f 36 Bottom
  CubeVertexData(QVector3D(-5.500000, 5.500000, -7.500000), 5, QVector3D(0.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, 5.500000, -7.500000), 5, QVector3D(1.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(-5.500000, -5.500000, -7.500000), 5, QVector3D(0.000000, 0.000000, 0.000000)),
  // f 37 Bottom
  CubeVertexData(QVector3D(-5.500000, -5.500000, -7.500000), 5, QVector3D(0.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, 5.500000, -7.500000), 5, QVector3D(1.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, -5.500000, -7.500000), 5, QVector3D(1.000000, 0.000000, 0.000000)),
  // f 38 Right
  CubeVertexData(QVector3D(-5.500000, -7.500000, -5.500000), 3, QVector3D(0.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, -7.500000, -5.500000), 3, QVector3D(1.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(-5.500000, -7.500000, 5.500000), 3, QVector3D(0.000000, 1.000000, 0.000000)),
  // f 39 Right
  CubeVertexData(QVector3D(-5.500000, -7.500000, 5.500000), 3, QVector3D(0.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, -7.500000, -5.500000), 3, QVector3D(1.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, -7.500000, 5.500000), 3, QVector3D(1.000000, 1.000000, 0.000000)),
  // f 40 Front
  CubeVertexData(QVector3D(-7.500000, -5.500000, 5.500000), 8, QVector3D(0.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 8, QVector3D(1.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 8, QVector3D(0.000000, 0.000000, 0.000000)),
  // f 41 Front
  CubeVertexData(QVector3D(-7.500000, -5.500000, -5.500000), 8, QVector3D(0.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, 5.500000), 8, QVector3D(1.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(-7.500000, 5.500000, -5.500000), 8, QVector3D(1.000000, 0.000000, 0.000000)),
  // f 42 Top
  CubeVertexData(QVector3D(-5.500000, -5.500000, 7.500000), 6, QVector3D(0.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, -5.500000, 7.500000), 6, QVector3D(1.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(-5.500000, 5.500000, 7.500000), 6, QVector3D(0.000000, 1.000000, 0.000000)),
  // f 43 Top
  CubeVertexData(QVector3D(-5.500000, 5.500000, 7.500000), 6, QVector3D(0.000000, 1.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, -5.500000, 7.500000), 6, QVector3D(1.000000, 0.000000, 0.000000)),
  CubeVertexData(QVector3D(5.500000, 5.500000, 7.500000), 6, QVector3D(1.000000, 1.000000, 0.000000)),
};

enum class CubeClickableFace {
    None = -1,
    BackLeft = 0,
    BackRight = 1,
    FrontLeft = 2,
    LeftBottom = 3,
    FrontRight = 4,
    FrontBottom = 5,
    RightBottom = 6,
    RightTop = 7,
    FrontTop = 8,
    BackBottom = 9,
    LeftTop = 10,
    BackTop = 11,
    Back = 12,
    Left = 13,
    Bottom = 14,
    Right = 15,
    Front = 16,
    Top = 17,
};

static const int clickables[][6] = {
    {6, 7, 8, 9, 10, 11},
    {21, 22, 23, 24, 25, 26},
    {30, 31, 32, 33, 34, 35},
    {36, 37, 38, 39, 40, 41},
    {48, 49, 50, 51, 52, 53},
    {54, 55, 56, 57, 58, 59},
    {60, 61, 62, 63, 64, 65},
    {66, 67, 68, 69, 70, 71},
    {72, 73, 74, 75, 76, 77},
    {78, 79, 80, 81, 82, 83},
    {84, 85, 86, 87, 88, 89},
    {90, 91, 92, 93, 94, 95},
    {96, 97, 98, 99, 100, 101},
    {102, 103, 104, 105, 106, 107},
    {108, 109, 110, 111, 112, 113},
    {114, 115, 116, 117, 118, 119},
    {120, 121, 122, 123, 124, 125},
    {126, 127, 128, 129, 130, 131},
};

#endif // CUBE_H
