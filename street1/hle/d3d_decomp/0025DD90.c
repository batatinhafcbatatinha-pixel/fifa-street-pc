// callers: 0x25e4a0

void FUN_0025dd90(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 local_170 [368];
  
  iVar4 = DAT_00261238;
  FUN_0025d5d0();
  FUN_00252fb0(param_1,0);
  FUN_00253800(0,param_2);
  FUN_0025d130();
  FUN_0025d330();
  uVar3 = DAT_00260db0;
  uVar1 = *(undefined4 *)(iVar4 + 0x96c);
  uVar2 = *(undefined4 *)(iVar4 + 0x970);
  *(undefined4 *)(iVar4 + 0x96c) = param_3;
  *(undefined4 *)(iVar4 + 0x970) = param_3;
  DAT_00260db0 = 5;
  FUN_0025d6d0();
  *(undefined4 *)(iVar4 + 0x96c) = uVar1;
  *(undefined4 *)(iVar4 + 0x970) = uVar2;
  DAT_00260db0 = uVar3;
  FUN_0025d450(local_170);
  FUN_0025d660();
  return;
}

