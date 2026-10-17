// callers: 0x257380

/* WARNING: Removing unreachable block (ram,0x002570fa) */
/* WARNING: Removing unreachable block (ram,0x002570d7) */
/* WARNING: Removing unreachable block (ram,0x0025711c) */

void FUN_00257080(uint param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  undefined4 *in_EAX;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  float local_1c;
  float local_14;
  float local_10;
  float local_c;
  int local_8;
  
  iVar4 = DAT_00261238;
  local_8 = DAT_002611d4;
  iVar6 = DAT_00261238 + 0xf00;
  uVar8 = 0xc0a10;
  uVar9 = 0xc03a8;
  uVar7 = 0x403b4;
  uVar5 = DAT_0026116c;
  while( true ) {
    local_14 = (float)(uVar5 >> 0x10 & 0xff) * 0.003921569;
    local_10 = (float)(uVar5 >> 8 & 0xff) * 0.003921569;
    fVar3 = (float)(uVar5 & 0xff) * 0.003921569;
    if ((param_1 & 0xc) == 0) {
      fVar1 = local_14 * *(float *)(iVar6 + 0x10);
      if ((param_1 & 3) == 0) {
        fVar1 = fVar1 + *(float *)(iVar6 + 0x30);
        local_14 = 0.0;
        fVar2 = local_10 * *(float *)(iVar6 + 0x14);
        local_10 = 0.0;
        local_c = 0.0;
        fVar2 = fVar2 + *(float *)(iVar6 + 0x34);
        local_1c = fVar3 * *(float *)(iVar6 + 0x18) + *(float *)(iVar6 + 0x38);
      }
      else {
        local_14 = 1.0;
        fVar2 = local_10 * *(float *)(iVar6 + 0x14);
        local_10 = 1.0;
        local_c = 1.0;
        local_1c = fVar3 * *(float *)(iVar6 + 0x18);
      }
    }
    else {
      fVar1 = *(float *)(iVar6 + 0x30);
      fVar2 = *(float *)(iVar6 + 0x34);
      local_1c = *(float *)(iVar6 + 0x38);
      local_c = fVar3;
    }
    in_EAX[3] = local_1c;
    in_EAX[1] = fVar1;
    in_EAX[5] = local_14;
    in_EAX[2] = fVar2;
    in_EAX[6] = local_10;
    *in_EAX = uVar8;
    in_EAX[4] = uVar9;
    in_EAX[7] = local_c;
    in_EAX[8] = uVar7;
    in_EAX[9] = *(undefined4 *)(iVar6 + 0xc);
    in_EAX = in_EAX + 10;
    if (local_8 == 0) break;
    param_1 = param_1 >> 8;
    iVar6 = iVar4 + 0xf44;
    uVar8 = 0xc17a0;
    uVar9 = 0xc17b0;
    uVar7 = 0x417ac;
    uVar5 = DAT_00261168;
    local_8 = local_8 + -1;
  }
  return;
}

