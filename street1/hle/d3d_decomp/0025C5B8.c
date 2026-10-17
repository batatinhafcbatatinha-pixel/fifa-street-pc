// callers: 0x259900 0x25a606 0x25c75c

void FUN_0025c5b8(uint param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int *in_ECX;
  uint uVar4;
  
  iVar1 = *in_ECX;
  if (*(int *)(iVar1 + 0x400100) != 0) {
    FUN_00259900();
  }
  uVar4 = *(uint *)(iVar1 + 0x400720);
  *(undefined4 *)(iVar1 + 0x400720) = 0;
  FUN_0025b717();
  uVar2 = in_ECX[0x4d];
  if (uVar2 != param_1) {
    FUN_0025b756(uVar2);
  }
  in_ECX[0x4d] = param_1;
  if (param_1 == 2) {
    *(undefined4 *)(iVar1 + 0x400144) = 0x10000100;
    *(undefined4 *)(iVar1 + 0x400764) = 0x8000000;
    *(uint *)(iVar1 + 0x400720) = uVar4 | 1;
  }
  else {
    if (in_ECX[param_1 + 0x51] != 0) {
      *(undefined4 *)(iVar1 + 0x400080) = 0x70000;
      *(undefined4 *)(iVar1 + 0x400080) = 0;
      *(undefined4 *)(iVar1 + 0x400750) = 0x3d0000;
      iVar3 = 0xf;
      do {
        iVar3 = iVar3 + -1;
        *(undefined4 *)(iVar1 + 0x400754) = 0;
      } while (iVar3 != 0);
    }
    *(uint *)(iVar1 + 0x400084) = *(uint *)(iVar1 + 0x400084) | 0x1000000;
    uVar4 = (param_1 & 0x1f) << 0x18;
    *(uint *)(iVar1 + 0x400148) = uVar4;
    *(uint *)(iVar1 + 0x400784) = in_ECX[param_1 + 0x4e] & 0xffff;
    *(undefined4 *)(iVar1 + 0x400788) = 1;
    FUN_0025b717();
    *(uint *)(iVar1 + 0x400148) = *(uint *)(iVar1 + 0x400148) & 0xe0ffffff | uVar4;
    *(undefined4 *)(iVar1 + 0x400144) = 0x10010100;
    *(uint *)(iVar1 + 0x400764) = *(uint *)(iVar1 + 0x400764) & 0xcfffffff;
  }
  return;
}

