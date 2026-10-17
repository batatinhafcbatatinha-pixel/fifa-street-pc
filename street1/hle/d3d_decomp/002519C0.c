// callers: 0x251f00

uint FUN_002519c0(void)

{
  uint uVar1;
  uint uVar2;
  uint *in_ECX;
  uint uVar3;
  
  uVar1 = *(uint *)(in_ECX[0x708] + 0x44) | 0x80000000;
  if ((in_ECX[9] <= uVar1) && (uVar1 < in_ECX[10])) {
    in_ECX[0x18] = in_ECX[0x10] - (uint)(*in_ECX < uVar1);
    in_ECX[0x17] = uVar1;
    return uVar1;
  }
  uVar1 = in_ECX[0x10];
  uVar3 = uVar1 - *(uint *)(in_ECX[0x24d] + 0x400b10) & 3;
  uVar2 = *(uint *)(in_ECX[0x24d] + 0x400b10) >> 5 & 0x7fffffc | 0x80000000;
  if ((uVar3 <= uVar1 - in_ECX[0x18]) &&
     ((uVar1 - in_ECX[0x18] != uVar3 || (in_ECX[0x17] <= uVar2)))) {
    in_ECX[0x18] = uVar1 - uVar3;
    in_ECX[0x17] = uVar2;
    return uVar2;
  }
  return in_ECX[0x17];
}

