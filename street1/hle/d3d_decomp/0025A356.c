// callers: 0x253c30

undefined4
FUN_0025a356(uint param_1,uint param_2,int param_3,undefined4 param_4,uint param_5,uint param_6,
            uint param_7)

{
  int iVar1;
  uint *puVar2;
  uint *puVar3;
  uint uVar4;
  int *in_ECX;
  undefined4 local_24;
  uint local_20;
  uint local_1c;
  uint local_18;
  int local_14;
  uint local_10;
  int local_8;
  
  iVar1 = *in_ECX;
  FUN_0025a041(&local_24);
  local_18 = param_5 & 1;
  local_10 = param_1 * 4 + 0x10 & 0x1ffc | 0xea0000;
  local_1c = param_1 * 4 + 0x30 & 0x1ffc | 0xea0000;
  local_14 = (param_2 - 1) + param_3;
  puVar2 = (uint *)((param_1 + 0x10024) * 0x10 + iVar1);
  puVar3 = (uint *)((param_1 + 0x40090) * 0x10 + iVar1);
  local_20 = param_1 * 4 + 0x50 & 0x1ffc | 0xea0000;
  local_8 = param_1 * 0x10 + iVar1;
  do {
    FUN_0025b717();
    *(int *)(local_8 + 0x100244) = local_14;
    *(undefined4 *)(local_8 + 0x100248) = param_4;
    *(int *)(local_8 + 0x400904) = local_14;
    *(undefined4 *)(local_8 + 0x400908) = param_4;
    if (local_18 == 0) {
      uVar4 = param_2 | 1;
    }
    else {
      uVar4 = param_2 | 3;
    }
    *puVar3 = uVar4;
    *puVar2 = uVar4;
    *(uint *)(iVar1 + 0x400750) = local_10;
    *(uint *)(iVar1 + 0x400754) = uVar4;
    *(uint *)(iVar1 + 0x400750) = local_1c;
    *(int *)(iVar1 + 0x400754) = local_14;
    *(uint *)(iVar1 + 0x400750) = local_20;
    *(undefined4 *)(iVar1 + 0x400754) = param_4;
  } while (((*puVar2 != *puVar3) || (*(int *)(local_8 + 0x100248) != *(int *)(local_8 + 0x400908)))
          || (((*(uint *)(local_8 + 0x400904) ^ *(uint *)(local_8 + 0x100244)) & 0xffffc000) != 0));
  if ((param_5 & 0x80000000) != 0) {
    param_2 = param_6 >> 2 | 0x80000000;
    if ((param_5 & 0x4000000) != 0) {
      param_2 = param_6 >> 2 | 0x84000000;
    }
    puVar2 = (uint *)(iVar1 + 0x100300 + param_1 * 4);
    do {
      FUN_0025b717();
      *puVar2 = param_2;
      *(uint *)(iVar1 + 0x400980 + param_1 * 4) = param_2;
      *(uint *)(iVar1 + 0x400750) = param_1 * 4 + 0x90 & 0x1ffc | 0xea0000;
      *(uint *)(iVar1 + 0x400754) = param_2;
    } while (*puVar2 != *(uint *)(iVar1 + 0x400980 + param_1 * 4));
    if (param_7 != 0) {
      uVar4 = param_1 | param_7 | 0x80000000;
      do {
        FUN_0025b717();
        *(uint *)(iVar1 + 0x100324) = uVar4;
        *(uint *)(iVar1 + 0x4009a0) = uVar4;
      } while (*(uint *)(iVar1 + 0x100324) != *(uint *)(iVar1 + 0x4009a0));
    }
  }
  *(undefined4 *)(iVar1 + 0x3220) = local_24;
  return 1;
}

