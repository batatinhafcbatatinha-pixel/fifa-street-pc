// callers: 0x198750 0x199930

int FUN_00252f00(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = DAT_00261238;
  FUN_002520a0();
  do {
    iVar2 = FUN_00259160();
  } while (iVar2 != 0);
  FUN_00255520();
  iVar2 = FUN_00255650(param_1);
  if (iVar2 < 0) {
    FUN_00255520();
    return iVar2;
  }
  uVar3 = FUN_00252e10(*(undefined4 *)(iVar1 + 0x1a18));
  iVar2 = FUN_00258afd(*param_1,param_1[1],param_1[0xb],param_1[10],param_1[2],param_1[0xc],uVar3);
  if (-1 < iVar2) {
    FUN_00252fb0(*(undefined4 *)(iVar1 + 0x1a14),*(undefined4 *)(iVar1 + 0x1a20));
    FUN_00258d70(0,0,3,0,0x3f800000,0);
    iVar2 = 0;
  }
  return iVar2;
}

