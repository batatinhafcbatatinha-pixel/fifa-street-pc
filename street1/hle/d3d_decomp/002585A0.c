// callers: 0x2587d0 0x25d010

void FUN_002585a0(int param_1,int param_2,int param_3,uint *param_4,uint *param_5,uint *param_6,
                 uint *param_7,undefined4 *param_8)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  byte bVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  uint local_18;
  uint local_14;
  uint local_c;
  uint local_4;
  
  iVar2 = param_1;
  uVar4 = *(uint *)(param_1 + 0x10);
  local_14 = *(uint *)(param_1 + 4) | 0x80000000;
  uVar3 = (byte)(&DAT_0025ed88)[*(byte *)(param_1 + 0xd)] & 0x3c;
  if (uVar4 == 0) {
    uVar4 = *(uint *)(param_1 + 0xc);
    bVar1 = *(byte *)(param_1 + 0xd);
    uVar9 = uVar4 >> 0x14 & 0xf;
    uVar12 = uVar4 >> 0x18 & 0xf;
    uVar4 = uVar4 >> 0x1c;
    bVar5 = 0;
    if ((bVar1 == 0xc) || ((0xd < bVar1 && (bVar1 < 0x10)))) {
      bVar5 = 1;
    }
    uVar6 = -(uint)bVar5 & 2;
    if (param_2 != 0) {
      iVar10 = 0;
      local_4 = *(byte *)(param_1 + 0xe) & 0xf;
      uVar11 = uVar9;
      local_18 = uVar12;
      local_c = uVar4;
      if ((*(byte *)(param_1 + 0xe) & 0xf) != 0) {
        do {
          uVar8 = uVar6;
          if (uVar6 < uVar11) {
            uVar8 = uVar11;
          }
          uVar7 = uVar6;
          if (uVar6 < local_18) {
            uVar7 = local_18;
          }
          iVar10 = iVar10 + ((1 << ((char)uVar7 + (char)uVar8 & 0x1fU)) * uVar3 >> 3);
          if (uVar11 != 0) {
            uVar11 = uVar11 - 1;
          }
          if (local_18 != 0) {
            local_18 = local_18 - 1;
          }
          if (local_c != 0) {
            local_c = local_c - 1;
          }
          local_4 = local_4 - 1;
        } while (local_4 != 0);
      }
      local_14 = local_14 + (iVar10 + 0x7fU & 0xffffff80) * param_2;
    }
    if (param_3 != 0) {
      param_1 = param_3;
      do {
        uVar11 = uVar9;
        if (uVar9 <= uVar6) {
          uVar11 = uVar6;
        }
        uVar8 = uVar12;
        if (uVar12 <= uVar6) {
          uVar8 = uVar6;
        }
        local_14 = local_14 +
                   ((1 << ((char)uVar8 + (char)uVar11 + (char)uVar4 & 0x1fU)) * uVar3 >> 3);
        if (uVar9 != 0) {
          uVar9 = uVar9 - 1;
        }
        if (uVar12 != 0) {
          uVar12 = uVar12 - 1;
        }
        if (uVar4 != 0) {
          uVar4 = uVar4 - 1;
        }
        param_1 = param_1 + -1;
      } while (param_1 != 0);
    }
    uVar11 = uVar9;
    if (uVar9 <= uVar6) {
      uVar11 = uVar6;
    }
    uVar8 = uVar12;
    if (uVar12 <= uVar6) {
      uVar8 = uVar6;
    }
    bVar1 = *(byte *)(iVar2 + 0xd);
    bVar5 = (byte)uVar11;
    if (bVar1 == 0xc) {
      uVar6 = (1 << (bVar5 & 0x1f)) << 1;
    }
    else if ((bVar1 < 0xe) || (0xf < bVar1)) {
      uVar6 = (1 << (bVar5 & 0x1f)) * uVar3 >> 3;
    }
    else {
      uVar6 = (1 << (bVar5 & 0x1f)) << 2;
    }
    *param_7 = ((uVar4 << 4 | uVar12) << 4 | uVar9) << 0x14 | *(uint *)(iVar2 + 0xc) & 0xfffff;
    *param_8 = *(undefined4 *)(iVar2 + 0x10);
    uVar4 = (1 << ((char)uVar8 + bVar5 & 0x1f)) * uVar3 >> 3;
  }
  else {
    *param_7 = *(uint *)(param_1 + 0xc);
    uVar6 = ((uVar4 >> 0x18) + 1) * 0x40;
    uVar4 = ((uVar4 >> 0xc & 0xfff) + 1) * uVar6;
    *param_8 = *(undefined4 *)(param_1 + 0x10);
  }
  *param_4 = local_14;
  *param_5 = uVar6;
  *param_6 = uVar4;
  return;
}

