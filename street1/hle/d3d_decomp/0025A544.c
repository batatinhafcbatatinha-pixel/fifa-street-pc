// callers: 0x253c30 0x25a606

undefined4 FUN_0025a544(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  int *in_ECX;
  undefined4 local_10;
  int *local_c;
  
  iVar1 = *in_ECX;
  FUN_0025a041(&local_10);
  piVar2 = (int *)((param_1 + 0x10024) * 0x10 + iVar1);
  local_c = (int *)((param_1 + 0x40090) * 0x10 + iVar1);
  do {
    FUN_0025b717();
    *piVar2 = 0;
    *local_c = 0;
    *(uint *)(iVar1 + 0x400750) = param_1 * 4 + 0x10U & 0x1ffc | 0xea0000;
    *(undefined4 *)(iVar1 + 0x400754) = 0;
  } while (*piVar2 != *local_c);
  *(undefined4 *)(iVar1 + 0x100300 + param_1 * 4) = 0;
  *(undefined4 *)(iVar1 + 0x400980 + param_1 * 4) = 0;
  *(uint *)(iVar1 + 0x400750) = param_1 * 4 + 0x90U & 0x1ffc | 0xea0000;
  *(undefined4 *)(iVar1 + 0x400754) = 0;
  if (param_2 != 0) {
    *(undefined4 *)(iVar1 + 0x100324) = 0;
    *(undefined4 *)(iVar1 + 0x4009a0) = 0;
  }
  *(undefined4 *)(iVar1 + 0x3220) = local_10;
  return 1;
}

