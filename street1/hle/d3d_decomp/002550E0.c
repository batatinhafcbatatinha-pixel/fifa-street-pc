// callers: 0x19f310 0x1a06b0 0x255bf0

void FUN_002550e0(uint param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined *puVar6;
  
  puVar3 = DAT_00261238;
  uVar1 = *(uint *)(DAT_00261238[0x1e5] + 4);
  iVar2 = *(int *)(DAT_00261238[0x1e5] + 0x10);
  if ((param_1 & 1) == 0) {
    puVar6 = &DAT_002638d0;
    FUN_00254470(&DAT_002638d0,param_1);
  }
  else {
    puVar6 = (undefined *)(param_1 - 1);
    if ((*(byte *)(param_1 + 3) & 0x10) != 0) {
      FUN_00254d70(param_1,0);
      FUN_00254dd0(param_1,0);
      return;
    }
  }
  uVar4 = DAT_00260d98 | 0x70;
  if (iVar2 != *(int *)(puVar6 + 0x10)) {
    uVar4 = DAT_00260d98 | 0x470;
  }
  DAT_00260d98 = uVar4;
  if (uVar1 != *(uint *)(puVar6 + 4)) {
    DAT_00260d98 = DAT_00260d98 | 0x1600;
  }
  puVar5 = (undefined4 *)*puVar3;
  puVar3[0x1e5] = puVar6;
  puVar3[0x1e6] = param_1;
  puVar3[0x1e7] = 0;
  if ((undefined4 *)puVar3[1] <= puVar5) {
    puVar5 = (undefined4 *)FUN_00252040();
  }
  if ((*(uint *)(puVar6 + 4) & 0x400) == 0) {
    *puVar5 = 0x4194c;
    puVar5[1] = 0xffffffff;
    puVar5 = puVar5 + 2;
  }
  if ((*(uint *)(puVar6 + 4) & 0x800) == 0) {
    *puVar5 = 0x41950;
    puVar5[1] = 0;
    puVar5 = puVar5 + 2;
  }
  if ((*(uint *)(puVar6 + 4) & 0x1000) == 0) {
    *puVar5 = 0x4195c;
    puVar5[1] = 0xffffffff;
    puVar5 = puVar5 + 2;
  }
  if ((*(uint *)(puVar6 + 4) & 0x2000) == 0) {
    *puVar5 = 0x41960;
    puVar5[1] = 0;
    puVar5 = puVar5 + 2;
  }
  if ((*(uint *)(puVar6 + 4) & 2) != 0) {
    puVar5 = (undefined4 *)FUN_00250c20();
    *puVar5 = 0x41ea0;
    puVar5[1] = 0;
    puVar5[2] = 0x81e94;
    puVar5[3] = 6;
    puVar5[4] = 0;
    *puVar3 = puVar5 + 5;
    FUN_00254ff0();
    return;
  }
  if (((*(uint *)(puVar6 + 4) ^ uVar1) & 0x12) != 0) {
    *puVar5 = 0x41e94;
    puVar5[1] = 4;
    puVar5 = (undefined4 *)FUN_00250c20();
  }
  *puVar3 = puVar5;
  return;
}

