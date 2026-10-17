// callers: 0x2578d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_002569f0(undefined4 *param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  bool bVar5;
  float local_8;
  
  puVar2 = param_1;
  FUN_00254ff0();
  if (DAT_00261110 == 0) {
    puVar2 = (undefined4 *)*param_1;
    if ((undefined4 *)param_1[1] <= puVar2) {
      puVar2 = (undefined4 *)FUN_00252040();
    }
    *puVar2 = 0x402a4;
    puVar2[1] = 0;
    if ((param_1[0x1e1] != 0) && (param_1[0x1e2] != 0)) {
      *param_1 = puVar2 + 2;
      return;
    }
    bVar5 = DAT_0026113c != 0;
    puVar2[2] = &PTR_s_cSlideTackle_00080288;
    puVar2[3] = (uint)bVar5 * 2 + 0xc;
    puVar2[4] = 0x1c80;
    *param_1 = puVar2 + 5;
    return;
  }
  iVar3 = (DAT_00261124 == 0) + 1;
  if (DAT_00261114 == 0) {
    param_1 = (undefined4 *)0x3f800000;
    local_8 = 1.0;
    uVar4 = 0x2601;
    iVar3 = 0;
  }
  else if (DAT_00261114 == 3) {
    local_8 = _DAT_0025f4e0;
    if (DAT_0026111c != DAT_00261118) {
      local_8 = 1.0 / (DAT_0026111c - DAT_00261118);
    }
    uVar4 = 0x2601;
    param_1 = (undefined4 *)(DAT_0026111c * local_8 + 1.0);
    local_8 = -local_8;
  }
  else {
    param_1 = (undefined4 *)0x3fc00000;
    if (DAT_00261114 == 1) {
      local_8 = DAT_00261120 * -0.090168074;
      uVar4 = 0x800;
    }
    else {
      local_8 = DAT_00261120 * -0.21233003;
      uVar4 = 0x801;
    }
  }
  puVar1 = (undefined4 *)*puVar2;
  if ((undefined4 *)puVar2[1] <= puVar1) {
    puVar1 = (undefined4 *)FUN_00252040();
  }
  *puVar1 = &LAB_000802a0;
  puVar1[1] = iVar3;
  puVar1[2] = 1;
  puVar1[3] = 0x4029c;
  puVar1[4] = uVar4;
  puVar1[5] = 0xc09c0;
  puVar1[6] = param_1;
  puVar1[7] = local_8;
  puVar1[8] = 0;
  if ((puVar2[0x1e1] != 0) && (puVar2[0x1e2] != 0)) {
    *puVar2 = puVar1 + 9;
    return;
  }
  bVar5 = DAT_0026113c != 0;
  puVar1[9] = &PTR_s_cSlideTackle_00080288;
  puVar1[10] = (-(uint)bVar5 & 0x20000) + 0x130c0300;
  puVar1[0xb] = 0x1c80;
  *puVar2 = puVar1 + 0xc;
  return;
}

