// callers: 0x259df0

void FUN_0025c75c(void)

{
  uint *puVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int *in_ECX;
  uint uVar5;
  int local_4;
  
  iVar2 = *in_ECX;
  puVar1 = (uint *)(iVar2 + 0x200);
  *puVar1 = *puVar1 & 0xffffefff;
  *puVar1 = *puVar1 | 0x1000;
  in_ECX[0x1fb] = 0;
  in_ECX[0x1ff] = 0;
  in_ECX[0x202] = 0;
  in_ECX[0x205] = 0;
  in_ECX[0x1fc] = 0x118700;
  in_ECX[0x1fd] = (int)"_pitch2_movement";
  in_ECX[0x1fe] = -0xc21fb87;
  in_ECX[0x200] = 4;
  in_ECX[0x201] = 0x45ead10e;
  in_ECX[0x203] = 0x78;
  in_ECX[0x204] = 0x40;
  *(undefined4 *)(iVar2 + 0x400080) = 0;
  *(int *)(iVar2 + 0x400084) = in_ECX[0x1fc];
  *(int *)(iVar2 + 0x400880) = in_ECX[0x1fd];
  *(int *)(iVar2 + 0x40008c) = in_ECX[0x1fe];
  *(int *)(iVar2 + 0x400090) = in_ECX[0x1ff];
  *(int *)(iVar2 + 0x400094) = in_ECX[0x200];
  *(int *)(iVar2 + 0x400b80) = in_ECX[0x201];
  *(int *)(iVar2 + 0x400b84) = in_ECX[0x202];
  *(int *)(iVar2 + 0x400098) = in_ECX[0x203];
  *(int *)(iVar2 + 0x40009c) = in_ECX[0x204];
  *(int *)(iVar2 + 0x400b88) = in_ECX[0x205];
  *(uint *)(iVar2 + 0x400780) = in_ECX[0x50] & 0xffff;
  FUN_0025b717();
  uVar5 = 0x50;
  puVar4 = (undefined4 *)(iVar2 + 0x400904);
  do {
    uVar3 = puVar4[-0xc01b0];
    *puVar4 = uVar3;
    *(uint *)(iVar2 + 0x400750) = uVar5 - 0x20 & 0x1ffc | 0xea0000;
    *(undefined4 *)(iVar2 + 0x400754) = uVar3;
    uVar3 = puVar4[-0xc01af];
    puVar4[1] = uVar3;
    *(uint *)(iVar2 + 0x400750) = uVar5 & 0x1ffc | 0xea0000;
    *(undefined4 *)(iVar2 + 0x400754) = uVar3;
    uVar3 = puVar4[-0xc01b1];
    puVar4[-1] = uVar3;
    *(uint *)(iVar2 + 0x400750) = uVar5 - 0x40 & 0x1ffc | 0xea0000;
    uVar5 = uVar5 + 4;
    puVar4 = puVar4 + 4;
    *(undefined4 *)(iVar2 + 0x400754) = uVar3;
  } while ((int)uVar5 < 0x70);
  puVar4 = (undefined4 *)(iVar2 + 0x400980);
  local_4 = 8;
  do {
    uVar3 = puVar4[-0xc01a0];
    uVar5 = (-0x4008f0 - iVar2) + (int)puVar4;
    *puVar4 = uVar3;
    puVar4 = puVar4 + 1;
    local_4 = local_4 + -1;
    *(uint *)(iVar2 + 0x400750) = uVar5 & 0x1ffc | 0xea0000;
    *(undefined4 *)(iVar2 + 0x400754) = uVar3;
  } while (local_4 != 0);
  *(undefined4 *)(iVar2 + 0x4009a0) = *(undefined4 *)(iVar2 + 0x100324);
  *(undefined4 *)(iVar2 + 0x400750) = 0xea000c;
  *(undefined4 *)(iVar2 + 0x400754) = *(undefined4 *)(iVar2 + 0x100324);
  *(undefined4 *)(iVar2 + 0x4009a4) = *(undefined4 *)(iVar2 + 0x100200);
  *(undefined4 *)(iVar2 + 0x400750) = 0xea0000;
  *(undefined4 *)(iVar2 + 0x400754) = *(undefined4 *)(iVar2 + 0x100200);
  *(undefined4 *)(iVar2 + 0x4009a8) = *(undefined4 *)(iVar2 + 0x100204);
  *(undefined4 *)(iVar2 + 0x400750) = 0xea0004;
  *(undefined4 *)(iVar2 + 0x400754) = *(undefined4 *)(iVar2 + 0x100204);
  *(undefined4 *)(iVar2 + 0x40014c) = 0;
  *(undefined4 *)(iVar2 + 0x400150) = 0;
  *(undefined4 *)(iVar2 + 0x400154) = 0;
  *(undefined4 *)(iVar2 + 0x400158) = 0;
  *(undefined4 *)(iVar2 + 0x400144) = 0x10000000;
  *(undefined4 *)(iVar2 + 0x400764) = 0x8000000;
  FUN_0025c5b8(in_ECX[0x4d]);
  return;
}

