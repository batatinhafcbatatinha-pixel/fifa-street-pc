// callers: 0x1970c0 0x1980d0 0x252f00 0x255bf0 0x25d660 0x25d850 0x25dd90

void FUN_00252fb0(int param_1,int param_2)

{
  byte bVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 local_c;
  int local_8;
  
  puVar2 = DAT_00261238;
  if (param_1 == 0) {
    param_1 = DAT_00261238[0x681];
  }
  FUN_00257c20(param_1);
  iVar5 = puVar2[0x681];
  if (iVar5 != 0) {
    if (*(int *)(iVar5 + 0x14) != 0) {
      iVar5 = *(int *)(iVar5 + 0x14);
    }
    *(undefined4 *)(iVar5 + 8) = puVar2[0xb];
    FUN_00257e50(puVar2[0x681]);
  }
  iVar5 = puVar2[0x682];
  puVar2[0x681] = param_1;
  if (iVar5 != 0) {
    if (*(int *)(iVar5 + 0x14) != 0) {
      iVar5 = *(int *)(iVar5 + 0x14);
    }
    *(undefined4 *)(iVar5 + 8) = puVar2[0xb];
    FUN_00257e50(puVar2[0x682]);
  }
  puVar2[0x682] = param_2;
  if (param_2 != 0) {
    FUN_00257c20(param_2);
    FUN_00258320(param_2,puVar2 + 0x252);
    uVar3 = FUN_00252e10(param_2);
    puVar2[0x253] = uVar3;
  }
  puVar4 = (undefined4 *)*puVar2;
  if ((undefined4 *)puVar2[1] <= puVar4) {
    puVar4 = (undefined4 *)FUN_00252040();
  }
  uVar3 = *(undefined4 *)(param_1 + 4);
  if (param_2 == 0) {
    local_c = 0;
  }
  else {
    local_c = *(undefined4 *)(param_2 + 4);
  }
  local_8 = 2;
  do {
    *puVar4 = 0x40100;
    puVar4[1] = 0;
    if (*(int *)(param_1 + 0x10) == 0) {
      iVar5 = 1 << ((byte)(*(uint *)(param_1 + 0xc) >> 0x14) & 0xf);
      bVar1 = *(byte *)(param_1 + 0xd);
      if (bVar1 == 0xc) {
        uVar7 = iVar5 * 2;
      }
      else if ((bVar1 < 0xe) || (0xf < bVar1)) {
        uVar7 = ((byte)(&DAT_0025ed88)[*(byte *)(param_1 + 0xd)] & 0x3c) * iVar5 >> 3;
      }
      else {
        uVar7 = iVar5 * 4;
      }
    }
    else {
      uVar7 = (*(byte *)(param_1 + 0x13) + 1) * 0x40;
    }
    iVar5 = puVar2[0x253];
    puVar4[2] = 0x4020c;
    puVar4[3] = iVar5 << 0x10 | uVar7 & 0xffff;
    puVar4[4] = 0x40100;
    puVar4[5] = 0;
    puVar4[6] = 0x40110;
    puVar4[7] = 0;
    puVar4[8] = 0x40100;
    puVar4[9] = 0;
    puVar4[10] = 0x40210;
    puVar4[0xb] = uVar3;
    puVar4[0xc] = 0x40100;
    puVar4[0xd] = 0;
    puVar4[0xe] = 0x40110;
    puVar4[0xf] = 0;
    puVar4[0x10] = 0x40100;
    puVar4[0x11] = 0;
    puVar4[0x12] = 0x40214;
    puVar4[0x13] = local_c;
    puVar4[0x14] = 0x40100;
    puVar4[0x15] = 0;
    puVar4[0x16] = 0x40110;
    puVar4[0x17] = 0;
    puVar4 = puVar4 + 0x18;
    local_8 = local_8 + -1;
  } while (local_8 != 0);
  puVar4 = (undefined4 *)FUN_00250e10();
  if ((DAT_002611dc == 0) || (puVar2[0x682] == 0)) {
    uVar3 = 0;
  }
  else {
    uVar3 = 1;
  }
  *puVar4 = 0x4030c;
  puVar4[1] = uVar3;
  if ((DAT_002611e0 == 0) || (puVar2[0x682] == 0)) {
    uVar3 = 0;
  }
  else {
    uVar3 = 1;
  }
  puVar4[2] = 0x4032c;
  puVar4[3] = uVar3;
  puVar6 = puVar4 + 4;
  if (((puVar2[2] & 0x100) != 0) && (param_1 == puVar2[0x685])) {
    puVar2[2] = puVar2[2] & 0xfffffeff;
    *puVar6 = 0x40130;
    puVar4[5] = 0;
    puVar6 = puVar4 + 6;
  }
  *puVar2 = puVar6;
  uVar3 = FUN_00258200(param_1,param_2);
  puVar2[0x683] = uVar3;
  FUN_00250f40();
  FUN_002505e0(DAT_0026121c);
  return;
}

