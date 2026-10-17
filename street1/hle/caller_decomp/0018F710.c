
void FUN_0018f710(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *in_ECX;
  
  if (DAT_002d50f0 < in_ECX[0x2a] + 1) {
    FUN_0018f190();
  }
  if (in_ECX[0x2a] == 0) {
    puVar2 = (undefined4 *)FUN_00251b80(DAT_002d511c);
    iVar1 = DAT_002d50f8;
    in_ECX[0x28] = puVar2;
    *puVar2 = 0x417fc;
    *(undefined4 *)(in_ECX[0x28] + 4) = *in_ECX;
    *(int *)(in_ECX[0x28] + 8) = iVar1 * 0x40000 + 0x40001818;
    in_ECX[0x29] = in_ECX[0x28] + 8;
    in_ECX[0x28] = in_ECX[0x28] + 0xc;
  }
  puVar2 = (undefined4 *)in_ECX[0x28];
  *puVar2 = *param_1;
  puVar2[1] = param_1[1];
  puVar2[2] = param_1[2];
  iVar1 = in_ECX[0x28];
  in_ECX[0x28] = (undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)(iVar1 + 0xc) = *param_2;
  iVar1 = in_ECX[0x28];
  puVar2 = (undefined4 *)(iVar1 + 4);
  in_ECX[0x28] = puVar2;
  *puVar2 = *param_3;
  *(undefined4 *)(iVar1 + 8) = param_3[1];
  in_ECX[0x28] = in_ECX[0x28] + 8;
  in_ECX[0x2a] = in_ECX[0x2a] + 1;
  return;
}

