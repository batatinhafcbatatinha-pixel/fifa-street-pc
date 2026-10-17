// callers: 0x259b40 0x25a041 0x25a606

uint FUN_002599b0(void)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *in_ECX;
  int iVar4;
  
  iVar1 = *in_ECX;
  uVar2 = *(uint *)(iVar1 + 0x2100);
  if ((uVar2 & 0x100000) != 0) {
    *(undefined4 *)(iVar1 + 0x2100) = 0x100000;
  }
  if ((uVar2 & 0x1000000) != 0) {
    *(undefined4 *)(iVar1 + 0x2100) = 0x1000000;
  }
  uVar2 = *(uint *)(iVar1 + 0x2100);
  if ((uVar2 & 1) != 0) {
    uVar3 = *(uint *)(iVar1 + 0x3250);
    *(undefined4 *)(iVar1 + 0x2500) = 0;
    *(undefined4 *)(iVar1 + 0x3250) = 0;
    *(undefined4 *)(iVar1 + 0x2100) = 1;
    iVar4 = 0xffff;
    do {
      if ((uVar3 & 0x1000) == 0) break;
      iVar4 = iVar4 + -1;
      uVar3 = *(uint *)(iVar1 + 0x3250);
    } while (iVar4 != 0);
    *(undefined4 *)(iVar1 + 0x3258) = 0;
    *(undefined4 *)(iVar1 + 0x3250) = 1;
    *(undefined4 *)(iVar1 + 0x2500) = 1;
  }
  if ((uVar2 & 0x1000) != 0) {
                    /* WARNING: Subroutine does not return */
    FUN_0025af90(s_GPU_generated_an_error__Try_repr_0025fde0);
  }
  if ((*(uint *)(iVar1 + 0x3220) & 0x1000) != 0) {
    if ((*(uint *)(iVar1 + 0x3214) & 0x10) == 0) {
      do {
        if (*(int *)(iVar1 + 0x2100) != 0) break;
        if (*(int *)(iVar1 + 0x400100) != 0) {
          FUN_00259900();
        }
        if ((*(uint *)(iVar1 + 0x100) & 0x1000000) != 0) {
          FUN_00259390();
        }
      } while ((*(byte *)(iVar1 + 0x3214) & 0x10) == 0);
    }
    if ((*(byte *)(iVar1 + 0x3214) & 0x10) != 0) {
      do {
      } while ((*(byte *)(iVar1 + 0x2500) & 0x10) != 0);
      *(uint *)(iVar1 + 0x3220) = *(uint *)(iVar1 + 0x3220) & 0xffffefff;
    }
  }
  if (*(int *)(iVar1 + 0x2100) == 0) {
    *(undefined4 *)(iVar1 + 0x3250) = 1;
    *(undefined4 *)(iVar1 + 0x2500) = 1;
  }
  return *(uint *)(iVar1 + 0x2080) & 1 | *(uint *)(iVar1 + 0x2100);
}

