// callers: 0x250ac0

void FUN_002508c0(uint param_1,uint param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  
  puVar1 = DAT_00261238;
  (&DAT_00260e10)[param_1 * 0x20] = param_2;
  piVar2 = (int *)*puVar1;
  iVar5 = 0;
  if ((int *)puVar1[1] <= piVar2) {
    piVar2 = (int *)FUN_00252040();
  }
  iVar3 = 0;
  uVar4 = param_2 & 0xffff0000;
  if (uVar4 != 0) {
    *piVar2 = param_1 * 4 + 0x41964;
    piVar2[1] = -0x1000000;
    piVar2 = piVar2 + 2;
    param_2 = param_1;
    if (uVar4 < 0x30001) {
      if (uVar4 == 0x30000) {
        iVar5 = 0x8512;
        iVar3 = 1;
      }
      else if (uVar4 == 0x10000) {
        iVar5 = 0x8511;
        iVar3 = 1;
      }
      else {
        iVar5 = 0x2400;
      }
    }
    else {
      iVar5 = 0x2401;
    }
  }
  (&DAT_002cea99)[param_1] = (char)param_2 + '\t';
  *piVar2 = (param_1 + 0xc03c) * 0x10;
  piVar2[1] = iVar5;
  piVar2[2] = iVar5;
  piVar2[3] = iVar5;
  *puVar1 = piVar2 + 4;
  if ((puVar1[0x254] == 0) && (iVar3 != 0)) {
    DAT_00260d98 = DAT_00260d98 | 0x200;
  }
  puVar1[0x254] = ~(1 << ((byte)param_1 & 0x1f)) & puVar1[0x254] | iVar3 << ((byte)param_1 & 0x1f);
  DAT_00260d98 = DAT_00260d98 | 0x47f;
  return;
}

