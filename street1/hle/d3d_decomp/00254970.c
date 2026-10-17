// callers: 0x19f2f0

undefined4 FUN_00254970(int param_1,ushort *param_2,uint *param_3,undefined4 param_4)

{
  ushort *puVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  uint in_ECX;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint local_4;
  
  puVar1 = param_2;
  local_4 = in_ECX;
  if (param_2 != (ushort *)0x0) {
    local_4 = (uint)*param_2;
    param_2 = (ushort *)(uint)param_2[1];
  }
  iVar7 = 0;
  if (puVar1 != (ushort *)0x0) {
    iVar7 = (int)param_2 * 0x10 + ((int)param_2 * 0x10 + 0x7fU >> 7) * 4;
  }
  iVar2 = 0;
  if (param_1 != 0) {
    iVar2 = FUN_00254870(param_1,0);
    iVar2 = iVar2 << 2;
  }
  puVar3 = (undefined4 *)FUN_001609a7(iVar2 + iVar7 + 0x118,0x24800000);
  if (puVar3 == (undefined4 *)0x0) {
    return 0x8007000e;
  }
  puVar4 = puVar3;
  for (iVar5 = 0x46; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  puVar3[1] = param_4;
  *puVar3 = 1;
  puVar4 = puVar3 + 7;
  iVar5 = 0x10;
  do {
    *puVar4 = 2;
    puVar4 = puVar4 + 4;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  if (param_1 != 0) {
    FUN_002546d0(puVar3,param_1,puVar1 == (ushort *)0x0);
  }
  iVar5 = 0;
  if (puVar1 != (ushort *)0x0) {
    uVar6 = puVar3[1];
    puVar3[1] = uVar6 | 0x10;
    if (local_4 != 0x2078) {
      puVar3[1] = uVar6 | 0x11;
    }
    if (local_4 == 0x7378) {
      puVar3[1] = puVar3[1] | 8;
    }
    puVar3[2] = param_2;
    iVar5 = FUN_00254660(puVar3 + 0x45,puVar1 + 2,(int)param_2 << 2);
    iVar5 = iVar5 - (int)(puVar3 + 0x45) >> 2;
  }
  if (param_1 != 0) {
    FUN_00254870(param_1,puVar3 + iVar5 + 0x45);
  }
  uVar6 = (uint)(iVar2 + iVar7) >> 2;
  puVar3[3] = uVar6;
  puVar3[uVar6 + 0x45] = 0;
  *param_3 = (uint)puVar3 | 1;
  return 0;
}

