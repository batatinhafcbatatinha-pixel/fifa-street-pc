// callers: 

void FUN_00250570(float param_1)

{
  undefined4 *puVar1;
  uint uVar2;
  undefined4 *puVar3;
  
  puVar1 = DAT_00261238;
  uVar2 = FUN_002529b0(param_1 * (float)DAT_00261238[0x259] * 8.0 + 0.5);
  if (0x1ff < uVar2) {
    uVar2 = 0x1ff;
  }
  puVar3 = (undefined4 *)*puVar1;
  if ((undefined4 *)puVar1[1] <= puVar3) {
    puVar3 = (undefined4 *)FUN_00252040();
  }
  *puVar3 = 0x40380;
  puVar3[1] = uVar2;
  *puVar1 = puVar3 + 2;
  DAT_00261214 = param_1;
  return;
}

