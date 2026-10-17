// callers: 0x259d6d

void FUN_0025b473(void)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int *in_ECX;
  int *local_8;
  
  iVar4 = *in_ECX;
  in_ECX[0x53] = *(int *)(iVar4 + 0x100200);
  in_ECX[0x54] = *(int *)(iVar4 + 0x100204);
  in_ECX[0x2c] = ((*(uint *)(iVar4 + 0x1218) & 0x100) != 0x100) + 2;
  local_8 = in_ECX;
  iVar1 = (*(code *)0x800000a8)(0x5000,&local_8);
  in_ECX[4] = iVar1 + -0x5000;
  in_ECX[0x4c] = (int)(local_8 + 0x1c0000);
  *(uint *)(iVar4 + 0x2210) = (uint)local_8 >> 8 & 0x1f0 | 0x3000000;
  in_ECX[0x4a] = (int)(local_8 + 0x1c0400);
  in_ECX[0x4b] = (int)(local_8 + 0x1c0420);
  *(uint *)(iVar4 + 0x2214) =
       ((uint)(local_8 + 0x1c0420) & 0x1fc00 | 0x200) << 7 | (uint)in_ECX[0x4a] >> 8 & 0x1fc;
  in_ECX[0x58] = (uint)(local_8 + 0x428) >> 4;
  *(uint *)(iVar4 + 0x100214) = *(uint *)(iVar4 + 0x100214) & 0xfffffffe;
  uVar2 = 0x700000;
  do {
    uVar3 = uVar2 + 4;
    *(undefined4 *)((int)local_8 + iVar4 + uVar2) = 0;
    uVar2 = uVar3;
  } while (uVar3 < 0x705000);
  iVar4 = FUN_00259c17(8);
  in_ECX[0x50] = iVar4;
  return;
}

