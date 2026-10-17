// callers: 0x196cc0 0x198750 0x252f00 0x255bf0

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Switch with 1 destination removed at 0x00258e66 */
/* WARNING: Exceeded maximum restarts with more pending */

void FUN_00258d70(int param_1,int *param_2,uint param_3,uint param_4,undefined4 param_5,
                 undefined4 param_6)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int local_34;
  uint local_28;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  puVar3 = DAT_00261238;
  iVar1 = DAT_00261238[0x682];
  iVar2 = DAT_00261238[0x681];
  local_28 = 0;
  if (((&DAT_0025ed88)[*(byte *)(iVar2 + 0xd)] & 1) != 0) {
    local_28 = FUN_00258200(iVar2,iVar1);
    puVar4 = (undefined4 *)*puVar3;
    if ((undefined4 *)puVar3[1] <= puVar4) {
      puVar4 = (undefined4 *)FUN_00252040();
    }
    *puVar4 = 0x40208;
    puVar4[1] = local_28 & 0xfffffdff | 0x100;
    *puVar3 = puVar4 + 2;
  }
  if ((param_3 & 0xf0) != 0) {
    switch(*(undefined1 *)(iVar2 + 0xd)) {
    case 3:
    case 0x1c:
      uVar5 = (param_4 >> 3 & 0x1f0000 | param_4 & 0xf800) >> 3;
      break;
    default:
      goto switchD_00258df7_caseD_4;
    case 5:
    case 0x11:
      uVar5 = (param_4 >> 3 & 0x1f0000 | param_4 & 0xfc00) >> 2;
    }
    param_4 = (uVar5 | param_4 & 0xf8) >> 3;
  }
switchD_00258df7_caseD_4:
  if ((iVar1 != 0) || (param_3 = param_3 & 0xfffffffc, param_3 != 0)) {
    if ((param_3 & 1) != 0) {
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
    iVar1 = puVar3[0x3b8];
    iVar2 = puVar3[0x3b9];
    iVar9 = puVar3[0x3ba] + iVar1;
    iVar10 = puVar3[0x3bb] + iVar2;
    if (param_1 == 0) {
      param_2 = &local_10;
      local_10 = iVar1;
      local_c = iVar2;
      local_8 = iVar9;
      local_4 = iVar10;
      param_1 = 1;
    }
    do {
      local_34 = *param_2;
      if (*param_2 <= iVar1) {
        local_34 = iVar1;
      }
      param_5 = param_2[1];
      if (param_2[1] <= iVar2) {
        param_5 = iVar2;
      }
      iVar6 = param_2[2];
      if (iVar9 <= param_2[2]) {
        iVar6 = iVar9;
      }
      iVar8 = param_2[3];
      if (iVar10 <= param_2[3]) {
        iVar8 = iVar10;
      }
      if ((local_34 < iVar6) && (param_5 < iVar8)) {
        uVar5 = FUN_002529b0((float)local_34 * (float)puVar3[599] + 0.5);
        iVar6 = FUN_002529b0((float)iVar6 * (float)puVar3[599] + 0.5);
        uVar7 = FUN_002529b0((float)param_5 * (float)puVar3[600] + 0.5);
        iVar8 = FUN_002529b0((float)iVar8 * (float)puVar3[600] + 0.5);
        puVar4 = (undefined4 *)*puVar3;
        if ((undefined4 *)puVar3[1] <= puVar4) {
          puVar4 = (undefined4 *)FUN_00252040();
        }
        *puVar4 = &DAT_00081d98;
        puVar4[1] = iVar6 * 0x10000 - 0x10000U | uVar5;
        puVar4[2] = iVar8 * 0x10000 - 0x10000U | uVar7;
        puVar4[3] = 0xc1d8c;
        puVar4[4] = param_6;
        puVar4[5] = param_4;
        puVar4[6] = param_3;
        *puVar3 = puVar4 + 7;
      }
      param_2 = param_2 + 4;
      param_1 = param_1 + -1;
    } while (param_1 != 0);
    if (local_28 != 0) {
      puVar4 = (undefined4 *)*puVar3;
      if ((undefined4 *)puVar3[1] <= puVar4) {
        puVar4 = (undefined4 *)FUN_00252040();
      }
      *puVar4 = 0x40208;
      puVar4[1] = local_28;
      *puVar3 = puVar4 + 2;
    }
  }
  return;
}

