// callers: 

undefined4 FUN_00259190(undefined4 param_1,int *param_2)

{
  int iVar1;
  bool bVar2;
  byte bVar3;
  uint uVar4;
  
  iVar1 = *param_2;
  if ((param_2[0x28] != 0) && (*(int *)(iVar1 + 0x140) != 0)) {
    if (((param_2[2] & 0xc0000000U) == 0x80000000) && ((param_2[0x6d] & 0x200000U) != 0)) {
      bVar2 = true;
    }
    else {
      bVar2 = false;
    }
    if ((param_2[0x29] == 0) &&
       ((((param_2[0x6d] & 0x1000000U) != 0 || (bVar2)) &&
        ((*(uint *)(iVar1 + 0x100) & 0x1000000) != 0)))) {
      bVar3 = in(0x80c0);
      uVar4 = ~(uint)(bVar3 >> 5) & 1;
      param_2[0x74] = uVar4;
      if ((param_2[2] & 0xc0000000U) == 0x40000000) {
        param_2[0x74] = (uint)(uVar4 == 0);
      }
      uVar4 = param_2[0x6f] & 1;
      param_2[0x29] = 1;
      if ((param_2[uVar4 * 3 + 0x5d] != 0) && (param_2[uVar4 * 3 + 0x5e] == param_2[0x70] + 1)) {
        DAT_002639f4 = param_2[uVar4 * 3 + 0x5f];
      }
      FUN_00257eb2(DAT_002639f4);
    }
    *(undefined4 *)(iVar1 + 0x140) = 0;
    (*(code *)0x80000077)(param_2 + 0x21,0,0);
    return 1;
  }
  return 0;
}

