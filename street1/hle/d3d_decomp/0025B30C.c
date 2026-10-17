// callers: 0x259d6d

void FUN_0025b30c(void)

{
  int *piVar1;
  byte bVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int *in_ECX;
  uint uVar6;
  
  iVar5 = *in_ECX;
  piVar1 = (int *)(iVar5 + 0x200);
  iVar4 = *piVar1;
  in_ECX[0x55] = iVar4;
  if (iVar4 == 0) {
    *piVar1 = 0;
  }
  in_ECX[0x56] = *(int *)(iVar5 + 0x140);
  *piVar1 = -1;
  uVar6 = *(uint *)(iVar5 + 0x680504) & 0xff;
  bVar2 = *(byte *)(iVar5 + 0x680505);
  uVar3 = *(uint *)(iVar5 + 0x680504);
  in_ECX[0x34] = uVar6;
  uVar3 = uVar3 >> 0x10 & 7;
  in_ECX[0x35] = (uint)bVar2;
  in_ECX[0x36] = 1;
  in_ECX[0x37] = uVar3;
  if (uVar6 == 0) {
    iVar4 = 0;
  }
  else {
    iVar4 = (int)(((ulonglong)(in_ECX[0x2f] * (uint)bVar2) / (ulonglong)(uint)(1 << (sbyte)uVar3)) /
                 (ulonglong)uVar6);
  }
  in_ECX[0x31] = iVar4;
  uVar6 = *(uint *)(iVar5 + 0x680508) & 0xff;
  bVar2 = *(byte *)(iVar5 + 0x680509);
  uVar3 = *(uint *)(iVar5 + 0x680508) >> 0x10 & 7;
  in_ECX[0x38] = uVar6;
  in_ECX[0x39] = (uint)bVar2;
  in_ECX[0x3a] = 1;
  in_ECX[0x3b] = uVar3;
  if (uVar6 == 0) {
    iVar4 = 0;
  }
  else {
    iVar4 = (int)(((ulonglong)(in_ECX[0x2f] * (uint)bVar2) / (ulonglong)(uint)(1 << (sbyte)uVar3)) /
                 (ulonglong)uVar6);
  }
  in_ECX[0x32] = iVar4;
  bVar2 = *(byte *)(iVar5 + 0x680501);
  uVar6 = *(uint *)(iVar5 + 0x680500) & 0xff;
  uVar3 = *(uint *)(iVar5 + 0x680500) >> 0x10 & 7;
  in_ECX[0x3c] = uVar6;
  in_ECX[0x3d] = (uint)bVar2;
  in_ECX[0x3e] = 1;
  in_ECX[0x3f] = uVar3;
  if (uVar6 == 0) {
    iVar5 = 0;
  }
  else {
    iVar5 = (int)(((ulonglong)(in_ECX[0x2f] * (uint)bVar2) / (ulonglong)(uint)(1 << (sbyte)uVar3)) /
                 (ulonglong)uVar6);
  }
  in_ECX[0x33] = iVar5;
  return;
}

