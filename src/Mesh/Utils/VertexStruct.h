// NoodlesPlate Copyright (C) 2017-2024 Stephane Cuillerdier aka Aiekick
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#pragma warning(disable : 4324)

#include <ctools/cTools.h>

namespace VertexStruct {
// ne pas utiliser size_t e, X64 il utilise des int64 ald des int32 en x86
// win64 => typedef unsigned __int64 size_t;
// win32 => typedef unsigned int     size_t;
// glBufferData supporte les uint mais pas les uint64
// vulkan, il semeble que uint64 verole les indes dans le gpu, est ce uniquement du au binaire x86 ?
// a tester sur x64. vk::DeviceSize est un uint64_t curieusement, mais peut etre que un indexBuffer ne peut supporter ce format
typedef uint32_t I1;

struct P2 {
    ct::fvec2 p;  // pos

    P2();
    P2(ct::fvec2 vp);
};

struct P2_T2 {
    ct::fvec2 p;  // pos
    ct::fvec2 t;  // tex coord

    P2_T2();
    P2_T2(ct::fvec2 vp);
    P2_T2(ct::fvec2 vp, ct::fvec2 vt);
};

struct P3_N3_T2 {
    ct::fvec3 p;  // pos
    ct::fvec3 n;  // normal
    ct::fvec2 t;  // tex coord

    P3_N3_T2();
    P3_N3_T2(ct::fvec3 vp);
    P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn);
    P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt);
};

struct P3_N3_T2_C4 {
    ct::fvec3 p;  // pos
    ct::fvec3 n;  // normal
    ct::fvec2 t;  // tex coord
    ct::fvec4 c;  // color

    P3_N3_T2_C4();
    P3_N3_T2_C4(ct::fvec3 vp);
    P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn);
    P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt);
    P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt, ct::fvec4 vc);
};

struct P3_N3_C4 {
    ct::fvec3 p;  // pos
    ct::fvec3 n;  // normal
    ct::fvec4 c;  // color

    P3_N3_C4();
    P3_N3_C4(ct::fvec3 vp);
    P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn);
    P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc);
};

struct P3_N3_C4_D1 {
    ct::fvec3 p;     // pos
    ct::fvec3 n;     // normal
    ct::fvec4 c;     // color
    float d = 0.0f;  // distance field

    P3_N3_C4_D1();
    P3_N3_C4_D1(ct::fvec3 vp);
    P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn);
    P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc);
    P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc, float vd);
};

struct P3_N3_TA3_BTA3_T2_C4 {
    ct::fvec3 p;     // pos
    ct::fvec3 n;     // normal
    ct::fvec3 tan;   // tangent
    ct::fvec3 btan;  // bitangent
    ct::fvec2 t;     // tex coord
    ct::fvec4 c;     // color

    P3_N3_TA3_BTA3_T2_C4();
    P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp);
    P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn);
    P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan);
    P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan);
    P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt);
    P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt, ct::fvec4 vc);
};
}  // namespace VertexStruct
