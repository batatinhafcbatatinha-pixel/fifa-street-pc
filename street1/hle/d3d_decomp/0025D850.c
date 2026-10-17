// callers: 0x25db90

void FUN_0025d850(byte param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  uint uVar6;
  uint uVar7;
  undefined4 *unaff_ESI;
  undefined1 local_170 [368];
  
  if ((param_1 & 3) != 0) {
    if (unaff_ESI[0x684] == 3) {
      iVar1 = unaff_ESI[0x686];
      iVar2 = unaff_ESI[0x687];
      uVar3 = *(undefined4 *)(iVar2 + 4);
      uVar4 = *(undefined4 *)(iVar2 + 8);
      *(undefined4 *)(iVar2 + 4) = *(undefined4 *)(iVar1 + 4);
      *(undefined4 *)(iVar2 + 8) = *(undefined4 *)(iVar1 + 8);
      *(undefined4 *)(iVar1 + 8) = uVar4;
      *(undefined4 *)(iVar1 + 4) = uVar3;
    }
    FUN_0025d5d0();
    FUN_00252fb0(unaff_ESI[0x686],0);
    FUN_00253800(0,unaff_ESI[0x685]);
    FUN_00253800(1,0);
    FUN_00253800(2,0);
    FUN_00253800(3,0);
    if ((param_1 & 1) != 0) {
      FUN_0025d130();
      FUN_0025d330();
    }
    puVar5 = (undefined4 *)*unaff_ESI;
    if ((undefined4 *)unaff_ESI[1] <= puVar5) {
      puVar5 = (undefined4 *)FUN_00252040();
    }
    if ((uint)unaff_ESI[0x684] < 3) {
      if (DAT_0026119c != 0x80000000) {
        *puVar5 = 0x40110;
        puVar5[1] = 0;
        puVar5 = puVar5 + 2;
      }
      uVar6 = *(uint *)(unaff_ESI[0x686] + 4);
      uVar7 = DAT_0026119c;
      if (DAT_0026119c == 0) {
        uVar7 = 1;
      }
      if ((uVar7 & 1) != 0) {
        uVar6 = uVar6 | 1;
      }
      if ((uVar7 & 2) != 0) {
        uVar6 = uVar6 | 2;
      }
      if ((uVar7 & 4) != 0) {
        uVar6 = uVar6 | 3;
      }
      if ((int)uVar7 < 0) {
        uVar6 = uVar6 | 8;
      }
      *puVar5 = 0x40100;
      puVar5[1] = uVar6 << 5 | 1;
      puVar5[2] = 0x4012c;
      puVar5[3] = 0;
      puVar5[4] = 0x40100;
      puVar5[5] = 0;
      puVar5[6] = 0x40130;
      puVar5[7] = 0;
      puVar5 = puVar5 + 8;
    }
    else {
      *puVar5 = 0x4012c;
      puVar5[1] = 0;
      puVar5[2] = 0x40100;
      puVar5[3] = 0;
      puVar5[4] = 0x40130;
      puVar5[5] = 0;
      puVar5 = puVar5 + 6;
    }
    *unaff_ESI = puVar5;
  }
  if ((param_1 & 1) != 0) {
    FUN_0025d6d0();
    FUN_0025d450(local_170);
  }
  if ((param_1 & 4) != 0) {
    FUN_0025d660();
    if (unaff_ESI[0x684] == 3) {
      puVar5 = (undefined4 *)*unaff_ESI;
      if ((undefined4 *)unaff_ESI[1] <= puVar5) {
        puVar5 = (undefined4 *)FUN_00252040();
      }
      *puVar5 = 0x40110;
      puVar5[1] = 0;
      uVar6 = *(uint *)(unaff_ESI[0x686] + 4);
      uVar7 = DAT_0026119c;
      if (DAT_0026119c == 0) {
        uVar7 = 1;
      }
      if ((uVar7 & 1) != 0) {
        uVar6 = uVar6 | 1;
      }
      if ((uVar7 & 2) != 0) {
        uVar6 = uVar6 | 2;
      }
      if ((uVar7 & 4) != 0) {
        uVar6 = uVar6 | 3;
      }
      if ((int)uVar7 < 0) {
        uVar6 = uVar6 | 8;
      }
      puVar5[2] = 0x40100;
      puVar5[3] = uVar6 << 5 | 1;
      *unaff_ESI = puVar5 + 4;
    }
  }
  return;
}

