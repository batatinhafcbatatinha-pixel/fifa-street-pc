// callers: 0x251a40 0x251d90

uint * FUN_002518c0(void)

{
  uint *puVar1;
  uint in_EAX;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint local_8;
  uint local_4;
  
  uVar3 = DAT_00261238[0xb];
  uVar2 = uVar3 - in_EAX;
  if (0x80 < uVar2) {
    local_4 = DAT_00261238[0xd];
    in_EAX = uVar3;
    if (uVar3 - *(int *)(DAT_00261238[0x12] + local_4 * 8) <= uVar2) {
      do {
        local_4 = local_4 + 1 & DAT_00261238[0xe];
      } while (uVar2 < uVar3 - *(int *)(DAT_00261238[0x12] + local_4 * 8));
      goto LAB_0025195c;
    }
  }
  uVar2 = in_EAX >> 1 & 0x3f;
  if (uVar2 != 0xffffffff) {
    uVar4 = DAT_00261238[uVar2 * 2 + 0x1a];
    if (uVar4 <= *DAT_00261238) {
      uVar4 = uVar4 + DAT_00261238[0x11];
    }
    uVar5 = uVar3 >> 1 & 0x3f;
    uVar3 = uVar2 + 1 & 0x3f;
    if (uVar3 != uVar5) {
      while( true ) {
        uVar6 = DAT_00261238[uVar3 * 2 + 0x1a];
        if (uVar6 <= *DAT_00261238) {
          uVar6 = uVar6 + DAT_00261238[0x11];
        }
        if (uVar6 < uVar4) break;
        uVar3 = uVar3 + 1 & 0x3f;
        if (uVar3 == uVar5) {
          return DAT_00261238 + uVar2 * 2 + 0x19;
        }
      }
      return (uint *)0x0;
    }
    return DAT_00261238 + uVar2 * 2 + 0x19;
  }
LAB_0025195c:
  uVar3 = DAT_00261238[0x12];
  local_8 = *(uint *)(uVar3 + 4 + local_4 * 8);
  puVar1 = (uint *)(uVar3 + local_4 * 8);
  if (local_8 <= *DAT_00261238) {
    local_8 = local_8 + DAT_00261238[0x11];
  }
  uVar2 = DAT_00261238[0xe];
  uVar5 = DAT_00261238[0xd] + 1 & uVar2;
  uVar4 = local_4 + 1 & uVar2;
  if (uVar4 != uVar5) {
    while( true ) {
      uVar6 = *(uint *)(uVar3 + 4 + uVar4 * 8);
      if (uVar6 <= *DAT_00261238) {
        uVar6 = uVar6 + DAT_00261238[0x11];
      }
      if (uVar6 < local_8) break;
      uVar4 = uVar4 + 1 & uVar2;
      if (uVar4 == uVar5) {
        return puVar1;
      }
    }
    return (uint *)0x0;
  }
  return puVar1;
}

