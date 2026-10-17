// callers: 0x25d850 0x25dd90

void FUN_0025d6d0(void)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int in_EAX;
  int iVar4;
  int iVar5;
  undefined4 local_14;
  undefined4 local_10;
  
  fVar1 = *(float *)(in_EAX + 0x970);
  fVar2 = *(float *)(in_EAX + 0x96c);
  iVar4 = *(int *)(in_EAX + 0x1a04);
  if (*(uint *)(iVar4 + 0x10) == 0) {
    iVar5 = 1 << ((byte)(*(uint *)(iVar4 + 0xc) >> 0x14) & 0xf);
  }
  else {
    iVar5 = (*(uint *)(iVar4 + 0x10) & 0xfff) + 1;
  }
  if (*(uint *)(iVar4 + 0x10) == 0) {
    iVar4 = 1 << (*(byte *)(iVar4 + 0xf) & 0xf);
  }
  else {
    iVar4 = (*(uint *)(iVar4 + 0x10) >> 0xc & 0xfff) + 1;
  }
  local_14 = 0.0;
  local_10 = 0.0;
  if ((DAT_00260db0 == 4) || (DAT_00260db0 == 5)) {
    if (fVar2 == 2.0) {
      local_14 = 0.5;
    }
    if (fVar1 == 2.0) {
      local_10 = 0.5;
    }
  }
  FUN_0025ce60(5);
  FUN_0025cd70(9,local_14,local_10);
  FUN_0025cd70(0,0,0);
  fVar3 = (float)iVar5;
  if (iVar5 < 0) {
    fVar3 = fVar3 + 4.2949673e+09;
  }
  FUN_0025cd70(9,fVar3 * fVar2 * 4.0 + local_14,local_10);
  FUN_0025cd70(0,fVar3 * 4.0,0);
  fVar2 = (float)iVar4;
  if (iVar4 < 0) {
    fVar2 = fVar2 + 4.2949673e+09;
  }
  FUN_0025cd70(9,local_14,fVar2 * fVar1 * 4.0 + local_10);
  FUN_0025cd70(0,0,fVar2 * 4.0);
  FUN_0025cea0();
  return;
}

