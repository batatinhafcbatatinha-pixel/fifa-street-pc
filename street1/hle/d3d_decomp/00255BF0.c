// callers: 0x25d080

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00255bf0(undefined4 *param_1)

{
  int *piVar1;
  uint uVar2;
  undefined4 uVar3;
  uint *in_ECX;
  int iVar4;
  undefined4 *puVar5;
  int unaff_FS_OFFSET;
  uint local_164;
  undefined1 local_160 [12];
  uint local_154;
  undefined1 local_150 [16];
  undefined1 local_140 [12];
  uint local_134;
  undefined1 local_130 [12];
  uint local_124;
  undefined1 local_120 [16];
  undefined1 local_110 [16];
  undefined1 local_100 [16];
  undefined1 local_f0 [16];
  undefined1 local_e0 [16];
  undefined1 local_d0 [16];
  undefined1 local_c0 [16];
  undefined1 local_b0 [16];
  undefined1 local_a0 [16];
  undefined1 local_90 [16];
  undefined1 local_80 [16];
  undefined1 local_70 [16];
  undefined1 local_60 [16];
  undefined1 local_50 [16];
  undefined1 local_40 [16];
  undefined1 local_30 [16];
  undefined1 local_20 [16];
  undefined1 local_10 [16];
  
  uVar2 = FUN_001609a7(0x60,0xac800000);
  in_ECX[0x922] = uVar2;
  if (uVar2 == 0) {
    return -0x7ff8fff2;
  }
  in_ECX[0xc] = uVar2;
  in_ECX[0x923] = uVar2 + 0x40;
  in_ECX[0x924] = uVar2 + 0x20;
  puVar5 = (undefined4 *)(uVar2 + 0x20);
  for (iVar4 = 0x10; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar5 = 0;
    puVar5 = puVar5 + 1;
  }
  iVar4 = FUN_002517f0();
  if (-1 < iVar4) {
    DAT_00260d98 = DAT_00260d98 | 0xff7f7f;
    in_ECX[0x253] = 0x40;
    FUN_0025a1f4();
    FUN_00259ee2(3,0x3d,0,0x7ffafff,local_120);
    FUN_00259ee2(5,2,0,0x7ffafff,local_f0);
    FUN_00259ee2(4,3,0,0x7ffafff,local_d0);
    FUN_00259ee2(9,0x3d,0,0x7ffafff,local_130);
    in_ECX[0x91f] = local_124;
    FUN_00259ee2(10,0x3d,0,0x7ffafff,local_140);
    in_ECX[0x920] = local_134;
    FUN_00259ee2(0xb,0x3d,0,0x7ffafff,local_160);
    in_ECX[0x921] = local_154;
    FUN_00259ee2(2,3,in_ECX[0x924],0x1f,local_c0);
    FUN_00259ee2(7,0x3d,in_ECX[0x923],0x1f,local_110);
    FUN_00259ee2(0xc,0x3d,0x80000000,0x10000000,local_e0);
    FUN_00259ee2(8,0x3d,in_ECX[0xc],0x20,local_100);
    FUN_00259ee2(6,2,0,0x7ffafff,local_150);
    FUN_00259c6a(0x206e,0,local_150,0,&local_164);
    FUN_00259ccd(local_150);
    FUN_00259ccd(local_e0);
    FUN_00259ccd(local_c0);
    FUN_00259ccd(local_110);
    FUN_00259ccd(local_d0);
    FUN_00259ccd(local_f0);
    FUN_00259ccd(local_120);
    FUN_00259ccd(local_130);
    FUN_00259ccd(local_140);
    FUN_00259ccd(local_160);
    FUN_00259ccd(local_100);
    FUN_00259fb1(0xd,0x97,local_60);
    FUN_00259fb1(0xe,0x39,local_b0);
    FUN_00259fb1(0x10,0x9f,local_10);
    FUN_00259fb1(0x11,0x62,local_90);
    FUN_00259fb1(0x12,0x44,local_40);
    FUN_00259fb1(0x13,0x57,local_a0);
    FUN_00259fb1(0x14,0x43,local_20);
    FUN_00259fb1(0x15,0x12,local_30);
    FUN_00259fb1(0x16,0x72,local_50);
    FUN_00259fb1(0x18,0x19,local_70);
    FUN_00259fb1(0x19,0x30,local_80);
    _DAT_00260d8c = in_ECX[0x70a];
    in_ECX[0x24d] = _DAT_00260d8c;
    in_ECX[0x708] = local_164;
    _DAT_80000000 = (in_ECX[9] & 0xfffffff) + 1;
    FUN_00251bb0();
    uVar2 = *in_ECX ^ *(uint *)(in_ECX[0x708] + 0x44);
    while ((uVar2 & 0xfffffff) != 0) {
      FUN_00251730();
      uVar2 = *in_ECX ^ *(uint *)(in_ECX[0x708] + 0x44);
    }
    _DAT_80000000 = 0xdeadbeef;
    FUN_00255340();
    iVar4 = FUN_00255650(param_1);
    if (-1 < iVar4) {
      uVar3 = FUN_00252e10(in_ECX[0x686]);
      iVar4 = FUN_00258afd(*param_1,param_1[1],param_1[0xb],param_1[10],param_1[2],param_1[0xc],
                           uVar3);
      if (-1 < iVar4) {
        FUN_00251cd0(0);
        do {
          iVar4 = FUN_00253ad0();
        } while (iVar4 != 0);
        in_ECX[0x1e5] = (uint)&DAT_002638d0;
        FUN_002550e0(4);
        FUN_002550e0(2);
        puVar5 = &DAT_002637b8;
        for (iVar4 = 0x46; iVar4 != 0; iVar4 = iVar4 + -1) {
          *puVar5 = 0;
          puVar5 = puVar5 + 1;
        }
        DAT_002637bc = 0x10;
        FUN_00252fb0(in_ECX[0x685],in_ECX[0x688]);
        FUN_00255450();
        FUN_00258d70(0,0,3,0,0x3f800000,0);
        iVar4 = *(int *)(*(int *)(unaff_FS_OFFSET + 0x20) + 0x250);
        if ((iVar4 != 0) && (piVar1 = *(int **)(iVar4 + 0x20), piVar1 != (int *)0x0)) {
          piVar1[1] = (int)(in_ECX + 0x91e);
          *piVar1 = (int)(in_ECX + 0x779);
          piVar1[2] = in_ECX[0x686];
          piVar1[3] = (int)(in_ECX + 0x70a);
          piVar1[5] = (int)(in_ECX + 0x235);
          piVar1[6] = (int)(in_ECX + 0x237);
          piVar1[7] = (int)&LAB_00254220;
          piVar1[8] = -0x54321100;
        }
        FUN_00252d50(5);
        FUN_00252da0(0);
        iVar4 = 0;
      }
    }
  }
  return iVar4;
}

