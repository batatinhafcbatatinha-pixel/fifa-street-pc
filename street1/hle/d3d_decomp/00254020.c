// callers: 0x2540a0

void FUN_00254020(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  iVar1 = FUN_00258870(param_1,param_2,1,1,param_3,0,0,0,&param_3,&param_2);
  uVar4 = 0x40;
  uVar3 = 0x7ffffff;
  puVar2 = (undefined4 *)(*(code *)0x800000a6)(iVar1 + 0x40,0,0x7ffffff,0x40,0x404);
  if (puVar2 == (undefined4 *)0x0) {
    return;
  }
  *puVar2 = 0x81050001;
  puVar2[1] = (uint)(puVar2 + 0x10) & 0xfffffff;
  puVar2[3] = uVar4;
  puVar2[4] = uVar3;
  puVar2[5] = 0;
  puVar2[2] = 0;
  return;
}

