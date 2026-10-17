// callers: 0x2584e0

void FUN_00258390(int param_1,int param_2,int *param_3,int *param_4,int *param_5,uint *param_6,
                 uint *param_7)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  
  uVar7 = *(uint *)(param_1 + 0x10);
  if (uVar7 == 0) {
    uVar7 = *(uint *)(param_1 + 0xc);
    iVar5 = (uVar7 >> 0x1c) - param_2;
    uVar4 = (uVar7 >> 0x14 & 0xf) - param_2;
    uVar7 = (uVar7 >> 0x18 & 0xf) - param_2;
    bVar1 = (&DAT_0025ed88)[*(byte *)(param_1 + 0xd)];
    *param_4 = 1 << (((int)uVar7 < 1) - 1U & (byte)uVar7 & 0x1f);
    *param_5 = 1 << ((iVar5 < 1) - 1U & (byte)iVar5 & 0x1f);
    *param_3 = 1 << (((int)uVar4 < 1) - 1U & (byte)uVar4 & 0x1f);
    bVar2 = *(byte *)(param_1 + 0xd);
    bVar3 = 0;
    if ((bVar2 == 0xc) || ((0xd < bVar2 && (bVar2 < 0x10)))) {
      bVar3 = 1;
    }
    uVar6 = -(uint)bVar3 & 2;
    if ((int)uVar4 < (int)uVar6) {
      uVar4 = uVar6;
    }
    iVar5 = 1 << ((byte)uVar4 & 0x1f);
    if ((int)uVar7 < (int)uVar6) {
      uVar7 = uVar6;
    }
    if (bVar2 == 0xc) {
      uVar4 = iVar5 * 2;
    }
    else if ((bVar2 < 0xe) || (0xf < bVar2)) {
      uVar4 = iVar5 * (bVar1 & 0x3c) >> 3;
    }
    else {
      uVar4 = iVar5 * 4;
    }
    *param_6 = uVar4;
    *param_7 = (1 << ((byte)uVar7 & 0x1f)) * iVar5 * (bVar1 & 0x3c) >> 3;
    return;
  }
  *param_3 = (uVar7 & 0xfff) + 1;
  *param_4 = (uVar7 >> 0xc & 0xfff) + 1;
  *param_5 = 1;
  uVar7 = ((uVar7 >> 0x18) + 1) * 0x40;
  *param_6 = uVar7;
  *param_7 = *param_4 * uVar7;
  return;
}

