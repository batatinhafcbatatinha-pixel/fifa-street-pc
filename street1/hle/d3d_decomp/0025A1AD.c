// callers: 0x2540a0 0x25d210

void FUN_0025a1ad(void)

{
  uint *puVar1;
  char cVar2;
  int iVar3;
  int *in_ECX;
  undefined4 uVar4;
  
  iVar3 = *in_ECX;
  if ((DAT_80000143 & 2) == 0) {
    cVar2 = *(char *)((int)in_ECX + 10);
    if (cVar2 == '\x0f') {
      uVar4 = 0xdaebc;
    }
    else if (cVar2 == '\x10') {
      uVar4 = 0xe2194;
    }
    else {
      if (cVar2 != '\x12') {
        return;
      }
      uVar4 = 0x108e48;
    }
    *(undefined4 *)(iVar3 + 0x680680) = uVar4;
    puVar1 = (uint *)(iVar3 + 0x6806a0);
    *puVar1 = ~*puVar1 & 1;
  }
  return;
}

