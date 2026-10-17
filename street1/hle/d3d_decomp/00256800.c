// callers: 0x2578d0

void FUN_00256800(undefined4 *param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  undefined *puVar5;
  uint uVar6;
  int *piVar7;
  uint *puVar8;
  uint uVar9;
  uint uVar10;
  uint *puVar11;
  uint uVar12;
  uint uVar13;
  uint local_18;
  int local_14;
  int local_c;
  
  puVar8 = param_1 + 0x1dd;
  local_14 = 0x41b14;
  puVar11 = &DAT_00260da4;
  local_c = 4;
  do {
    if ((param_2 & 1) != 0) {
      uVar1 = puVar11[1];
      uVar2 = *puVar11;
      uVar3 = *(uint *)(&DAT_00261128 + (puVar11[0x1b] & 0xffff) * 4);
      uVar4 = puVar11[-1];
      uVar12 = puVar11[6] << 0x1a | puVar11[10] | puVar11[8];
      uVar13 = uVar12 | 0x3ffc0;
      if (puVar8[0x205] != 0) {
        uVar13 = uVar12 | 0x4003ffc0;
      }
      uVar12 = puVar11[3];
      uVar10 = puVar11[2];
      local_18 = 0x2000;
      if ((2 < uVar12) || (2 < uVar10)) {
        if ((uVar12 < 4) && (uVar10 < 4)) {
          if (puVar11[7] == 0) {
            uVar12 = 1;
            uVar10 = 1;
          }
          else {
            uVar12 = 2;
            uVar10 = 2;
            uVar13 = uVar13 | (puVar11[7] - 1) * 0x10;
          }
        }
        else {
          if ((uVar12 == 5) || (uVar10 == 5)) {
            local_18 = 0x4000;
          }
          local_18 = local_18 | 0x70000;
          uVar12 = 2;
          uVar10 = 4;
        }
      }
      *puVar8 = uVar13 | 0x40000000;
      uVar6 = FUN_002529b0(((float)puVar11[5] + (float)param_1[0x25a]) * 256.0 + 0.5);
      if ((int)uVar6 < -0x1000) {
        uVar6 = 0xfffff000;
      }
      else if (0xfff < (int)uVar6) {
        uVar6 = 0xfff;
      }
      uVar9 = 0xc0000000;
      if (puVar11[0xb] < 0x19) {
        uVar9 = puVar11[9];
      }
      puVar5 = (&PTR_FUN_0025ee58)[puVar11[4] + uVar12 * 3];
      piVar7 = (int *)*param_1;
      if ((int *)param_1[1] <= piVar7) {
        piVar7 = (int *)FUN_00252040();
      }
      *piVar7 = local_14 + 0x3fff4;
      piVar7[1] = (uVar1 << 8 | uVar2) << 8 | uVar3 | uVar4;
      piVar7[2] = uVar13;
      piVar7[3] = local_14;
      piVar7[4] = uVar6 & 0x1fff | (uint)puVar5 | uVar10 << 0x18 | uVar9 | local_18;
      *param_1 = piVar7 + 5;
    }
    puVar8 = puVar8 + 1;
    local_14 = local_14 + 0x40;
    param_2 = param_2 >> 1;
    puVar11 = puVar11 + 0x20;
    local_c = local_c + -1;
  } while (local_c != 0);
  return;
}

