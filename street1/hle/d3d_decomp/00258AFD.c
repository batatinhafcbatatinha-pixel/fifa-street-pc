// callers: 0x252f00 0x255bf0

undefined4
FUN_00258afd(uint param_1,uint param_2,int param_3,uint param_4,undefined4 param_5,
            undefined4 param_6,undefined4 param_7)

{
  uint *puVar1;
  uint uVar2;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int in_ECX;
  int iVar8;
  uint uVar9;
  undefined8 uVar10;
  uint local_1c;
  uint local_c;
  uint *local_8;
  
  uVar3 = FUN_002580b0(param_5);
  FUN_00258190(uVar3);
  local_8 = (uint *)FUN_00258a84();
  uVar4 = FUN_00258a65();
  uVar2 = *local_8;
  local_1c = 0;
  uVar5 = uVar4 & 0xff;
  local_c = param_4 & 0x60;
  uVar6 = uVar4 & 0xc00000;
  if ((char)param_4 < '\0') {
    local_c = param_4 & 0x40;
  }
  if (uVar5 == 4) {
    local_1c = uVar4 & 0xe0000;
  }
  if (param_3 == 0x32) {
    if (((param_4 & 0x200) != 0) && ((uVar4 & 0x400000) != 0)) {
      puVar1 = (uint *)(in_ECX + 0x820);
      *(undefined4 *)(in_ECX + 0x824) = 0;
      *puVar1 = 20000000;
      uVar10 = FUN_0025a0ac();
      iVar7 = (uint)uVar10 + *puVar1;
      iVar8 = (int)((ulonglong)uVar10 >> 0x20) + *(int *)(in_ECX + 0x824) +
              (uint)CARRY4((uint)uVar10,*puVar1);
      *(int *)(in_ECX + 0x828) = iVar7;
      *(int *)(in_ECX + 0x82c) = iVar8;
      FUN_0025a0df(iVar7,iVar8,1);
      uVar6 = 0x400000;
      goto LAB_00258bca;
    }
  }
  else {
    if (param_3 != 0x3c) goto LAB_00258bca;
    if (((param_4 & 0x200) == 0) || ((uVar4 & 0x800000) == 0)) {
      uVar6 = 0x400000;
      goto LAB_00258bca;
    }
    puVar1 = (uint *)(in_ECX + 0x820);
    *(undefined4 *)(in_ECX + 0x824) = 0;
    *puVar1 = 0xfe9156;
    uVar10 = FUN_0025a0ac();
    iVar7 = (uint)uVar10 + *puVar1;
    iVar8 = (int)((ulonglong)uVar10 >> 0x20) + *(int *)(in_ECX + 0x824) +
            (uint)CARRY4((uint)uVar10,*puVar1);
    *(int *)(in_ECX + 0x828) = iVar7;
    *(int *)(in_ECX + 0x82c) = iVar8;
    FUN_0025a0df(iVar7,iVar8,1);
  }
  uVar6 = 0x800000;
LAB_00258bca:
  uVar9 = 0;
  if (uVar5 == 0) {
LAB_00258bd8:
    *(undefined4 *)(in_ECX + 0xc) = uVar3;
    *(undefined4 *)(in_ECX + 4) = param_7;
    *(uint *)(in_ECX + 8) = uVar9;
    *(undefined4 *)(in_ECX + 0x7dc + *(int *)(in_ECX + 0x7e4) * 4) = 1;
    uVar2 = *local_8;
    *(undefined4 *)(in_ECX + 0x1b8) = 1;
    *(uint *)(in_ECX + 0x1b4) = uVar2;
    uVar3 = 0;
  }
  else {
    for (; uVar9 = *local_8, (uVar9 & 0xff) == (uVar2 & 0xff); local_8 = local_8 + 3) {
      if ((((((uVar5 != 4) || ((local_1c & uVar9) != 0)) || ((uVar9 & 0xe0000) == 0)) &&
           ((((ushort)local_8[1] == param_1 && (*(ushort *)((int)local_8 + 6) == param_2)) &&
            ((((~(uVar9 >> 0x10) ^ ~(param_4 >> 4)) & 1) == 0 &&
             ((((~(*local_8 >> 0x18) ^ ~(param_4 >> 7)) & 1) == 0 &&
              (((~(*local_8 >> 0x19) ^ ~(param_4 >> 8)) & 1) == 0)))))))) &&
          ((*local_8 & uVar6) != 0)) &&
         (((local_c == 0 || (((local_c & 0x20) != 0 && ((*local_8 & 0x200000) != 0)))) ||
          (((local_c & 0x40) != 0 && ((*local_8 & 0x200000) == 0)))))) {
        uVar9 = local_8[2];
        if (uVar9 != 0) {
          if ((uVar4 & uVar6) == 0) {
            uVar9 = 0;
          }
          if ((((uVar5 != 4) || ((*local_8 & 0x60000) == 0)) && ((param_4 & 0x10) != 0)) &&
             ((uVar4 & 0x10000) == 0)) {
            uVar9 = 0;
          }
          if (uVar9 != 0) goto LAB_00258bd8;
        }
        break;
      }
    }
    uVar3 = 0x80004005;
  }
  return uVar3;
}

