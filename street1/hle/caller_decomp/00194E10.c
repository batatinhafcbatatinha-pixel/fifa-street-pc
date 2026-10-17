
undefined4 FUN_00194e10(void)

{
  uint *puVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  undefined4 uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int *in_ECX;
  uint uVar10;
  undefined4 *puVar11;
  undefined1 *puVar12;
  undefined4 *puVar13;
  int local_18;
  uint local_14;
  int local_c;
  
  local_c = 0;
  local_14 = 0;
  bVar4 = false;
  local_18 = FUN_00264172(*(undefined4 *)(in_ECX[0x10] + 0x14));
  iVar9 = *(int *)(in_ECX[0x10] + 0x14);
  if ((((iVar9 == 0xc) || (iVar9 == 0xe)) || (iVar9 == 0xf)) || (iVar9 == 0x24)) {
    switch(iVar9) {
    case 0xc:
      local_18 = 8;
      break;
    case 0xe:
    case 0xf:
      local_18 = 0x10;
      break;
    case 0x24:
      local_18 = local_18 / 2;
    }
    if (iVar9 == 0x24) {
      *(undefined1 *)(in_ECX[0x10] + 0x2c) = 1;
    }
  }
  if (*(int *)(in_ECX[0x10] + 0x24) == 0) {
    uVar5 = (*(code *)PTR_FUN_002d4d38)(0x14,"D3DTexture");
    *(undefined4 *)(in_ECX[0x10] + 0x24) = uVar5;
  }
  else {
    if ((&DAT_002f3450)[*in_ECX] == *(int *)(in_ECX[0x10] + 0x24)) {
      FUN_00253800(*in_ECX,0);
      (&DAT_002f3450)[*in_ECX] = 0;
    }
    thunk_FUN_002520c0(*(undefined4 *)(in_ECX[0x10] + 0x24));
  }
  uVar6 = FUN_00239875(*(undefined4 *)(in_ECX[0x10] + 0x18));
  if ((uVar6 & 2) != 0) {
    *(undefined4 *)(in_ECX[0x10] + 0x30) = 0xe0000000;
  }
  iVar9 = in_ECX[0x10];
  if ((((*(byte *)(iVar9 + 0x13) & 1) == 0) && (iVar2 = *(int *)(iVar9 + 0x14), iVar2 != 0xc)) &&
     ((iVar2 != 0xe && (iVar2 != 0xf)))) {
    bVar4 = true;
    in_ECX[1] = 3;
    in_ECX[2] = 3;
    in_ECX[3] = 3;
    in_ECX[4] = 3;
  }
  iVar2 = *(int *)(iVar9 + 0x14);
  iVar3 = *(int *)(iVar9 + 0xc);
  iVar7 = *(int *)(iVar9 + 8);
  if (((iVar2 == 0xc) || (iVar2 == 0xe)) || (iVar8 = iVar3, iVar2 == 0xf)) {
    iVar7 = (int)(iVar7 + (iVar7 >> 0x1f & 3U)) >> 2;
    iVar8 = (int)(iVar3 + (iVar3 >> 0x1f & 3U)) >> 2;
  }
  if (bVar4) {
    uVar6 = iVar7 * local_18 & 0x8000003f;
    if ((int)uVar6 < 0) {
      uVar6 = (uVar6 - 1 | 0xffffffc0) + 1;
    }
    local_14 = 0;
    if (uVar6 != 0) {
      local_14 = 0x40 - uVar6;
    }
    uVar6 = (iVar7 * local_18 + local_14) * iVar8;
  }
  else {
    uVar6 = 0;
    do {
      uVar6 = uVar6 + iVar8 * iVar7 * local_18;
      iVar7 = iVar7 >> 1;
      iVar8 = iVar8 >> 1;
      local_c = local_c + 1;
    } while (local_c < (*(int *)(iVar9 + 0x10) << 0x1a) >> 0x1a);
  }
  if ((iVar9 + 0x1cU < 0x80000000) || (0x8fffffff < iVar9 + 0x1cU)) {
    if (*(int *)(iVar9 + 0x20) != 0) {
      FUN_0015b9a4(*(int *)(iVar9 + 0x20));
      *(undefined4 *)(in_ECX[0x10] + 0x20) = 0;
      *(undefined4 *)(*(int *)(in_ECX[0x10] + 0x24) + 4) = 0;
    }
    uVar5 = FUN_0015b934(uVar6,0xffffffff,0x80,4);
    *(undefined4 *)(in_ECX[0x10] + 0x20) = uVar5;
    puVar11 = *(undefined4 **)(in_ECX[0x10] + 0x20);
    for (uVar10 = uVar6 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
      *puVar11 = 0;
      puVar11 = puVar11 + 1;
    }
    for (uVar10 = uVar6 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
      *(undefined1 *)puVar11 = 0;
      puVar11 = (undefined4 *)((int)puVar11 + 1);
    }
    iVar9 = in_ECX[0x10];
    puVar11 = *(undefined4 **)(iVar9 + 0x20);
    if (puVar11 == (undefined4 *)0x0) {
      return 0;
    }
    puVar12 = *(undefined1 **)(iVar9 + 0x1c);
    if (bVar4) {
      local_c = 0;
      if (0 < *(int *)(iVar9 + 0xc)) {
        do {
          iVar9 = 0;
          if (0 < *(int *)(in_ECX[0x10] + 8) * local_18) {
            do {
              *(undefined1 *)puVar11 = *puVar12;
              puVar11 = (undefined4 *)((int)puVar11 + 1);
              puVar12 = puVar12 + 1;
              iVar9 = iVar9 + 1;
            } while (iVar9 < *(int *)(in_ECX[0x10] + 8) * local_18);
          }
          if (0 < (int)local_14) {
            puVar13 = puVar11;
            for (uVar6 = local_14 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
              *puVar13 = 0;
              puVar13 = puVar13 + 1;
            }
            for (uVar6 = local_14 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
              *(undefined1 *)puVar13 = 0;
              puVar13 = (undefined4 *)((int)puVar13 + 1);
            }
            puVar11 = (undefined4 *)((int)puVar11 + local_14);
          }
          local_c = local_c + 1;
        } while (local_c < *(int *)(in_ECX[0x10] + 0xc));
      }
    }
    else if (0 < (int)uVar6) {
      do {
        *(undefined1 *)puVar11 = *puVar12;
        puVar11 = (undefined4 *)((int)puVar11 + 1);
        puVar12 = puVar12 + 1;
        uVar6 = uVar6 - 1;
      } while (uVar6 != 0);
    }
    iVar9 = in_ECX[0x10];
    FUN_00265a66(*(int *)(iVar9 + 8),*(undefined4 *)(iVar9 + 0xc),
                 (*(int *)(iVar9 + 0x10) << 0x1a) >> 0x1a,0,*(undefined4 *)(iVar9 + 0x14),0,
                 *(undefined4 *)(iVar9 + 0x24),0,*(int *)(iVar9 + 8) * local_18 + local_14);
    iVar9 = in_ECX[0x10];
    uVar5 = *(undefined4 *)(iVar9 + 0x20);
  }
  else {
    FUN_00265a66(*(int *)(iVar9 + 8),iVar3,(*(int *)(iVar9 + 0x10) << 0x1a) >> 0x1a,0,
                 *(undefined4 *)(iVar9 + 0x14),0,*(undefined4 *)(iVar9 + 0x24),0,
                 *(int *)(iVar9 + 8) * local_18 + local_14);
    iVar9 = in_ECX[0x10];
    uVar5 = *(undefined4 *)(iVar9 + 0x1c);
  }
  FUN_00257e10(*(undefined4 *)(iVar9 + 0x24),uVar5);
  uVar6 = FUN_00239875(*(undefined4 *)(in_ECX[0x10] + 0x18));
  if ((uVar6 & 4) != 0) {
    puVar1 = (uint *)(*(int *)(in_ECX[0x10] + 0x24) + 0xc);
    *puVar1 = *puVar1 | 4;
  }
  DAT_002f3d78 = 1;
  *(undefined1 *)(in_ECX + 8) = 1;
  return 1;
}

