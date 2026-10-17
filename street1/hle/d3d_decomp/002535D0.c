// callers: 0x1970c0 0x250f40 0x25d660

void FUN_002535d0(uint *param_1)

{
  float fVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  
  puVar2 = DAT_00261238;
  if (param_1 != (uint *)0x0) {
    uVar6 = DAT_00261238[0x681];
    if (uVar6 == DAT_00261238[0x685]) {
      uVar6 = DAT_00261238[0x686];
    }
    uVar7 = *param_1;
    uVar3 = -(uint)(uVar7 != 0) & uVar7;
    uVar8 = param_1[1];
    uVar4 = -(uint)(uVar8 != 0) & uVar8;
    uVar5 = *(uint *)(uVar6 + 0x10);
    if (uVar5 == 0) {
      uVar9 = 1 << ((byte)(*(uint *)(uVar6 + 0xc) >> 0x14) & 0xf);
    }
    else {
      uVar9 = (uVar5 & 0xfff) + 1;
    }
    uVar7 = uVar7 + param_1[2];
    if (uVar9 <= uVar7) {
      if (uVar5 == 0) {
        uVar7 = 1 << ((byte)(*(uint *)(uVar6 + 0xc) >> 0x14) & 0xf);
      }
      else {
        uVar7 = (uVar5 & 0xfff) + 1;
      }
    }
    if (uVar5 == 0) {
      uVar9 = 1 << (*(byte *)(uVar6 + 0xf) & 0xf);
    }
    else {
      uVar9 = (uVar5 >> 0xc & 0xfff) + 1;
    }
    uVar8 = uVar8 + param_1[3];
    if (uVar9 <= uVar8) {
      if (uVar5 == 0) {
        uVar8 = 1 << (*(byte *)(uVar6 + 0xf) & 0xf);
      }
      else {
        uVar8 = (uVar5 >> 0xc & 0xfff) + 1;
      }
    }
    fVar1 = (float)(int)uVar7;
    if ((int)uVar7 < 0) {
      fVar1 = fVar1 + 4.2949673e+09;
    }
    uVar6 = DAT_00261238[0x255];
    uVar5 = FUN_002529b0(fVar1 * (float)DAT_00261238[599] + 0.5);
    if (uVar6 < uVar5) {
      fVar1 = (float)(int)uVar6;
      if ((int)uVar6 < 0) {
        fVar1 = fVar1 + 4.2949673e+09;
      }
      uVar7 = FUN_002529b0(fVar1 / (float)puVar2[599]);
    }
    fVar1 = (float)(int)uVar8;
    if ((int)uVar8 < 0) {
      fVar1 = fVar1 + 4.2949673e+09;
    }
    uVar6 = puVar2[0x256];
    uVar5 = FUN_002529b0(fVar1 * (float)puVar2[600] + 0.5);
    if (uVar6 < uVar5) {
      fVar1 = (float)(int)uVar6;
      if ((int)uVar6 < 0) {
        fVar1 = fVar1 + 4.2949673e+09;
      }
      uVar8 = FUN_002529b0(fVar1 / (float)puVar2[600]);
    }
    puVar2[0x3b8] = uVar3;
    puVar2[0x3bb] = uVar8 - uVar4;
    puVar2[0x3b9] = uVar4;
    puVar2[0x3ba] = uVar7 - uVar3;
    puVar2[0x3bc] = param_1[4];
    puVar2[0x3bd] = param_1[5];
    DAT_00260d98 = DAT_00260d98 | 0x100;
  }
  FUN_00253d20(0,0,0);
  FUN_00250210();
  if (puVar2[1] <= *puVar2) {
    FUN_00252040();
  }
  uVar6 = FUN_00250c20();
  *puVar2 = uVar6;
  return;
}

