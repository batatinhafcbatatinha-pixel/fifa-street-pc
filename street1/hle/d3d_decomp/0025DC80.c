// callers: 0x198750

/* WARNING: Removing unreachable block (ram,0x0025dca2) */
/* WARNING: Removing unreachable block (ram,0x0025dcbf) */

void FUN_0025dc80(float param_1,float param_2)

{
  uint uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = DAT_00261238;
  param_1 = (float)((uint)DAT_00261238[0x65b] >> 4 & 0xf) * param_1;
  param_2 = (float)(DAT_00261238[0x65b] & 0xf) * param_2;
  if (DAT_00261238[0x65c] != 3) {
    if ((param_1 == 1.0) && (param_2 == 1.0)) {
      if ((*(int *)(DAT_00261238[0x685] + 0x10) == *(int *)(DAT_00261238[0x686] + 0x10)) &&
         (*(int *)(DAT_00261238[0x685] + 0xc) == *(int *)(DAT_00261238[0x686] + 0xc))) {
        DAT_00261238[2] = DAT_00261238[2] & 0xffffbfff;
      }
    }
    else {
      uVar1 = DAT_00261238[2];
      DAT_00261238[2] = uVar1 | 0x4000;
      if ((uVar1 & 0x100) != 0) {
        puVar2[2] = uVar1 & 0xfffffeff | 0x4000;
        puVar3 = (undefined4 *)*puVar2;
        if ((undefined4 *)puVar2[1] <= puVar3) {
          puVar3 = (undefined4 *)FUN_00252040();
        }
        *puVar3 = 0x40130;
        puVar3[1] = 0;
        *puVar2 = puVar3 + 2;
      }
    }
  }
  puVar2[0x25b] = param_1;
  puVar2[0x25c] = param_2;
  FUN_00250f40();
  return;
}

