// callers: 0x2550e0

void FUN_00254dd0(uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  uint *puVar3;
  undefined4 *puVar4;
  
  puVar3 = DAT_00261238;
  if (param_1 != 0) {
    uVar1 = *(uint *)(DAT_00261238[0x1e5] + 4);
    DAT_00261238[0x1e5] = param_1 - 1;
    puVar3[0x1e6] = param_1;
    uVar2 = DAT_00260d98;
    DAT_00260d98 = DAT_00260d98 | 0x70;
    if (((*(uint *)(param_1 + 3) ^ uVar1) & 0x13) != 0) {
      DAT_00260d98 = uVar2 | 0x1070;
      if (puVar3[1] <= *puVar3) {
        FUN_00252040();
      }
      puVar4 = (undefined4 *)FUN_00250c20();
      uVar1 = *(uint *)(param_1 + 3);
      *puVar4 = 0x81e94;
      puVar4[1] = 6;
      puVar4[2] = uVar1 & 1;
      *puVar3 = (uint)(puVar4 + 3);
    }
  }
  puVar4 = (undefined4 *)*puVar3;
  if ((undefined4 *)puVar3[1] <= puVar4) {
    puVar4 = (undefined4 *)FUN_00252040();
  }
  *puVar4 = 0x41ea0;
  puVar4[1] = param_2;
  *puVar3 = (uint)(puVar4 + 2);
  puVar3[0x1e7] = param_2;
  return;
}

