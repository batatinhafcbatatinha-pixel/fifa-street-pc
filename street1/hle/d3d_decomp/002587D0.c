// callers: 0x25a910

void FUN_002587d0(int param_1,undefined4 param_2,undefined4 param_3,int *param_4,int *param_5,
                 uint param_6)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined1 local_4 [4];
  
  uVar4 = param_6;
  iVar2 = param_1;
  if ((param_6 & 0x20) == 0) {
    FUN_002520c0(param_1);
  }
  FUN_002585a0(iVar2,param_2,param_3,&param_6,&param_1,local_4,&param_2,&param_3);
  if ((uVar4 & 0x40) != 0) {
    param_6 = param_6 | 0xf0000000;
  }
  if (param_5 != (int *)0x0) {
    bVar1 = (&DAT_0025ed88)[*(byte *)(iVar2 + 0xd)];
    iVar2 = param_5[1];
    iVar3 = *param_5;
    *param_4 = param_1;
    param_4[1] = iVar2 * param_1 + param_6 + ((bVar1 & 0x3c) * iVar3 >> 3);
    return;
  }
  *param_4 = param_1;
  param_4[1] = param_6;
  return;
}

