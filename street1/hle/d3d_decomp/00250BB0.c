// callers: 0x25dec0

void FUN_00250bb0(void)

{
  uint *puVar1;
  int in_ECX;
  uint uVar2;
  uint *in_EDX;
  uint *puVar3;
  uint *puVar4;
  
  uVar2 = 0x181b68;
  puVar4 = (uint *)(&DAT_00260df8 + (uint)(*(int *)(in_ECX + 0x784) != 0) * 0x80);
  puVar3 = in_EDX + 1;
  do {
    *in_EDX = uVar2;
    *puVar3 = *puVar4;
    puVar3[1] = puVar4[1];
    puVar3[2] = puVar4[2];
    puVar3[3] = puVar4[3];
    puVar3[4] = puVar4[4];
    puVar1 = puVar4 + 5;
    uVar2 = uVar2 + 0x40;
    in_EDX = in_EDX + 7;
    puVar4 = puVar4 + 0x20;
    puVar3[5] = *puVar1;
    puVar3 = puVar3 + 7;
  } while (uVar2 < 0x181be9);
  return;
}

