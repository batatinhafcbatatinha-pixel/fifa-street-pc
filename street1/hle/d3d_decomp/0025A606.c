// callers: 0x2560c0

void FUN_0025a606(void)

{
  byte *pbVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int iVar6;
  int *in_ECX;
  undefined4 *puVar7;
  uint local_8;
  
  iVar2 = *in_ECX;
  if (in_ECX[0x206] != 0) {
    *(undefined4 *)(iVar2 + 0x9140) = 0;
    in_ECX[0x206] = 0;
  }
  iVar6 = (*(code *)0x80000001)();
  if (iVar6 == 0) {
    (*(code *)0x80000002)(iVar2,9,1,0);
  }
  local_8 = 0;
  do {
    FUN_0025a544(local_8,1);
    local_8 = local_8 + 1;
  } while ((int)local_8 < 8);
  *(undefined4 *)(iVar2 + 0x2044) = 0;
  while ((((*(byte *)(iVar2 + 0x3214) & 0x10) == 0 || ((*(byte *)(iVar2 + 0x2400) & 0x10) == 0)) ||
         ((*(byte *)(iVar2 + 0x3220) & 0x10) != 0))) {
    FUN_002599b0();
    if (*(int *)(iVar2 + 0x400100) != 0) {
      FUN_00259900();
    }
    if ((*(uint *)(iVar2 + 0x100) & 0x1000000) != 0) {
      FUN_00259390();
    }
  }
  pbVar1 = (byte *)(iVar2 + 0x3220);
  pbVar1[0] = 0;
  pbVar1[1] = 0;
  pbVar1[2] = 0;
  pbVar1[3] = 0;
  do {
  } while ((*pbVar1 & 0x10) != 0);
  *(undefined4 *)(iVar2 + 0x2500) = 0;
  *(undefined4 *)(iVar2 + 0x3000) = 0;
  *(undefined4 *)(iVar2 + 0x3050) = 0;
  *(undefined4 *)(iVar2 + 0x3200) = 0;
  *(undefined4 *)(iVar2 + 0x3250) = 0;
  FUN_0025b78f(1);
  *(undefined4 *)(iVar2 + 0x3210) = 0;
  *(undefined4 *)(iVar2 + 0x3270) = 0;
  uVar3 = *(undefined4 *)(iVar2 + 0x2500);
  uVar4 = *(undefined4 *)(iVar2 + 0x3200);
  uVar5 = *(undefined4 *)(iVar2 + 0x3250);
  *(undefined4 *)(iVar2 + 0x2500) = 0;
  *(undefined4 *)(iVar2 + 0x3200) = 0;
  *(undefined4 *)(iVar2 + 0x3250) = 0;
  local_8 = 0;
  do {
    if (in_ECX[0x42] != 0) {
      puVar7 = (undefined4 *)(local_8 + iVar2 + in_ECX[0x4a]);
      puVar7[1] = *puVar7;
      *(undefined4 *)(local_8 + iVar2 + 0x10 + in_ECX[0x4a]) = 0;
    }
    local_8 = local_8 + 0x40;
  } while (local_8 < 0x80);
  *(undefined4 *)(iVar2 + 0x3250) = uVar5;
  *(undefined4 *)(iVar2 + 0x3200) = uVar4;
  *(undefined4 *)(iVar2 + 0x2500) = uVar3;
  *(undefined4 *)(iVar2 + 0x2508) = 0;
  *(undefined4 *)(iVar2 + 0x2140) = 0;
  FUN_0025c5b8(2);
  *(int *)(iVar2 + 0x100200) = in_ECX[0x53];
  *(int *)(iVar2 + 0x100204) = in_ECX[0x54];
  *(int *)(iVar2 + 0x200) = in_ECX[0x55];
  *(int *)(iVar2 + 0x140) = in_ECX[0x56];
  (*(code *)0x8000002f)(in_ECX + 0x59,0);
  (*(code *)0x80000064)(in_ECX + 5);
  return;
}

