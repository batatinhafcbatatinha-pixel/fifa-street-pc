// callers: 0x252f00 0x2540a0 0x2560c0

undefined4 FUN_00259160(void)

{
  int in_ECX;
  
  return *(undefined4 *)(in_ECX + ((*(uint *)(in_ECX + 0x1bc) & 1) * 3 + 0x5d) * 4);
}

