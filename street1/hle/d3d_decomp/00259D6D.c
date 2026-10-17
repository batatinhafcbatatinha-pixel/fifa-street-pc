// callers: 0x25a1f4

undefined4 FUN_00259d6d(void)

{
  int iVar1;
  int *in_ECX;
  
  iVar1 = *in_ECX;
  *(undefined4 *)(iVar1 + 0x1830) = 0;
  *(undefined4 *)(iVar1 + 0x180c) = 0xf800;
  FUN_0025b30c();
  in_ECX[0x2d] = 1;
  *(undefined4 *)(iVar1 + 0x9200) = 0xde86;
  *(undefined4 *)(iVar1 + 0x9210) = 0x1dcd;
  *(undefined4 *)(iVar1 + 0x9420) = 0xffffffff;
  FUN_0025b473();
  FUN_0025b571();
  FUN_00259c2a(0);
  FUN_0025b640();
  in_ECX[0x4d] = 2;
  FUN_0025b676();
  FUN_0025b6a6();
  return 1;
}

