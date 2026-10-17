// callers: 0x194900 0x194d60 0x195320 0x1954e0 0x195db0 0x196480

void FUN_002539b0(int param_1,uint *param_2)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  int *piVar5;
  
  puVar4 = DAT_00261238;
  if (DAT_00261238[param_1 + 0x3e6] != 0) {
    *(uint *)(DAT_00261238[param_1 + 0x3e6] + 8) = DAT_00261238[0xb];
    puVar1 = (uint *)puVar4[param_1 + 0x3e6];
    uVar2 = *puVar1;
    *puVar1 = uVar2 - 0x80000;
    if ((uVar2 - 0x80000 & 0x78ffff) == 0) {
      FUN_00257b40(puVar1);
    }
  }
  puVar4[param_1 + 0x3e6] = (uint)param_2;
  if (param_2 != (uint *)0x0) {
    uVar2 = *param_2;
    *param_2 = uVar2 + 0x80000;
    piVar5 = (int *)*puVar4;
    if ((int *)puVar4[1] <= piVar5) {
      piVar5 = (int *)FUN_00252040();
    }
    uVar3 = param_2[1];
    *piVar5 = param_1 * 0x40 + 0x41b20;
    piVar5[1] = uVar3 | uVar2 + 0x80000 >> 0x1c;
    *puVar4 = (uint)(piVar5 + 2);
  }
  return;
}

