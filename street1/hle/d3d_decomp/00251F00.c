// callers: 0x252040 0x252050

uint FUN_00251f00(int param_1,uint param_2)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  
  puVar5 = DAT_00261238;
  if ((DAT_00261238[2] & 4) != 0) {
    puVar1 = DAT_00261238 + 0x1d9;
    DAT_00261238[0x1da] = DAT_00261238[0x1da] + (*DAT_00261238 - *(int *)(*puVar1 + 4));
    uVar6 = *(uint *)(*puVar1 + 4);
    *puVar5 = uVar6;
    return uVar6;
  }
  uVar8 = *DAT_00261238;
  uVar3 = *(uint *)DAT_00261238[0xc];
  uVar6 = FUN_002519c0();
  uVar4 = puVar5[10];
  uVar7 = uVar8 + param_2;
  if ((uVar4 <= uVar8 + param_2 + 0x4000) && (uVar7 = uVar4, uVar4 < param_1 + uVar8)) {
    puVar5[0x10] = puVar5[0x10] + 1;
    puVar5[0x11] = *puVar5 - puVar5[9];
    *(uint *)*puVar5 = (puVar5[9] & 0xfffffff) + 1;
    if (*puVar5 < uVar6) {
      uVar6 = puVar5[9] + 4;
    }
    uVar8 = puVar5[9];
    if (uVar6 == uVar8) {
      uVar6 = uVar8 + 4;
    }
    *puVar5 = uVar8;
    uVar7 = uVar8 + param_2;
  }
  param_2 = uVar7;
  if ((uVar8 < uVar6) && (uVar6 <= param_2)) {
    uVar6 = puVar5[0xd];
    uVar4 = puVar5[0x12];
    do {
      iVar2 = uVar6 * 8;
      uVar6 = uVar6 - 1 & puVar5[0xe];
      uVar7 = *(uint *)(uVar4 + 4 + uVar6 * 8);
      if (uVar7 < uVar8) {
        uVar7 = uVar7 + puVar5[0x11];
      }
    } while ((((DAT_002639ec >> 1) + uVar8 < uVar7) && (uVar6 != puVar5[0xd])) &&
            (uVar3 < *(uint *)(uVar4 + uVar6 * 8)));
    FUN_00251d90(*(undefined4 *)(uVar4 + iVar2),8);
  }
  puVar5[1] = param_2 - 0x204;
  if ((puVar5[2] & 0x800) == 0) {
    FUN_00251cd0(3);
    FUN_00251bb0();
    return *puVar5;
  }
  puVar5[2] = puVar5[2] | 0x1000;
  FUN_00251bb0();
  return *puVar5;
}

