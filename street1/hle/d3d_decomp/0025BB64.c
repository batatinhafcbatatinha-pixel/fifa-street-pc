// callers: 0x259fb1

void FUN_0025bb64(int param_1,uint param_2)

{
  int iVar1;
  int *in_ECX;
  uint uVar2;
  undefined4 uVar3;
  
  iVar1 = *in_ECX;
  uVar2 = param_2 & 0xff;
  uVar3 = 0;
  if (param_2 == 0x39) {
    uVar2 = 0x1000039;
  }
  else if (param_2 == 0x44) {
    uVar3 = 0x201;
  }
  else if (param_2 == 0x57) {
    uVar3 = 0xb00;
  }
  else if (param_2 == 0x97) {
    uVar3 = 0xa00;
  }
  *(uint *)((param_1 + 0x70000) * 0x10 + iVar1) = uVar2;
  iVar1 = param_1 * 0x10 + iVar1;
  *(undefined4 *)(iVar1 + 0x700004) = uVar3;
  *(undefined4 *)(iVar1 + 0x700008) = 0;
  *(undefined4 *)(iVar1 + 0x70000c) = 0;
  return;
}

