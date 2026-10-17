// callers: 0x2578d0

void FUN_002576d0(uint *param_1,int param_2)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  undefined4 *puVar4;
  int *piVar5;
  undefined4 *puVar6;
  int *piVar7;
  int *local_158;
  uint local_154;
  undefined1 local_150 [64];
  undefined1 local_110 [64];
  undefined1 local_d0 [64];
  undefined4 local_90 [16];
  int local_50 [19];
  
  if (((-1 < param_2) && ((*(byte *)(param_1[0x1e5] + 4) & 0x12) == 0)) &&
     ((param_1[0x64a] & 0xffffffef) != 1)) {
    uVar1 = *param_1;
    if (param_1[1] <= uVar1) {
      uVar1 = FUN_00252040();
    }
    FUN_002521e0(local_150,param_1 + 0x378,param_1 + 0x318);
    FUN_00251780(uVar1,0x400480,local_150);
    puVar4 = (undefined4 *)(uVar1 + 0x44);
    if ((param_1[0x254] != 0) || (DAT_00261138 != 0)) {
      FUN_00252460(local_90,local_150,DAT_002611d8 == 0);
      *puVar4 = &DAT_00300580;
      puVar4 = local_90;
      puVar6 = (undefined4 *)(uVar1 + 0x48);
      for (iVar3 = 0xc; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar6 = *puVar4;
        puVar4 = puVar4 + 1;
        puVar6 = puVar6 + 1;
      }
      puVar4 = (undefined4 *)(uVar1 + 0x78);
    }
    if (DAT_002611c4 == 0) {
      FUN_002521e0(local_d0,local_150,param_1 + 0x260);
      FUN_00251780(puVar4,0x400680,local_d0);
      *param_1 = (uint)(puVar4 + 0x11);
      return;
    }
    FUN_00251780(puVar4,0x400680,param_1 + 0x260);
    piVar2 = puVar4 + 0x11;
    *param_1 = (uint)piVar2;
    if ((int *)param_1[1] <= piVar2) {
      piVar2 = (int *)FUN_00252040();
    }
    local_158 = piVar2 + 1;
    local_154 = 0x4004c0;
    do {
      FUN_002521e0(local_110,(int)param_1 + (local_154 - 0x3ff6a0),param_1 + 0x318);
      FUN_00251780(piVar2,local_154,local_110);
      piVar5 = piVar2 + 0x11;
      if ((param_1[0x254] != 0) || (piVar7 = local_158 + 0x11, DAT_00261138 != 0)) {
        FUN_00252460(local_50,local_110,DAT_002611d8 == 0);
        *piVar5 = local_154 - 0xfff00;
        piVar5 = piVar2 + 0x1e;
        piVar2 = local_50;
        piVar7 = local_158 + 0x11;
        for (iVar3 = 0xc; iVar3 != 0; iVar3 = iVar3 + -1) {
          *piVar7 = *piVar2;
          piVar2 = piVar2 + 1;
          piVar7 = piVar7 + 1;
        }
        piVar7 = local_158 + 0x1e;
      }
      local_158 = piVar7;
      local_154 = local_154 + 0x40;
      piVar2 = piVar5;
    } while (local_154 < 0x400580);
    *param_1 = (uint)piVar5;
  }
  return;
}

