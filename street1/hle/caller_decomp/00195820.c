
int FUN_00195820(undefined4 param_1)

{
  uint *puVar1;
  int iVar2;
  undefined4 auStack_1c [5];
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  iVar2 = (*(code *)PTR_FUN_002d4d38)(0x4c,0);
  if (iVar2 == 0) {
    iVar2 = 0;
  }
  else {
    iVar2 = FUN_001953e0();
  }
  thunk_FUN_002584e0(param_1,0,auStack_1c);
  *(undefined4 *)(*(int *)(iVar2 + 0x40) + 0x18) = 0;
  *(undefined4 *)(iVar2 + 0x3c) = 0;
  *(undefined4 *)(iVar2 + 0x38) = 0;
  *(undefined4 *)(*(int *)(iVar2 + 0x40) + 8) = uStack_8;
  *(undefined4 *)(*(int *)(iVar2 + 0x40) + 0xc) = uStack_4;
  *(undefined4 *)(*(int *)(iVar2 + 0x40) + 0x14) = auStack_1c[0];
  puVar1 = (uint *)(*(int *)(iVar2 + 0x40) + 0x10);
  *puVar1 = *puVar1 & 0xffffffc0;
  puVar1 = (uint *)(*(int *)(iVar2 + 0x40) + 0x10);
  *puVar1 = *puVar1 & 0xfeffffff;
  puVar1 = (uint *)(*(int *)(iVar2 + 0x40) + 0x10);
  *puVar1 = *puVar1 & 0xffffc03f;
  *(undefined4 *)(iVar2 + 8) = 3;
  *(undefined4 *)(iVar2 + 0xc) = 3;
  *(undefined4 *)(iVar2 + 0x10) = 3;
  *(undefined4 *)(*(int *)(iVar2 + 0x40) + 0x24) = param_1;
  return iVar2;
}

