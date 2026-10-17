
int FUN_00195ad0(undefined4 param_1,undefined4 param_2,int param_3)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  
  iVar4 = (*(code *)PTR_FUN_002d4d38)(0x4c,0);
  if (iVar4 == 0) {
    iVar4 = 0;
  }
  else {
    iVar4 = FUN_001953e0();
  }
  *(undefined4 *)(*(int *)(iVar4 + 0x40) + 0x18) = 0;
  *(undefined4 *)(iVar4 + 0x3c) = 0;
  *(undefined4 *)(iVar4 + 0x38) = 0;
  *(undefined4 *)(*(int *)(iVar4 + 0x40) + 8) = param_1;
  *(undefined4 *)(*(int *)(iVar4 + 0x40) + 0xc) = param_2;
  *(uint *)(*(int *)(iVar4 + 0x40) + 0x10) =
       *(uint *)(*(int *)(iVar4 + 0x40) + 0x10) & 0xffffffc1 | 1;
  puVar1 = (uint *)(*(int *)(iVar4 + 0x40) + 0x10);
  *puVar1 = *puVar1 & 0xfeffffff;
  uVar2 = *(uint *)(*(int *)(iVar4 + 0x40) + 0x10);
  *(uint *)(*(int *)(iVar4 + 0x40) + 0x10) = uVar2 ^ (param_3 << 6 ^ uVar2) & 0x3fc0;
  *(undefined4 *)(iVar4 + 4) = 3;
  *(undefined4 *)(iVar4 + 8) = 3;
  *(undefined4 *)(iVar4 + 0xc) = 3;
  *(undefined4 *)(iVar4 + 0x10) = 3;
  if (param_3 == 0x10) {
    *(undefined4 *)(*(int *)(iVar4 + 0x40) + 0x14) = 0x30;
  }
  else if (param_3 == 0x20) {
    *(undefined4 *)(*(int *)(iVar4 + 0x40) + 0x14) = 0x2e;
  }
  iVar3 = *(int *)(iVar4 + 0x40);
  uVar5 = FUN_0025a860(*(undefined4 *)(iVar3 + 8),*(undefined4 *)(iVar3 + 0xc),2,
                       *(undefined4 *)(iVar3 + 0x14));
  *(undefined4 *)(iVar3 + 0x24) = uVar5;
  *(undefined1 *)(iVar4 + 0x20) = 0;
  *(uint *)(*(int *)(iVar4 + 0x40) + 0x28) = *(uint *)(*(int *)(iVar4 + 0x40) + 0x28) | 1;
  puVar6 = (undefined4 *)(*(code *)PTR_FUN_002d4d38)(0x14,"EGLRTZShape");
  *puVar6 = 0;
  puVar6[1] = 0;
  puVar6[2] = 0;
  puVar6[3] = 0;
  puVar6[4] = 0;
  if (param_3 == 0x10) {
    *(undefined1 *)puVar6 = 0x78;
  }
  else if (param_3 == 0x20) {
    *(undefined1 *)puVar6 = 0x7d;
  }
  *(undefined2 *)(puVar6 + 1) = (undefined2)param_1;
  *(undefined2 *)((int)puVar6 + 6) = (undefined2)param_2;
  puVar6[3] = puVar6[3] | 0x1000;
  puVar6[4] = (*(int *)(*(int *)(*(int *)(iVar4 + 0x40) + 0x24) + 4) - (int)puVar6) + -0x80000000;
  *(undefined4 **)(*(int *)(iVar4 + 0x40) + 0x18) = puVar6;
  return iVar4;
}

