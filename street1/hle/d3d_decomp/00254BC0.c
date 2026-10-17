// callers: 0x254ca0

void FUN_00254bc0(uint param_1)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  uint in_ECX;
  uint *in_EDX;
  
  if (DAT_00261244 <= DAT_00261240) {
    DAT_00261240 = (uint *)FUN_00252040();
  }
  *DAT_00261240 = 0x41ea4;
  DAT_00261240[1] = in_ECX;
  DAT_00261240 = DAT_00261240 + 2;
  if (param_1 < 0x10) {
LAB_00254beb:
    *DAT_00261240 = param_1 << 0x12 | 0xb80;
    for (; DAT_00261240 = DAT_00261240 + 1, param_1 != 0; param_1 = param_1 - 1) {
      *DAT_00261240 = *in_EDX;
      in_EDX = in_EDX + 1;
    }
  }
  else {
    while( true ) {
      uVar1 = *(undefined8 *)in_EDX;
      uVar2 = *(undefined8 *)(in_EDX + 2);
      uVar3 = *(undefined8 *)(in_EDX + 4);
      uVar4 = *(undefined8 *)(in_EDX + 6);
      uVar5 = *(undefined8 *)(in_EDX + 8);
      uVar6 = *(undefined8 *)(in_EDX + 10);
      uVar7 = *(undefined8 *)(in_EDX + 0xc);
      uVar8 = *(undefined8 *)(in_EDX + 0xe);
      *DAT_00261240 = 0x400b80;
      *(undefined8 *)(DAT_00261240 + 1) = uVar1;
      *(undefined8 *)(DAT_00261240 + 3) = uVar2;
      *(undefined8 *)(DAT_00261240 + 5) = uVar3;
      *(undefined8 *)(DAT_00261240 + 7) = uVar4;
      *(undefined8 *)(DAT_00261240 + 9) = uVar5;
      *(undefined8 *)(DAT_00261240 + 0xb) = uVar6;
      *(undefined8 *)(DAT_00261240 + 0xd) = uVar7;
      *(undefined8 *)(DAT_00261240 + 0xf) = uVar8;
      DAT_00261240 = DAT_00261240 + 0x11;
      param_1 = param_1 - 0x10;
      if (param_1 == 0) break;
      in_EDX = in_EDX + 0x10;
      if (param_1 < 0x10) goto LAB_00254beb;
      if (DAT_00261244 <= DAT_00261240) {
        DAT_00261240 = (uint *)FUN_00252040();
      }
    }
  }
  return;
}

