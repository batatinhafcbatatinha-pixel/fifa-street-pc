// callers: 0x2550e0

void FUN_00254d70(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  puVar2 = DAT_00261238;
  if ((*(byte *)(DAT_00261238 + 2) & 0x10) == 0) {
    FUN_002545b0(param_1 + -1,param_2);
  }
  iVar4 = *(int *)(param_1 + 0xb);
  iVar1 = iVar4 + 2;
  puVar3 = (undefined4 *)FUN_00252050(iVar1);
  *puVar3 = 0x41e9c;
  puVar3[1] = param_2;
  puVar5 = (undefined4 *)(param_1 + 0x113);
  puVar6 = puVar3 + 2;
  for (; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar6 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar6 = puVar6 + 1;
  }
  *puVar2 = puVar3 + iVar1;
  return;
}

