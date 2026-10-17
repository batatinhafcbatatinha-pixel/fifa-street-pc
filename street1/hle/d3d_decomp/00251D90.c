// callers: 0x251f00 0x2520a0 0x2520c0 0x252140 0x252c20 0x25db90

void FUN_00251d90(uint param_1,uint param_2)

{
  uint *puVar1;
  uint *puVar2;
  uint uVar3;
  undefined4 uVar4;
  int iVar5;
  uint uVar6;
  
  puVar1 = DAT_00261238;
  uVar3 = DAT_00261238[0xb];
  if (uVar3 - *(int *)DAT_00261238[0xc] <= uVar3 - param_1) {
    return;
  }
  if (param_1 == uVar3) {
    FUN_00251cd0(0);
  }
  puVar2 = (uint *)FUN_002518c0();
  if (puVar2 == (uint *)0x0) {
    return;
  }
  uVar3 = FUN_00251a40(puVar1,0);
  if (uVar3 <= *puVar1) {
    uVar3 = uVar3 + puVar1[0x11];
  }
  uVar6 = puVar2[1];
  if (uVar6 <= *puVar1) {
    uVar6 = uVar6 + puVar1[0x11];
  }
  if ((-1 < (int)(uVar6 - uVar3)) && (0x23ff < uVar6 - uVar3)) {
    if (puVar1[0x67f] != 0) {
      puVar1[0x680] = param_1;
      (*(code *)puVar1[0x67f])(param_2);
      uVar4 = FUN_00251a40(puVar1,0);
      uVar3 = FUN_00251750(uVar4);
      if (uVar3 < 0x2000) goto LAB_00251e9b;
    }
    uVar3 = puVar2[1];
    puVar1[0x774] = 0;
    if ((param_2 & 8) == 0) {
      *(undefined4 *)(uVar3 + 0x10) = 0x40110;
      *(undefined4 *)(uVar3 + 0x14) = 0;
    }
    *(undefined4 *)(uVar3 + 0x18) = 0x40100;
    *(undefined4 *)(uVar3 + 0x1c) = 5;
    FUN_00251ac0();
    uVar4 = FUN_00251a40(puVar1,1);
    uVar6 = FUN_00251750(uVar4);
    if (0x1fff < uVar6) {
      do {
        iVar5 = (*(code *)0x8000009f)(puVar1 + 0x773,6,1,0,0);
      } while (iVar5 != 0);
      return;
    }
    *(undefined4 *)(uVar3 + 0x10) = 0x40100;
    *(undefined4 *)(uVar3 + 0x14) = 0;
    *(undefined4 *)(uVar3 + 0x18) = 0x40100;
    *(undefined4 *)(uVar3 + 0x1c) = 0;
    FUN_00251ac0();
    param_1 = *puVar2;
  }
LAB_00251e9b:
  if (puVar1[0x67f] != 0) {
    puVar1[0x680] = param_1;
    (*(code *)puVar1[0x67f])(param_2 | 0x80000000);
  }
  do {
  } while (puVar1[0xb] - param_1 < puVar1[0xb] - *(int *)puVar1[0xc]);
  return;
}

