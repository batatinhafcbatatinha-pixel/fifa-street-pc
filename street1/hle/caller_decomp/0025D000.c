
void thunk_FUN_002584e0(uint param_1,uint param_2,uint *param_3)

{
  uint uVar1;
  uint *puVar2;
  uint uVar3;
  undefined1 auStack_8 [4];
  undefined1 auStack_4 [4];
  
  puVar2 = param_3;
  uVar1 = param_1;
  *param_3 = (uint)*(byte *)(param_1 + 0xd);
  uVar3 = FUN_00257cf0(param_1);
  puVar2[1] = uVar3;
  puVar2[2] = 0;
  if (param_2 == 0) {
    if ((char)(&DAT_0025ed88)[*(byte *)(uVar1 + 0xd)] < '\0') {
      puVar2[2] = 1;
    }
    else if (((&DAT_0025ed88)[*(byte *)(uVar1 + 0xd)] & 0x40) != 0) {
      puVar2[2] = 2;
    }
  }
  if (*(int *)(uVar1 + 4) == *(int *)(*(int *)(DAT_00261238 + 0x1a14) + 4)) {
    puVar2[4] = *(uint *)(DAT_00261238 + 0x196c);
  }
  else {
    puVar2[4] = 0x11;
  }
  FUN_00258390(uVar1,param_2,&param_1,&param_3,auStack_4,auStack_8,&param_2);
  puVar2[5] = param_1;
  puVar2[6] = (uint)param_3;
  puVar2[3] = param_2;
  return;
}

