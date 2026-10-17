// callers: 0x254020 0x255650 0x25a860 0x25cf50 0x25e3a0

int FUN_00258870(int param_1,int param_2,undefined4 param_3,uint param_4,uint param_5,uint param_6,
                int param_7,int param_8,uint *param_9,uint *param_10)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  undefined8 uVar11;
  int local_14;
  uint local_10;
  uint local_c;
  int local_8;
  
  local_14 = 0;
  uVar3 = (byte)(&DAT_0025ed88)[param_5] & 0x3c;
  if (((((&DAT_0025ed88)[param_5] & 1) == 0) && (param_5 != 0xc)) &&
     (((int)param_5 < 0xe || (0xf < (int)param_5)))) {
    uVar2 = 0;
    local_10 = 0;
    local_c = 0;
    if (param_4 == 0) {
      param_4 = 1;
    }
    if (param_6 == 0) {
      param_6 = (uVar3 * param_1 >> 3) + 0x3f & 0xffffffc0;
    }
    local_8 = param_2;
    local_14 = param_2 * param_6;
  }
  else {
    local_c = FUN_002580a0();
    local_10 = FUN_002580a0();
    uVar11 = FUN_002580a0();
    uVar10 = (uint)((ulonglong)uVar11 >> 0x20);
    uVar2 = (uint)uVar11;
    bVar1 = 0;
    local_8 = 0;
    if ((param_5 == 0xc) || ((0xd < (int)param_5 && ((int)param_5 < 0x10)))) {
      bVar1 = 1;
    }
    uVar4 = -(uint)bVar1 & 2;
    uVar9 = local_c;
    uVar5 = param_4;
    if (param_4 == 0) {
      uVar7 = uVar10;
      if (uVar10 <= uVar2) {
        uVar7 = uVar2;
      }
      uVar5 = local_c;
      if ((local_c <= uVar7) && (uVar5 = uVar10, uVar10 <= uVar2)) {
        uVar5 = uVar2;
      }
      uVar5 = uVar5 + 1;
      param_4 = uVar5;
    }
    for (; uVar5 != 0; uVar5 = uVar5 - 1) {
      uVar7 = (uint)((ulonglong)uVar11 >> 0x20);
      iVar8 = (int)uVar11;
      uVar10 = uVar4;
      if (uVar4 < uVar9) {
        uVar10 = uVar9;
      }
      uVar6 = uVar4;
      if (uVar4 < uVar7) {
        uVar6 = uVar7;
      }
      local_14 = local_14 +
                 ((1 << ((char)uVar6 + (char)uVar10 + (char)uVar11 & 0x1fU)) * uVar3 >> 3);
      if (uVar9 != 0) {
        uVar9 = uVar9 - 1;
      }
      if (uVar7 != 0) {
        uVar7 = uVar7 - 1;
      }
      if (iVar8 != 0) {
        iVar8 = iVar8 + -1;
      }
      uVar11 = CONCAT44(uVar7,iVar8);
    }
    param_1 = 0;
    if (param_7 != 0) {
      local_14 = (local_14 + 0x7fU & 0xffffff80) * 6;
    }
  }
  *param_9 = (((((uVar2 << 4 | local_10) << 4 | local_c) << 4 | param_4) << 8 | param_5) << 4 |
             (param_8 != 0) + 2) << 4 | -(uint)(param_7 != 0) & 4 | 9;
  if (param_1 == 0) {
    *param_10 = 0;
    return local_14;
  }
  *param_10 = (((param_6 >> 6) - 1) * 0x1000 | local_8 - 1U) << 0xc | param_1 - 1U;
  return local_14;
}

