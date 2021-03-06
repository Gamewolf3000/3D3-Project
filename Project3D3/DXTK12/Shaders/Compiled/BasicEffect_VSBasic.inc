#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0     NONE   float   xyzw
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// COLOR                    0   xyzw        0     NONE   float   xyzw
// COLOR                    1   xyzw        1     NONE   float   xyzw
// SV_Position              0   xyzw        2      POS   float   xyzw
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[26], immediateIndexed
dcl_input v0.xyzw
dcl_output o0.xyzw
dcl_output o1.xyzw
dcl_output_siv o2.xyzw, position
mov o0.xyzw, cb0[0].xyzw
dp4_sat o1.w, v0.xyzw, cb0[14].xyzw
mov o1.xyz, l(0,0,0,0)
dp4 o2.x, v0.xyzw, cb0[22].xyzw
dp4 o2.y, v0.xyzw, cb0[23].xyzw
dp4 o2.z, v0.xyzw, cb0[24].xyzw
dp4 o2.w, v0.xyzw, cb0[25].xyzw
ret 
// Approximately 0 instruction slots used
#endif

const BYTE BasicEffect_VSBasic[] =
{
     68,  88,  66,  67,  79, 104, 
     27, 224, 166, 119, 111,  47, 
    140, 225, 247, 142,  10, 231, 
    129, 223,   1,   0,   0,   0, 
     56,   2,   0,   0,   4,   0, 
      0,   0,  48,   0,   0,   0, 
    100,   0,   0,   0, 208,   0, 
      0,   0,   4,   2,   0,   0, 
     73,  83,  71,  78,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,  15,   0,   0,  83,  86, 
     95,  80, 111, 115, 105, 116, 
    105, 111, 110,   0,  79,  83, 
     71,  78, 100,   0,   0,   0, 
      3,   0,   0,   0,   8,   0, 
      0,   0,  80,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  80,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,  15,   0, 
      0,   0,  86,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,  15,   0, 
      0,   0,  67,  79,  76,  79, 
     82,   0,  83,  86,  95,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0, 171, 171,  83,  72, 
     69,  88,  44,   1,   0,   0, 
     80,   0,   1,   0,  75,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     26,   0,   0,   0,  95,   0, 
      0,   3, 242,  16,  16,   0, 
      0,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      0,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      1,   0,   0,   0, 103,   0, 
      0,   4, 242,  32,  16,   0, 
      2,   0,   0,   0,   1,   0, 
      0,   0,  54,   0,   0,   6, 
    242,  32,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  17,  32,   0,   8, 
    130,  32,  16,   0,   1,   0, 
      0,   0,  70,  30,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     14,   0,   0,   0,  54,   0, 
      0,   8, 114,  32,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     17,   0,   0,   8,  18,  32, 
     16,   0,   2,   0,   0,   0, 
     70,  30,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,  22,   0, 
      0,   0,  17,   0,   0,   8, 
     34,  32,  16,   0,   2,   0, 
      0,   0,  70,  30,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     23,   0,   0,   0,  17,   0, 
      0,   8,  66,  32,  16,   0, 
      2,   0,   0,   0,  70,  30, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  24,   0,   0,   0, 
     17,   0,   0,   8, 130,  32, 
     16,   0,   2,   0,   0,   0, 
     70,  30,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,  25,   0, 
      0,   0,  62,   0,   0,   1, 
     82,  84,  83,  48,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  24,   0, 
      0,   0,   0,   0,   0,   0, 
     44,   0,   0,   0,  29,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,  36,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0
};
