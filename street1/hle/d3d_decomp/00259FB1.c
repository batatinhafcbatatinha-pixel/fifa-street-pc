// callers: 0x255bf0

undefined4 FUN_00259fb1(undefined4 param_1,uint param_2,undefined4 *param_3)

{
  int iVar1;
  bool bVar2;
  uint uVar3;
  int in_ECX;
  
  bVar2 = false;
  if ((((param_2 < 0x58) || (param_2 == 0x62)) || (param_2 == 0x72)) || (param_2 != 0x97)) {
    uVar3 = 0x10;
  }
  else {
    uVar3 = 0x330;
    bVar2 = true;
  }
  iVar1 = *(int *)(in_ECX + 0x160);
  *(int *)(in_ECX + 0x160) = (uVar3 >> 4) + iVar1;
  if (bVar2) {
    *(int *)(in_ECX + 0x144 + *(int *)(in_ECX + 0x100) * 4) = iVar1;
    FUN_0025bbd9();
  }
  FUN_0025bb64(iVar1,param_2);
  *param_3 = 0;
  param_3[1] = 0;
  param_3[2] = 0;
  param_3[3] = 0;
  *param_3 = param_1;
  param_3[2] = param_2;
  param_3[3] = iVar1;
  *(undefined2 *)((int)param_3 + 6) = 1;
  FUN_00259ccd(param_3);
  return 1;
}

