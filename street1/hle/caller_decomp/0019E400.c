
void FUN_0019e400(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  FUN_001a06b0(DAT_002f3a88);
  FUN_001a06e0(DAT_002f3a8c);
  FUN_001934b0();
  iVar2 = *param_1;
  FUN_00194900();
  local_10 = param_2;
  local_c = param_3;
  local_8 = 0;
  local_4 = 0;
  FUN_001a0700(0xffffffa0,&local_10,1);
  iVar1 = FUN_00251b80(iVar2);
  iVar2 = iVar2 * 4;
  FUN_001668e0(iVar1,param_1 + 2,iVar2);
  FUN_00251ba0(iVar2 + iVar1);
  return;
}

