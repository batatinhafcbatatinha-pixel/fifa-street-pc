// callers: 0x259900

void FUN_00259690(uint param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int *in_ECX;
  int iVar4;
  uint uVar5;
  int iVar6;
  int local_1c;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  uVar1 = param_1 >> 5;
  iVar6 = *in_ECX;
  switch(param_1 & 0x1f) {
  case 1:
    break;
  default:
                    /* WARNING: Subroutine does not return */
    FUN_0025af90(s_GPU_generated_an_error__Try_repr_0025fde0);
  case 5:
    (*(code *)0x80000091)(in_ECX + 0x69,1,0);
    return;
  case 6:
  case 7:
    (**(code **)(iVar6 + 0x401a88))(*(undefined4 *)(iVar6 + 0x40186c));
    return;
  case 8:
    *(undefined4 *)(iVar6 + 0x400750) = 0xe00050;
    *(uint *)(iVar6 + 0x400754) = uVar1;
    *(undefined4 *)(iVar6 + 0x400750) = 0xdf0008;
    *(uint *)(iVar6 + 0x400754) = uVar1;
    return;
  case 9:
    *(undefined4 *)(*(int *)(iVar6 + 0x401a88) + iVar6) = *(undefined4 *)(iVar6 + 0x40186c);
    return;
  case 0xb:
    return;
  case 0xc:
    DAT_00261290 = DAT_00261290 + -1;
  case 0xe:
    FUN_0025afd0(uVar1 | 0x80000000);
    FUN_00251ac0();
    return;
  case 0xd:
    DAT_00261290 = DAT_00261290 + -1;
    FUN_0025afd0(&DAT_002614a4 + uVar1 * 0x14);
    FUN_00251ac0();
    return;
  }
  uVar5 = uVar1 & 7;
  iVar6 = in_ECX[0x72] + uVar5;
  iVar2 = in_ECX[0x70];
  iVar4 = iVar6 - iVar2;
  local_1c = 0;
  param_1 = 0;
  if (iVar4 < 1) {
    if (uVar5 != 0) {
      local_1c = 1 - iVar4;
    }
    iVar6 = iVar2;
    if ((uVar1 & 8) == 0) {
      iVar6 = iVar2 + 1;
    }
    iVar4 = iVar6 - iVar2;
  }
  in_ECX[0x71] = iVar6 + uVar5;
  in_ECX[0x72] = iVar6;
  if (in_ECX[0x75] != 0) {
    iVar2 = FUN_00259180();
    param_1 = (in_ECX[0x75] * iVar4 + in_ECX[0x76]) - iVar2;
    if ((int)param_1 < 0) {
      param_1 = 0;
    }
  }
  uVar3 = in_ECX[0x73] & 1;
  in_ECX[uVar3 * 3 + 0x5d] = 1;
  in_ECX[uVar3 * 3 + 0x5e] = iVar6;
  in_ECX[uVar3 * 3 + 0x5f] = uVar1 & 0xfffffff0;
  in_ECX[0x73] = in_ECX[0x73] + 1;
  FUN_002592b0();
  if ((code *)in_ECX[99] == (code *)0x0) {
    return;
  }
  local_14 = in_ECX[0x73];
  local_c = local_1c;
  local_4 = in_ECX[0x75] * uVar5;
  local_8 = param_1;
  local_10 = iVar6;
  (*(code *)in_ECX[99])(&local_14);
  return;
}

