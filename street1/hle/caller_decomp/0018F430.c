
void FUN_0018f430(undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *in_ECX;
  
  if (DAT_002d50f0 < in_ECX[0x29] + 1) {
    FUN_0018eee0();
  }
  if (in_ECX[0x29] == 0) {
    puVar2 = (undefined4 *)FUN_00251b80(DAT_002d5118);
    iVar1 = DAT_002d50f4;
    in_ECX[0x27] = puVar2;
    *puVar2 = 0x417fc;
    *(undefined4 *)(in_ECX[0x27] + 4) = *in_ECX;
    *(int *)(in_ECX[0x27] + 8) = iVar1 * 0x40000 + 0x40001818;
    in_ECX[0x28] = in_ECX[0x27] + 8;
    in_ECX[0x27] = in_ECX[0x27] + 0xc;
  }
  puVar2 = (undefined4 *)in_ECX[0x27];
  *puVar2 = *param_1;
  puVar2[1] = param_1[1];
  puVar2[2] = param_1[2];
  puVar2[3] = param_1[3];
  iVar1 = in_ECX[0x27];
  in_ECX[0x27] = (undefined4 *)(iVar1 + 0x10);
  *(undefined4 *)(iVar1 + 0x10) = *param_2;
  in_ECX[0x27] = in_ECX[0x27] + 4;
  in_ECX[0x29] = in_ECX[0x29] + 1;
  return;
}

