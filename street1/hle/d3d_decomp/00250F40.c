// callers: 0x252fb0 0x25dc80

void FUN_00250f40(void)

{
  float fVar1;
  uint uVar2;
  float fVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *in_ECX;
  undefined4 in_EDX;
  uint uVar9;
  int iVar10;
  float local_10;
  
  iVar10 = DAT_00261208;
  iVar4 = in_ECX[0x681];
  if (iVar4 == in_ECX[0x685]) {
    fVar1 = (float)in_ECX[0x25c];
    local_10 = (float)in_ECX[0x25b];
    uVar6 = *(uint *)(in_ECX[0x686] + 0x10);
    if (uVar6 == 0) {
      iVar4 = 1 << ((byte)(*(uint *)(in_ECX[0x686] + 0xc) >> 0x14) & 0xf);
    }
    else {
      iVar4 = (uVar6 & 0xfff) + 1;
    }
    fVar3 = (float)iVar4;
    if (iVar4 < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    uVar5 = FUN_002529b0(fVar3 * local_10 + 0.5);
    uVar6 = *(uint *)(in_ECX[0x686] + 0x10);
    if (uVar6 == 0) {
      iVar4 = 1 << (*(byte *)(in_ECX[0x686] + 0xf) & 0xf);
    }
    else {
      iVar4 = (uVar6 >> 0xc & 0xfff) + 1;
    }
    fVar3 = (float)iVar4;
    if (iVar4 < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    uVar6 = FUN_002529b0(fVar3 * fVar1 + 0.5);
    iVar4 = in_ECX[0x685];
    if (*(uint *)(iVar4 + 0x10) == 0) {
      uVar9 = 1 << ((byte)(*(uint *)(iVar4 + 0xc) >> 0x14) & 0xf);
    }
    else {
      uVar9 = (*(uint *)(iVar4 + 0x10) & 0xfff) + 1;
    }
    if (uVar9 <= uVar5) {
      if (*(uint *)(iVar4 + 0x10) == 0) {
        uVar5 = 1 << ((byte)(*(uint *)(iVar4 + 0xc) >> 0x14) & 0xf);
      }
      else {
        uVar5 = (*(uint *)(iVar4 + 0x10) & 0xfff) + 1;
      }
    }
    if (*(uint *)(iVar4 + 0x10) == 0) {
      uVar9 = 1 << (*(byte *)(iVar4 + 0xf) & 0xf);
    }
    else {
      uVar9 = (*(uint *)(iVar4 + 0x10) >> 0xc & 0xfff) + 1;
    }
    if (uVar6 < uVar9) goto LAB_002510fe;
    uVar6 = *(uint *)(iVar4 + 0x10);
    if (uVar6 == 0) {
      uVar6 = 1 << (*(byte *)(iVar4 + 0xf) & 0xf);
      goto LAB_002510fe;
    }
  }
  else {
    uVar6 = *(uint *)(iVar4 + 0x10);
    fVar1 = 1.0;
    local_10 = 1.0;
    iVar10 = DAT_0026120c;
    if (uVar6 == 0) {
      uVar5 = 1 << ((byte)(*(uint *)(iVar4 + 0xc) >> 0x14) & 0xf);
      uVar6 = 1 << (*(byte *)(iVar4 + 0xf) & 0xf);
      goto LAB_002510fe;
    }
    uVar5 = (uVar6 & 0xfff) + 1;
  }
  uVar6 = (uVar6 >> 0xc & 0xfff) + 1;
LAB_002510fe:
  uVar9 = in_ECX[0x683];
  uVar2 = in_ECX[2];
  in_ECX[2] = uVar2 & 0xffff7fff;
  if ((iVar10 != 0) && ((uVar9 & 0x200) == 0)) {
    local_10 = local_10 * 0.5;
    uVar5 = uVar5 + 1 >> 1;
    in_ECX[2] = uVar2 & 0xffff7fff | 0x8000;
    if (iVar10 == 2) {
      fVar1 = fVar1 * 0.5;
      uVar6 = uVar6 + 1 >> 1;
      uVar9 = uVar9 | 0x2000;
    }
    else {
      uVar9 = uVar9 | 0x1000;
    }
  }
  in_ECX[600] = fVar1;
  in_ECX[599] = local_10;
  in_ECX[0x255] = uVar5;
  in_ECX[0x256] = uVar6;
  if (local_10 < fVar1) {
    fVar1 = local_10;
  }
  local_10 = fVar1;
  puVar7 = (undefined4 *)*in_ECX;
  if ((undefined4 *)in_ECX[1] <= puVar7) {
    puVar7 = (undefined4 *)FUN_00252040();
  }
  *puVar7 = 0x40208;
  puVar7[1] = uVar9;
  puVar8 = puVar7 + 2;
  if ((float)in_ECX[0x259] != local_10) {
    in_ECX[0x259] = local_10;
    iVar4 = FUN_002529b0(local_10 + local_10 + 0.5);
    in_ECX[0x25a] = *(undefined4 *)(&DAT_0025edcc + iVar4 * 4);
    DAT_00260d98 = DAT_00260d98 | 0x10f;
    uVar6 = FUN_002529b0(DAT_00261214 * local_10 * 8.0 + 0.5);
    if (0x1ff < uVar6) {
      uVar6 = 0x1ff;
    }
    *puVar8 = 0x40380;
    puVar7[3] = uVar6;
    puVar8 = puVar7 + 4;
  }
  *in_ECX = puVar8;
  uVar6 = DAT_00261204 << 0x10 | DAT_00261218;
  if (((char)((uint)in_ECX[2] >> 8) < '\0') && (DAT_00261200 != 0)) {
    uVar6 = uVar6 | 1;
  }
  if ((undefined4 *)in_ECX[1] <= puVar8) {
    puVar8 = (undefined4 *)FUN_00252040();
  }
  *puVar8 = 0x41d7c;
  puVar8[1] = uVar6;
  *in_ECX = puVar8 + 2;
  if ((*(byte *)(in_ECX[0x1e5] + 4) & 2) != 0) {
    FUN_00254ff0();
  }
  FUN_002535d0(in_EDX);
  return;
}

