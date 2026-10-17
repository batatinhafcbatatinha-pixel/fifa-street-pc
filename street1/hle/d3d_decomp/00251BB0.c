// callers: 0x251cd0 0x251f00 0x255bf0 0x2560c0

void FUN_00251bb0(void)

{
  int iVar1;
  uint *in_ECX;
  uint uVar2;
  
  if ((in_ECX[2] & 4) == 0) {
    uVar2 = *in_ECX;
  }
  else {
    uVar2 = in_ECX[0x1dc];
  }
  if ((in_ECX[2] & 0x2000) == 0) {
    iVar1 = *(int *)(DAT_00261238 + 0x1c28);
    *(uint *)(iVar1 + 0x100410) = *(uint *)(iVar1 + 0x100410) | 0x10000;
    do {
    } while ((*(uint *)(iVar1 + 0x100410) & 0x10000) != 0);
    *(uint *)(in_ECX[0x708] + 0x40) = uVar2 & 0xfffffff;
    if (DAT_00260d90 != 0) {
      do {
        iVar1 = FUN_00253ad0();
      } while (iVar1 != 0);
      in_ECX[2] = in_ECX[2] | 0x2000;
      return;
    }
  }
  else {
    in_ECX[0x708] = (uint)&DAT_00260d40;
    DAT_00260d80 = *in_ECX & 0xfffffff;
    *(uint *)(in_ECX[0x708] + 0x44) = DAT_00260d80;
    *(uint *)in_ECX[0xc] = in_ECX[0xb] - 2;
    in_ECX[0x779] = in_ECX[0x91e];
  }
  return;
}

