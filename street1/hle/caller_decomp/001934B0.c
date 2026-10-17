
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_001934b0(void)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *in_ECX;
  undefined4 **ppuVar6;
  undefined4 uVar7;
  undefined4 *local_4;
  
  local_4 = in_ECX;
  if (*(char *)(in_ECX + 9) == '\0') {
    if (DAT_002f3450 != 0) {
      FUN_00253800(0,0);
    }
    if (DAT_002f3454 != 0) {
      FUN_00253800(1,0);
    }
    if (DAT_002f3458 != 0) {
      FUN_00253800(2,0);
    }
    if (DAT_002f345c != 0) {
      FUN_00253800(3,0);
    }
    DAT_002f345c = 0;
    DAT_002f3458 = 0;
    DAT_002f3454 = 0;
    DAT_002f3450 = 0;
    if (DAT_002f34b0 != '\0') {
      DAT_002f34b0 = '\0';
      FUN_00251440(0);
    }
  }
  DAT_002d5190 = *(undefined1 *)(in_ECX + 9);
  if (DAT_002d519c != in_ECX[0xb]) {
    DAT_002d519c = in_ECX[0xb];
    FUN_00250740(in_ECX[0xb]);
  }
  if (DAT_002d51a0 != in_ECX[0xc]) {
    uVar7 = in_ECX[0xc];
    DAT_002d51a0 = in_ECX[0xc];
    FUN_00250050();
    _DAT_002610c8 = uVar7;
  }
  if (DAT_002f3440 != (float)in_ECX[0xf]) {
    DAT_002f3440 = (float)in_ECX[0xf];
    uVar7 = in_ECX[0xf];
    FUN_00250050();
    _DAT_002610d4 = uVar7;
  }
  if (DAT_002f3444 != (float)in_ECX[0x10]) {
    DAT_002f3444 = (float)in_ECX[0x10];
    uVar7 = in_ECX[0x10];
    FUN_00250050();
    _DAT_002610d8 = uVar7;
    if ((float)in_ECX[0x10] == 0.0) {
      FUN_00250050();
      _DAT_002610dc = 0;
      FUN_00250050();
      _DAT_002610e0 = 0;
      FUN_00250050();
      _DAT_002610e4 = 0;
    }
    else {
      FUN_00250050();
      _DAT_002610dc = 1;
      FUN_00250050();
      _DAT_002610e0 = 1;
      FUN_00250050();
      _DAT_002610e4 = 1;
    }
  }
  uVar2 = in_ECX[0x12];
  if (DAT_002d5fc4 != uVar2) {
    if (uVar2 == 0xffffffff) {
      ppuVar6 = &local_4;
      FUN_00197be0(ppuVar6);
      FUN_00197b50();
      FUN_00199000(ppuVar6);
      FUN_00197be0(local_4);
      FUN_00197b50();
      FUN_00198fc0(local_4);
    }
    else {
      DAT_002d5fc4 = (byte)uVar2 & 1;
      uVar7 = in_ECX[0x12];
      _DAT_002d51b0 = uVar2;
      FUN_00250050();
      _DAT_002610a0 = uVar7;
    }
  }
  if (DAT_002d51ac != in_ECX[0x11]) {
    uVar7 = in_ECX[0x11];
    DAT_002d51ac = in_ECX[0x11];
    FUN_00250050();
    _DAT_002610cc = uVar7;
  }
  if ((DAT_002d5170 != *(byte *)(in_ECX + 2)) || (DAT_002d5174 != in_ECX[3])) {
    DAT_002d5170 = (uint)*(byte *)(in_ECX + 2);
    DAT_002d5174 = in_ECX[3];
    if (*(char *)(in_ECX + 2) == '\0') {
      uVar7 = 0;
    }
    else {
      uVar7 = in_ECX[3];
    }
    FUN_00250420(uVar7);
  }
  iVar3 = in_ECX[1];
  if (DAT_002d516c == iVar3) goto LAB_001937be;
  iVar4 = in_ECX[1];
  DAT_002d516c = iVar3;
  if (iVar4 == 0) {
    FUN_00250050();
    _DAT_002610a8 = 0x1d00;
LAB_001937ae:
    DAT_0026113c = 0;
  }
  else {
    if (iVar4 == 1) {
      FUN_00250050();
      _DAT_002610a8 = 0x1d01;
      goto LAB_001937ae;
    }
    if (iVar4 != 2) goto LAB_001937be;
    FUN_00250050();
    _DAT_002610a8 = 0x1d01;
    DAT_0026113c = 1;
  }
  DAT_00260d98 = DAT_00260d98 | 0x3000;
LAB_001937be:
  if (DAT_002d517c != in_ECX[4]) {
    uVar7 = in_ECX[4];
    DAT_002d517c = in_ECX[4];
    FUN_00250050();
    _DAT_00261084 = uVar7;
  }
  if (((DAT_002d5180 != in_ECX[5]) || (DAT_002d51a4 != in_ECX[0xd])) ||
     (DAT_002d51a8 != in_ECX[0xe])) {
    DAT_002d51a4 = in_ECX[0xd];
    DAT_002d51a8 = in_ECX[0xe];
    uVar7 = in_ECX[0xd];
    DAT_002d5180 = in_ECX[5];
    FUN_00250050();
    uVar5 = in_ECX[0xe];
    _DAT_00261098 = uVar7;
    FUN_00250050();
    _DAT_0026109c = uVar5;
  }
  if (DAT_002d5184 != *(byte *)(in_ECX + 6)) {
    DAT_002d5184 = (uint)*(byte *)(in_ECX + 6);
    bVar1 = *(byte *)(in_ECX + 6);
    FUN_00250050();
    DAT_00261090 = (uint)bVar1;
  }
  if (DAT_002d5188 != in_ECX[7]) {
    uVar7 = in_ECX[7];
    DAT_002d5188 = in_ECX[7];
    FUN_00250050();
    _DAT_00261094 = uVar7;
  }
  if (DAT_002d518c != in_ECX[8]) {
    uVar7 = in_ECX[8];
    DAT_002d518c = in_ECX[8];
    FUN_00250050();
    _DAT_00261088 = uVar7;
  }
  iVar3 = in_ECX[10];
  if (DAT_002d5198 != iVar3) {
    DAT_002d5198 = iVar3;
    if (in_ECX[10] == 0) {
      FUN_00250050();
      DAT_0026108c = 0;
      _DAT_002d5168 = *in_ECX;
      return 1;
    }
    if (in_ECX[10] != 1) {
      _DAT_002d5168 = *in_ECX;
      return 1;
    }
    FUN_00250050();
    DAT_0026108c = 1;
  }
  _DAT_002d5168 = *in_ECX;
  return 1;
}

