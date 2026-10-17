// callers: 0x2512a0 0x251610 0x2535d0 0x2541d0 0x254dd0 0x2550e0 0x25d450

undefined4 * FUN_00250c20(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  float fVar3;
  float fVar4;
  int in_ECX;
  undefined4 *in_EDX;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if ((*(byte *)(*(int *)(in_ECX + 0x794) + 4) & 0x12) == 0) {
    fVar3 = (float)*(int *)(in_ECX + 0xee0);
    if (*(int *)(in_ECX + 0xee0) < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    fVar3 = fVar3 * *(float *)(in_ECX + 0x95c) + *(float *)(in_ECX + 0xef8);
    fVar4 = (float)*(int *)(in_ECX + 0xee4);
    if (*(int *)(in_ECX + 0xee4) < 0) {
      fVar4 = fVar4 + 4.2949673e+09;
    }
    fVar4 = fVar4 * *(float *)(in_ECX + 0x960) + *(float *)(in_ECX + 0xefc);
    if (((char)((uint)*(undefined4 *)(in_ECX + 8) >> 8) < '\0') && (DAT_00261200 != 0)) {
      fVar3 = fVar3 - 0.5;
      fVar4 = fVar4 - 0.5;
    }
    *in_EDX = FUN_00100a20;
    in_EDX[1] = fVar3;
    in_EDX[3] = 0;
    in_EDX[4] = 0;
    in_EDX[2] = fVar4;
    in_EDX = in_EDX + 5;
    if ((*(byte *)(in_ECX + 0x1c18) & 1) == 0) {
      if (DAT_002611dc == 2) {
        *(undefined4 *)(in_ECX + 0x1c0c) = *(undefined4 *)(in_ECX + 0x948);
        *(float *)(in_ECX + 0x1c08) =
             *(float *)(in_ECX + 0x944) * *(float *)(in_ECX + 0x93c) * *(float *)(in_ECX + 0x948);
      }
      else {
        *(float *)(in_ECX + 0x1c08) = *(float *)(in_ECX + 0xef0) * *(float *)(in_ECX + 0x948);
        *(float *)(in_ECX + 0x1c0c) = *(float *)(in_ECX + 0xef4) * *(float *)(in_ECX + 0x948);
      }
    }
    uVar1 = *(undefined4 *)(in_ECX + 0x1c0c);
    uVar2 = *(undefined4 *)(in_ECX + 0x1c08);
  }
  else {
    FUN_002500e0(&local_20,&local_10);
    if ((*(uint *)(in_ECX + 8) & 0x200) == 0) {
      in_EDX[1] = local_20;
      in_EDX[2] = local_1c;
      in_EDX[3] = local_18;
      in_EDX[4] = local_14;
      *in_EDX = FUN_00100a20;
      in_EDX[6] = local_10;
      in_EDX[5] = 0x100af0;
      in_EDX[7] = local_c;
      in_EDX[8] = local_8;
      in_EDX[9] = local_4;
      in_EDX = in_EDX + 10;
    }
    if ((*(byte *)(in_ECX + 0x1c18) & 2) == 0) {
      if (DAT_002611dc == 2) {
        *(undefined4 *)(in_ECX + 0x1c14) = *(undefined4 *)(in_ECX + 0x948);
        *(float *)(in_ECX + 0x1c10) =
             *(float *)(in_ECX + 0x944) * *(float *)(in_ECX + 0x93c) * *(float *)(in_ECX + 0x948);
        uVar1 = *(undefined4 *)(in_ECX + 0x1c14);
        uVar2 = *(undefined4 *)(in_ECX + 0x1c10);
        goto LAB_00250df0;
      }
      *(undefined4 *)(in_ECX + 0x1c10) = 0;
      *(undefined4 *)(in_ECX + 0x1c14) = *(undefined4 *)(in_ECX + 0x948);
    }
    uVar1 = *(undefined4 *)(in_ECX + 0x1c14);
    uVar2 = *(undefined4 *)(in_ECX + 0x1c10);
  }
LAB_00250df0:
  in_EDX[1] = uVar2;
  *in_EDX = &LAB_00080394;
  in_EDX[2] = uVar1;
  return in_EDX + 3;
}

