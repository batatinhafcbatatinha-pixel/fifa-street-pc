// callers: 0x25db90

void FUN_0025da40(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  undefined4 *unaff_ESI;
  int *piVar10;
  
  iVar1 = unaff_ESI[0x684];
  iVar6 = unaff_ESI[iVar1 + 0x684];
  uVar2 = *(undefined4 *)(iVar6 + 8);
  uVar3 = *(undefined4 *)(iVar6 + 4);
  iVar8 = iVar1 + -1;
  piVar10 = unaff_ESI + iVar1 + 0x683;
  do {
    iVar1 = *piVar10;
    *(undefined4 *)(iVar6 + 4) = *(undefined4 *)(iVar1 + 4);
    *(undefined4 *)(iVar6 + 8) = *(undefined4 *)(iVar1 + 8);
    iVar8 = iVar8 + -1;
    piVar10 = piVar10 + -1;
    iVar6 = iVar1;
  } while (iVar8 != 0);
  *(undefined4 *)(iVar1 + 4) = uVar3;
  *(undefined4 *)(iVar1 + 8) = uVar2;
  puVar4 = (undefined4 *)*unaff_ESI;
  if ((undefined4 *)unaff_ESI[1] <= puVar4) {
    puVar4 = (undefined4 *)FUN_00252040();
  }
  uVar7 = *(uint *)(unaff_ESI[0x686] + 4);
  *puVar4 = 0x40110;
  puVar4[1] = 0;
  uVar9 = DAT_0026119c;
  if (DAT_0026119c == 0) {
    uVar9 = 1;
  }
  if ((uVar9 & 1) != 0) {
    uVar7 = uVar7 | 1;
  }
  if ((uVar9 & 2) != 0) {
    uVar7 = uVar7 | 2;
  }
  if ((uVar9 & 4) != 0) {
    uVar7 = uVar7 | 3;
  }
  if ((int)uVar9 < 0) {
    uVar7 = uVar7 | 8;
  }
  puVar4[2] = 0x40100;
  puVar4[3] = uVar7 << 5 | 1;
  puVar4[4] = 0x4012c;
  puVar4[5] = 0;
  puVar4[6] = 0x40100;
  puVar4[7] = 0;
  puVar5 = puVar4 + 8;
  if ((unaff_ESI[0x681] == unaff_ESI[0x685]) || (DAT_0026119c == 0x80000000)) {
    *puVar5 = 0x40130;
    puVar4[9] = 0;
    puVar5 = puVar4 + 10;
  }
  else {
    unaff_ESI[2] = unaff_ESI[2] | 0x100;
  }
  uVar2 = *(undefined4 *)(unaff_ESI[0x685] + 4);
  *puVar5 = 0x40210;
  puVar5[1] = uVar2;
  *unaff_ESI = puVar5 + 2;
  return;
}

