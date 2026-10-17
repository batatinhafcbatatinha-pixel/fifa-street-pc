// callers: 0x258afd 0x2594a0

void FUN_0025a0df(uint param_1,int param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  int *in_ECX;
  uint uVar3;
  bool bVar4;
  
  iVar1 = *in_ECX;
  if (param_3 == 0) {
    uVar3 = (uint)(in_ECX[0x211] != 0);
    in_ECX[0x211] = uVar3;
    bVar4 = CARRY4(*(uint *)(iVar1 + 0x9400),param_1);
    param_1 = *(uint *)(iVar1 + 0x9400) + param_1;
    param_2 = *(int *)(iVar1 + 0x9410) + param_2 + (uint)bVar4;
  }
  else {
    uVar3 = (uint)(in_ECX[0x211] == 0);
  }
  in_ECX[uVar3 * 2 + 0x20c] = param_1;
  in_ECX[uVar3 * 2 + 0x20d] = param_2;
  uVar3 = in_ECX[0x20d];
  uVar2 = in_ECX[0x20f];
  if (((uVar3 < uVar2) ||
      (((uVar3 == uVar2 && ((uint)in_ECX[0x20c] < (uint)in_ECX[0x20e])) ||
       (in_ECX[0x20e] == 0 && uVar2 == 0)))) && (in_ECX[0x20c] != 0 || uVar3 != 0)) {
    in_ECX[0x210] = 0;
  }
  else {
    in_ECX[0x210] = 1;
  }
  if (in_ECX[in_ECX[0x210] * 2 + 0x20c] != 0 || in_ECX[in_ECX[0x210] * 2 + 0x20d] != 0) {
    *(int *)(iVar1 + 0x9420) = in_ECX[in_ECX[0x210] * 2 + 0x20c];
    if (in_ECX[0x206] == 0) {
      *(undefined4 *)(*in_ECX + 0x9140) = 1;
      in_ECX[0x206] = 1;
    }
  }
  return;
}

