// callers: 0x254970

void FUN_00254870(uint *param_1,uint *param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint *puVar7;
  
  uVar5 = *param_1;
  uVar1 = 0xdeadbeef;
  while (uVar5 != 0xffffffff) {
    param_1 = param_1 + 1;
    uVar2 = uVar1;
    if ((uVar5 & 0xe0000000) == 0x80000000) {
      uVar4 = uVar5 >> 0x19 & 0xf;
      uVar2 = uVar5 & 0xff;
      if ((uVar2 != uVar1 + 1) && (param_2 != (uint *)0x0)) {
        *param_2 = 0x41ea4;
        param_2[1] = uVar2;
        param_2 = param_2 + 2;
      }
      do {
        uVar5 = uVar4;
        if (7 < uVar4) {
          uVar5 = 8;
        }
        if (param_2 != (uint *)0x0) {
          *param_2 = uVar5 * 0x100000 + 0xb80;
          puVar6 = param_1;
          puVar7 = param_2 + 1;
          for (iVar3 = (uVar5 & 0xfffffff) << 2; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar7 = *puVar6;
            puVar6 = puVar6 + 1;
            puVar7 = puVar7 + 1;
          }
          for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
            *(char *)puVar7 = (char)*puVar6;
            puVar6 = (uint *)((int)puVar6 + 1);
            puVar7 = (uint *)((int)puVar7 + 1);
          }
          param_2 = param_2 + 1 + uVar5 * 4;
        }
        param_1 = param_1 + uVar5 * 4;
        uVar4 = uVar4 - uVar5;
      } while (uVar4 != 0);
    }
    uVar1 = uVar2;
    uVar5 = *param_1;
  }
  return;
}

