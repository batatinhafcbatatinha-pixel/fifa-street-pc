// callers: 

void FUN_00259b40(undefined4 param_1,int *param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  iVar1 = *param_2;
  do {
    uVar4 = *(uint *)(iVar1 + 0x100);
    uVar3 = 0;
    if ((uVar4 & 0x1000) != 0) {
      uVar3 = FUN_00259900();
    }
    if ((uVar4 & 0xffffefff) != 0) {
      if ((uVar4 & 0x100000) != 0) {
        uVar2 = FUN_002594a0();
        uVar3 = uVar3 | uVar2;
      }
      if ((uVar4 & 0x1000000) != 0) {
        if (param_2[0x208] == 0 && param_2[0x209] == 0) {
          uVar2 = FUN_00259390();
          uVar3 = uVar3 | uVar2;
        }
        else {
          do {
            *(undefined4 *)(iVar1 + 0x600100) = 1;
            uVar4 = *(uint *)(iVar1 + 0x100);
          } while ((uVar4 & 0x1000000) != 0);
        }
      }
      if ((uVar4 & 0x100) != 0) {
        uVar2 = FUN_002599b0();
        uVar3 = uVar3 | uVar2;
      }
      if (((uVar4 & 0x10000) != 0) && ((*(byte *)(*param_2 + 0x8100) & 1) != 0)) {
        *(undefined4 *)(*param_2 + 0x8100) = 1;
      }
    }
  } while (uVar3 != 0);
  param_2[0x29] = 0;
  *(int *)(iVar1 + 0x140) = param_2[0x2d];
  return;
}

