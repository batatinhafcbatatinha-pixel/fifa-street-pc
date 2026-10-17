// callers: 0x255bf0

undefined4
FUN_00259ee2(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,
            undefined4 *param_5)

{
  int iVar1;
  int iVar2;
  int *in_ECX;
  int iVar3;
  int extraout_EDX;
  uint uVar4;
  uint local_c;
  int local_8;
  
  local_c = 0;
  local_8 = 0;
  iVar1 = *in_ECX;
  FUN_00259e88(param_3,&local_c,&local_8,0);
  iVar2 = *(int *)(extraout_EDX + 0x160);
  *(int *)(extraout_EDX + 0x160) = iVar2 + 1;
  uVar4 = 2;
  if (param_2 != 2) {
    if (param_2 == 3) {
      uVar4 = 3;
    }
    else {
      uVar4 = 0x3d;
    }
  }
  uVar4 = uVar4 | local_c << 0x14 | 0x3000;
  if (local_8 != 2) {
    if (local_8 == 3) {
      uVar4 = uVar4 | 0x30000;
    }
    else if (local_8 == 1) {
      uVar4 = uVar4 | 0x20000;
    }
  }
  iVar3 = iVar2 * 0x10 + iVar1;
  *(uint *)(iVar3 + 0x700008) = local_c | 3;
  *(uint *)(iVar3 + 0x70000c) = local_c | 3;
  *(uint *)((iVar2 + 0x70000) * 0x10 + iVar1) = uVar4 | 0x8000;
  *(undefined4 *)(iVar3 + 0x700004) = param_4;
  *param_5 = 0;
  param_5[1] = 0;
  param_5[2] = 0;
  param_5[3] = 0;
  *(undefined2 *)((int)param_5 + 6) = 0;
  *param_5 = param_1;
  param_5[2] = param_2;
  param_5[3] = iVar2;
  return 1;
}

