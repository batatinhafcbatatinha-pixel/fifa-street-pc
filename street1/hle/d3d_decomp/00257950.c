// callers: 0x251b80 0x25b070 0x25ca70 0x25cb10

void FUN_00257950(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  int iVar6;
  undefined4 *in_ECX;
  uint uVar7;
  int iVar8;
  uint uVar9;
  int local_8;
  
  uVar7 = DAT_00260d98;
  uVar4 = DAT_00260d98 & 0xffffffaf;
  uVar9 = DAT_00260d98 & 0x3fffff8f;
  DAT_00260d98 = uVar4;
  if (uVar9 != 0) {
    FUN_002578d0();
  }
  if (((uVar7 & 0x40000000) == 0) && (((uVar7 & 0x40) != 0 || (in_ECX[8] != param_1)))) {
    iVar1 = in_ECX[0x1e5];
    in_ECX[8] = param_1;
    uVar9 = *(uint *)(iVar1 + 4) & 0x10;
    puVar5 = (uint *)*in_ECX;
    if ((uint *)in_ECX[1] <= puVar5) {
      puVar5 = (uint *)FUN_00252040();
    }
    if ((uVar7 & 0x10) != 0) {
      *puVar5 = 0x401760;
      puVar5 = puVar5 + 1;
      uVar7 = 0;
      do {
        iVar6 = (uint)(byte)(&DAT_002cea90)[uVar7 + uVar9] * 0x10;
        *puVar5 = (&DAT_002636f8)[*(int *)(iVar6 + iVar1 + 0x14) * 3] * 0x100 +
                  *(int *)(iVar6 + 0x1c + iVar1);
        puVar5 = puVar5 + 1;
        uVar7 = uVar7 + 1;
      } while (uVar7 < 0x10);
    }
    if (param_1 == 0) {
      uVar7 = 0x41720;
      iVar6 = 0;
      do {
        iVar8 = iVar1 + (uint)(byte)(&DAT_002cea90)[iVar6 + uVar9] * 0x10;
        if (*(int *)(iVar8 + 0x1c) != 2) {
          if ((&DAT_00263700)[*(int *)(iVar8 + 0x14) * 3] != 0) {
            iVar2 = *(int *)((&DAT_00263700)[*(int *)(iVar8 + 0x14) * 3] + 4);
            iVar3 = *(int *)(iVar8 + 0x18);
            iVar8 = (&DAT_002636fc)[*(int *)(iVar8 + 0x14) * 3];
            *puVar5 = uVar7;
            puVar5[1] = iVar2 + iVar3 + iVar8;
            puVar5 = puVar5 + 2;
          }
        }
        uVar7 = uVar7 + 4;
        iVar6 = iVar6 + 1;
      } while (uVar7 < 0x41760);
      *in_ECX = puVar5;
      return;
    }
    local_8 = 0;
    uVar7 = 0x41720;
    do {
      iVar6 = (uint)(byte)(&DAT_002cea90)[local_8 + uVar9] * 0x10 + iVar1;
      if (*(int *)(iVar6 + 0x1c) != 2) {
        iVar8 = *(int *)(iVar6 + 0x14);
        if ((&DAT_00263700)[iVar8 * 3] != 0) {
          iVar2 = (&DAT_002636f8)[iVar8 * 3];
          iVar3 = *(int *)((&DAT_00263700)[iVar8 * 3] + 4);
          iVar6 = *(int *)(iVar6 + 0x18);
          iVar8 = (&DAT_002636fc)[iVar8 * 3];
          *puVar5 = uVar7;
          puVar5[1] = iVar2 * param_1 + iVar3 + iVar6 + iVar8;
          puVar5 = puVar5 + 2;
        }
      }
      local_8 = local_8 + 1;
      uVar7 = uVar7 + 4;
    } while (uVar7 < 0x41760);
    *in_ECX = puVar5;
  }
  return;
}

