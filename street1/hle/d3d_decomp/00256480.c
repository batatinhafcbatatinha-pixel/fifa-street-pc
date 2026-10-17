// callers: 0x2578d0

void FUN_00256480(undefined4 *param_1)

{
  uint uVar1;
  uint uVar2;
  undefined4 *puVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  
  if (param_1[0x1e1] == 0) {
    uVar7 = param_1[6];
    iVar4 = 1;
    if ((int)uVar7 < 0) {
      iVar4 = 0;
    }
    else if (DAT_00260ed0 == 0x19) {
      iVar4 = 6;
    }
    else if (DAT_00260ed0 == 0x1a) {
      iVar4 = 7;
    }
    else if ((uVar7 & 4) == 0) {
      if (((uVar7 & 0x40000000) != 0) || (((byte)uVar7 & 0xf0) == 0x30)) {
        iVar4 = 2;
      }
    }
    else {
      iVar4 = 3;
    }
    uVar7 = param_1[5];
    uVar6 = 1;
    if ((int)uVar7 < 0) {
      uVar6 = 0;
    }
    else if (DAT_00260e50 == 0x19) {
      uVar6 = 6;
    }
    else if (DAT_00260e50 == 0x1a) {
      uVar6 = 7;
    }
    else if ((uVar7 & 4) == 0) {
      if (((uVar7 & 0x40000000) != 0) || (((byte)uVar7 & 0xf0) == 0x30)) {
        uVar6 = 2;
      }
    }
    else {
      uVar6 = 3;
    }
    uVar1 = param_1[4];
    uVar7 = 1;
    if ((int)uVar1 < 0) {
      uVar7 = 0;
    }
    else if (DAT_00260dd0 == 0x19) {
      uVar7 = 6;
    }
    else if (DAT_00260dd0 == 0x1a) {
      uVar7 = 7;
    }
    else if ((uVar1 & 4) == 0) {
      if (((uVar1 & 0x40000000) != 0) || (((byte)uVar1 & 0xf0) == 0x30)) {
        uVar7 = 2;
      }
    }
    else {
      uVar7 = 3;
    }
    uVar1 = param_1[3];
    uVar7 = (iVar4 << 5 | uVar6) << 5 | uVar7;
    uVar6 = 1;
    if ((int)uVar1 < 0) {
      uVar7 = uVar7 << 5;
    }
    else if ((uVar1 & 4) == 0) {
      if (((uVar1 & 0x40000000) != 0) || (((byte)uVar1 & 0xf0) == 0x30)) {
        uVar6 = 2;
      }
      uVar7 = uVar7 << 5 | uVar6;
    }
    else {
      uVar7 = uVar7 << 5 | 3;
    }
  }
  else {
    uVar7 = param_1[0x1e4];
    if (param_1[0x1e3] != 0) {
      uVar6 = param_1[6];
      uVar1 = uVar7 >> 0xf & 0x1f;
      if (((((int)uVar6 < 0) && (uVar1 != 4)) && (uVar1 != 5)) && ((uVar1 != 10 && (uVar1 != 0x11)))
         ) {
        uVar1 = 0;
      }
      else if ((uVar1 == 0) || (3 < uVar1)) {
        if ((uVar1 == 0xd) || (uVar1 == 0xe)) {
          uVar1 = ((uVar6 & 4) != 0) + 0xd;
        }
      }
      else if ((uVar6 & 4) == 0) {
        if (((uVar6 & 0x40000000) == 0) && (((byte)uVar6 & 0xf0) != 0x30)) {
          uVar1 = 1;
        }
        else {
          uVar1 = 2;
        }
      }
      else {
        uVar1 = 3;
      }
      uVar6 = param_1[5];
      uVar5 = uVar7 >> 10 & 0x1f;
      if (((((int)uVar6 < 0) && (uVar5 != 4)) && (uVar5 != 5)) && ((uVar5 != 10 && (uVar5 != 0x11)))
         ) {
        uVar5 = 0;
      }
      else if ((uVar5 == 0) || (3 < uVar5)) {
        if ((uVar5 == 0xd) || (uVar5 == 0xe)) {
          uVar5 = ((uVar6 & 4) != 0) + 0xd;
        }
      }
      else if ((uVar6 & 4) == 0) {
        if (((uVar6 & 0x40000000) == 0) && (((byte)uVar6 & 0xf0) != 0x30)) {
          uVar5 = 1;
        }
        else {
          uVar5 = 2;
        }
      }
      else {
        uVar5 = 3;
      }
      uVar6 = param_1[4];
      uVar2 = uVar7 >> 5 & 0x1f;
      if (((((int)uVar6 < 0) && (uVar2 != 4)) && (uVar2 != 5)) && ((uVar2 != 10 && (uVar2 != 0x11)))
         ) {
        uVar2 = 0;
      }
      else if ((uVar2 == 0) || (3 < uVar2)) {
        if ((uVar2 == 0xd) || (uVar2 == 0xe)) {
          uVar2 = ((uVar6 & 4) != 0) + 0xd;
        }
      }
      else if ((uVar6 & 4) == 0) {
        if (((uVar6 & 0x40000000) == 0) && (((byte)uVar6 & 0xf0) != 0x30)) {
          uVar2 = 1;
        }
        else {
          uVar2 = 2;
        }
      }
      else {
        uVar2 = 3;
      }
      uVar6 = param_1[3];
      uVar7 = uVar7 & 0x1f;
      if (((((int)uVar6 < 0) && (uVar7 != 4)) && (uVar7 != 5)) && ((uVar7 != 10 && (uVar7 != 0x11)))
         ) {
        uVar7 = 0;
      }
      else if ((uVar7 == 0) || (3 < uVar7)) {
        if ((uVar7 == 0xd) || (uVar7 == 0xe)) {
          uVar7 = ((uVar6 & 4) != 0) + 0xd;
        }
      }
      else if ((uVar6 & 4) == 0) {
        if (((uVar6 & 0x40000000) == 0) && (((byte)uVar6 & 0xf0) != 0x30)) {
          uVar7 = 1;
        }
        else {
          uVar7 = 2;
        }
      }
      else {
        uVar7 = 3;
      }
      uVar7 = ((uVar1 << 5 | uVar5) << 5 | uVar2) << 5 | uVar7;
    }
  }
  puVar3 = (undefined4 *)*param_1;
  if ((undefined4 *)param_1[1] <= puVar3) {
    puVar3 = (undefined4 *)FUN_00252040();
  }
  *puVar3 = 0x41e70;
  puVar3[1] = uVar7;
  *param_1 = puVar3 + 2;
  return;
}

