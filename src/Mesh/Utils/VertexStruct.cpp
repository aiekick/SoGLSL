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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VertexStruct.h"

VertexStruct::P2::P2() {}
VertexStruct::P2::P2(ct::fvec2 vp) { p = vp; }

VertexStruct::P2_T2::P2_T2() {}
VertexStruct::P2_T2::P2_T2(ct::fvec2 vp) { p = vp; }
VertexStruct::P2_T2::P2_T2(ct::fvec2 vp, ct::fvec2 vt) { p = vp; t = vt; }

VertexStruct::P3_N3_T2::P3_N3_T2() {}
VertexStruct::P3_N3_T2::P3_N3_T2(ct::fvec3 vp) { p = vp; }
VertexStruct::P3_N3_T2::P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
VertexStruct::P3_N3_T2::P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt) { p = vp; n = vn; t = vt; }

VertexStruct::P3_N3_T2_C4::P3_N3_T2_C4() {}
VertexStruct::P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp) { p = vp; }
VertexStruct::P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
VertexStruct::P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt) { p = vp; n = vn; t = vt; }
VertexStruct::P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt, ct::fvec4 vc) { p = vp; n = vn; t = vt; c = vc; }

VertexStruct::P3_N3_C4::P3_N3_C4() {}
VertexStruct::P3_N3_C4::P3_N3_C4(ct::fvec3 vp) { p = vp; }
VertexStruct::P3_N3_C4::P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
VertexStruct::P3_N3_C4::P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc) { p = vp; n = vn; c = vc; }

VertexStruct::P3_N3_C4_D1::P3_N3_C4_D1() {}
VertexStruct::P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp) { p = vp; }
VertexStruct::P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
VertexStruct::P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc) { p = vp; n = vn; c = vc; }
VertexStruct::P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc, float vd) { p = vp; n = vn; c = vc; d = vd; }

VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4() {}
VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp) { p = vp; }
VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan) { p = vp; n = vn; tan = vtan; }
VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan) { p = vp; n = vn; tan = vtan; btan = vbtan; }
VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt) { p = vp; n = vn; tan = vtan; btan = vbtan; t = vt; }
VertexStruct::P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt, ct::fvec4 vc) { p = vp; n = vn; tan = vtan; btan = vbtan; t = vt; c = vc; }