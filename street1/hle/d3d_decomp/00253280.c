// callers: 0x19a210 0x2540a0

void FUN_00253280(int param_1,uint *param_2,uint param_3,int param_4,uint *param_5)

{
  byte bVar1;
  byte bVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  bool bVar8;
  undefined4 *puVar9;
  char cVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  uint *puVar14;
  undefined4 *puVar15;
  uint uVar16;
  int *piVar17;
  int iVar18;
  uint *puVar19;
  uint *puVar20;
  uint local_34;
  uint local_28;
  uint local_24;
  uint local_18 [5];
  int local_4;
  
  bVar1 = *(byte *)(param_1 + 0xd);
  bVar2 = (&DAT_0025ed88)[bVar1];
  uVar16 = (bVar2 & 0x38) >> 3;
  uVar11 = FUN_00252e10(param_1);
  uVar12 = FUN_00252e10(param_4);
  if ((bVar2 & 1) == 0) {
    cVar10 = FUN_00258170((uint)bVar1);
    bVar8 = false;
    if (cVar10 != '\0') goto LAB_002532ce;
  }
  else {
LAB_002532ce:
    bVar8 = true;
  }
  puVar9 = DAT_00261238;
  iVar18 = DAT_00261238[0x64b];
  if (param_3 == 0) {
    param_3 = 1;
  }
  if (bVar8) {
    local_34 = FUN_00252e80(param_1);
    if (local_34 < 0x1fc1) {
      uVar11 = local_34 + 0x3f & 0xffffffc0;
      local_28 = 1;
    }
    else {
      uVar11 = 0x1000;
      local_28 = local_34 >> 0xc;
      local_34 = 0x1000;
    }
    uVar12 = uVar11;
    if (iVar18 != 0) {
      local_34 = local_34 / uVar16;
      goto LAB_00253350;
    }
    uVar16 = 1;
  }
  else {
LAB_00253350:
    if (iVar18 != 0) goto LAB_00253371;
  }
  if (uVar16 == 1) {
    iVar18 = 1;
  }
  else if (uVar16 == 2) {
    iVar18 = 4;
  }
  else {
    iVar18 = 10;
  }
LAB_00253371:
  puVar13 = (undefined4 *)*puVar9;
  if ((undefined4 *)puVar9[1] <= puVar13) {
    puVar13 = (undefined4 *)FUN_00252040();
  }
  uVar3 = *(undefined4 *)(param_4 + 4);
  uVar4 = *(undefined4 *)(param_1 + 4);
  *puVar13 = 0x86308;
  puVar13[1] = uVar4;
  puVar13[2] = uVar3;
  puVar13[3] = 0x86300;
  puVar13[4] = iVar18;
  puVar13[5] = uVar12 << 0x10 | uVar11 & 0xffff;
  *puVar9 = puVar13 + 6;
  do {
    puVar13 = DAT_00261238;
    local_24 = param_3;
    if (1 < param_3) {
      local_24 = 1;
    }
    param_3 = param_3 - local_24;
    if (param_2 == (uint *)0x0) {
      puVar20 = local_18 + 2;
      if (local_24 != 0) {
        uVar12 = *(uint *)(param_1 + 0x10);
        piVar17 = &local_4;
        uVar11 = local_24;
        do {
          piVar17[-2] = 0;
          if (uVar12 == 0) {
            iVar18 = 1 << (*(byte *)(param_1 + 0xf) & 0xf);
          }
          else {
            iVar18 = (uVar12 >> 0xc & 0xfff) + 1;
          }
          *piVar17 = iVar18;
          piVar17[-3] = 0;
          if (uVar12 == 0) {
            iVar18 = 1 << ((byte)(*(uint *)(param_1 + 0xc) >> 0x14) & 0xf);
          }
          else {
            iVar18 = (uVar12 & 0xfff) + 1;
          }
          piVar17[-1] = iVar18;
          piVar17 = piVar17 + 4;
          uVar11 = uVar11 - 1;
        } while (uVar11 != 0);
      }
    }
    else {
      puVar20 = param_2;
      param_2 = param_2 + 4;
    }
    if (param_5 == (uint *)0x0) {
      uVar12 = 0;
      puVar19 = local_18;
      puVar14 = puVar20;
      if (local_24 != 0) {
        do {
          local_18[uVar12 * 2] = *puVar14;
          local_18[uVar12 * 2 + 1] = puVar14[1];
          uVar12 = uVar12 + 1;
          puVar14 = puVar14 + 4;
        } while (uVar12 < local_24);
      }
    }
    else {
      puVar19 = param_5;
      param_5 = param_5 + 2;
    }
    if ((bVar8) && (local_24 != 0)) {
      puVar14 = puVar20 + 2;
      uVar12 = local_24;
      do {
        puVar14[-2] = 0;
        puVar14[-1] = 0;
        *puVar14 = local_34;
        puVar14[1] = local_28;
        puVar14 = puVar14 + 4;
        uVar12 = uVar12 - 1;
      } while (uVar12 != 0);
    }
    if (local_24 != 0) {
      puVar20 = puVar20 + 1;
      do {
        puVar15 = (undefined4 *)*puVar13;
        if ((undefined4 *)puVar13[1] <= puVar15) {
          puVar15 = (undefined4 *)FUN_00252040();
        }
        uVar12 = *puVar20;
        uVar11 = puVar20[-1];
        uVar16 = puVar19[1];
        uVar5 = *puVar19;
        uVar6 = puVar20[2];
        uVar7 = puVar20[1];
        *puVar15 = 0xc4300;
        puVar15[1] = uVar12 << 0x10 | uVar11 & 0xffff;
        puVar15[2] = uVar16 << 0x10 | uVar5 & 0xffff;
        puVar15[3] = (uVar6 - uVar12) * 0x10000 ^ uVar7 - uVar11 & 0xffff;
        *puVar13 = puVar15 + 4;
        puVar19 = puVar19 + 2;
        puVar20 = puVar20 + 4;
        local_24 = local_24 - 1;
      } while (local_24 != 0);
    }
  } while (param_3 != 0);
  if (*(int *)(param_1 + 0x14) != 0) {
    param_1 = *(int *)(param_1 + 0x14);
  }
  *(undefined4 *)(param_1 + 8) = puVar9[0xb];
  if (*(int *)(param_4 + 0x14) == 0) {
    *(undefined4 *)(param_4 + 8) = puVar9[0xb];
    return;
  }
  *(undefined4 *)(*(int *)(param_4 + 0x14) + 8) = puVar9[0xb];
  return;
}

