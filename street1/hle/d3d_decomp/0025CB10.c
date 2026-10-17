// callers: 0x1a08a0

void FUN_0025cb10(int *param_1,uint param_2,int *param_3)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  ushort uVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  uint uVar11;
  int iVar12;
  uint uVar13;
  int iVar14;
  int *piVar15;
  int *piVar16;
  int local_8;
  
  puVar9 = DAT_00261238;
  FUN_00257950(DAT_00261238[7]);
  puVar9[2] = puVar9[2] | 0x800;
  puVar10 = (undefined4 *)FUN_00252050(0x209);
  *puVar10 = 0x417fc;
  puVar10[1] = param_1;
  param_1 = puVar10 + 2;
  uVar11 = (uint)-(int)param_1 >> 2 & 7;
  if (uVar11 * 2 + 0x3e <= param_2) {
    if (uVar11 != 0) {
      iVar12 = uVar11 - 1;
      *param_1 = iVar12 * 0x40000 + 0x40001800;
      piVar15 = param_3;
      for (iVar14 = iVar12; param_1 = param_1 + 1, iVar14 != 0; iVar14 = iVar14 + -1) {
        *param_1 = *piVar15;
        piVar15 = piVar15 + 1;
      }
      param_3 = param_3 + iVar12;
      param_1 = puVar10 + uVar11 + 2;
      param_2 = param_2 + iVar12 * -2;
    }
    if (param_2 % 0x3fe < 0x3e) {
      local_8 = 0x1ff;
    }
    else {
      local_8 = ((param_2 % 0x3fe >> 1) + 1 & 0xfffffff0) - 1;
    }
    do {
      *param_1 = local_8 * 0x40000 + 0x40001800;
      uVar11 = local_8 - 0xfU >> 4;
      uVar1 = *(undefined8 *)(param_3 + 1);
      uVar2 = *(undefined8 *)(param_3 + 3);
      uVar3 = *(undefined8 *)(param_3 + 5);
      uVar4 = *(undefined8 *)(param_3 + 7);
      uVar5 = *(undefined8 *)(param_3 + 9);
      uVar6 = *(undefined8 *)(param_3 + 0xb);
      uVar7 = *(undefined8 *)(param_3 + 0xd);
      param_1[1] = *param_3;
      *(undefined8 *)(param_1 + 2) = uVar1;
      *(undefined8 *)(param_1 + 4) = uVar2;
      *(undefined8 *)(param_1 + 6) = uVar3;
      *(undefined8 *)(param_1 + 8) = uVar4;
      *(undefined8 *)(param_1 + 10) = uVar5;
      *(undefined8 *)(param_1 + 0xc) = uVar6;
      *(undefined8 *)(param_1 + 0xe) = uVar7;
      piVar16 = param_3 + 0xf;
      piVar15 = param_1;
      do {
        uVar1 = *(undefined8 *)(piVar16 + 2);
        uVar2 = *(undefined8 *)(piVar16 + 4);
        uVar3 = *(undefined8 *)(piVar16 + 6);
        uVar4 = *(undefined8 *)(piVar16 + 8);
        uVar5 = *(undefined8 *)(piVar16 + 10);
        uVar6 = *(undefined8 *)(piVar16 + 0xc);
        uVar7 = *(undefined8 *)(piVar16 + 0xe);
        *(undefined8 *)(piVar15 + 0x10) = *(undefined8 *)piVar16;
        *(undefined8 *)(piVar15 + 0x12) = uVar1;
        *(undefined8 *)(piVar15 + 0x14) = uVar2;
        *(undefined8 *)(piVar15 + 0x16) = uVar3;
        *(undefined8 *)(piVar15 + 0x18) = uVar4;
        *(undefined8 *)(piVar15 + 0x1a) = uVar5;
        *(undefined8 *)(piVar15 + 0x1c) = uVar6;
        *(undefined8 *)(piVar15 + 0x1e) = uVar7;
        piVar16 = piVar16 + 0x10;
        uVar11 = uVar11 - 1;
        piVar15 = piVar15 + 0x10;
      } while (uVar11 != 0);
      param_3 = param_3 + local_8;
      param_2 = param_2 + local_8 * -2;
      *puVar9 = param_1 + local_8 + 1;
      param_1 = (int *)FUN_00252050(0x204);
      local_8 = 0x1ff;
    } while (0x3fd < param_2);
  }
  uVar13 = param_2 >> 1;
  *param_1 = uVar13 * 0x40000 + 0x40001800;
  piVar16 = param_3;
  piVar15 = param_1;
  for (uVar11 = uVar13; piVar15 = piVar15 + 1, uVar11 != 0; uVar11 = uVar11 - 1) {
    *piVar15 = *piVar16;
    piVar16 = piVar16 + 1;
  }
  param_1 = param_1 + uVar13 + 1;
  if ((param_2 & 1) != 0) {
    uVar8 = *(ushort *)(param_3 + uVar13);
    *param_1 = 0x41808;
    param_1[1] = (uint)uVar8;
    param_1 = param_1 + 2;
  }
  *param_1 = 0x417fc;
  param_1[1] = 0;
  uVar11 = puVar9[2];
  *puVar9 = param_1 + 2;
  puVar9[2] = uVar11 & 0xffffe7ff;
  if ((uVar11 & 0x1000) != 0) {
    FUN_00251cd0(1);
  }
  return;
}

