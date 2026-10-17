// callers: 0x252f00 0x255bf0

/* WARNING: Removing unreachable block (ram,0x00255713) */
/* WARNING: Removing unreachable block (ram,0x0025572b) */

undefined4 FUN_00255650(int *param_1)

{
  undefined4 uVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *in_ECX;
  int *piVar7;
  uint uVar8;
  undefined4 *puVar9;
  undefined4 *local_50;
  undefined4 local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  uint local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  uint local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  uint local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  piVar2 = param_1;
  local_44 = 1;
  if (param_1[3] != 0) {
    local_44 = param_1[3];
  }
  local_2c = FUN_002580b0(param_1[2]);
  local_1c = FUN_002580b0(param_1[9]);
  local_24 = *param_1;
  DAT_0026119c = param_1[0xc];
  local_28 = param_1[1];
  in_ECX[0x65c] = param_1[5];
  uVar8 = in_ECX[2];
  local_34 = local_44 + 1;
  in_ECX[0x684] = local_34;
  in_ECX[2] = uVar8 & 0xffffbfff;
  DAT_00261208 = 0;
  local_3c = 0;
  if (((param_1[4] & 0x3000U) != 0) || (param_1[5] == 3)) {
    local_3c = 1;
    in_ECX[2] = uVar8 & 0xffffbfff | 0x4000;
  }
  uVar8 = param_1[4];
  if (uVar8 == 0) {
    uVar8 = 0x11;
  }
  uVar3 = uVar8 >> 4 & 0xf;
  local_20 = uVar8 & 0xf;
  in_ECX[0x65b] = uVar8;
  in_ECX[0x25b] = (float)uVar3;
  in_ECX[0x25c] = (float)local_20;
  if (((uVar8 & 0x1000) != 0) && (DAT_00261208 = 2, (char)uVar8 == '!')) {
    DAT_00261208 = 1;
  }
  DAT_00261198 = 2;
  if ((uVar8 & 0x100) == 0) {
    if ((uVar8 & 0x200) != 0) {
      DAT_00261198 = 5;
    }
  }
  else {
    DAT_00261198 = 4;
  }
  piVar7 = param_1 + 0xd;
  if (*piVar7 == 0) {
    local_50 = (undefined4 *)local_2c;
    uVar8 = param_1[4] & 0xf0000;
    if (uVar8 < 0x30001) {
      if (uVar8 == 0x30000) {
        local_50 = (undefined4 *)0x1e;
      }
      else if (uVar8 == 0x10000) {
        local_50 = (undefined4 *)0x1c;
      }
      else if (uVar8 == 0x20000) {
        local_50 = (undefined4 *)0x11;
      }
    }
    else if (uVar8 == 0x40000) {
      local_50 = (undefined4 *)0x12;
    }
    iVar4 = uVar3 * local_24;
    local_20 = local_20 * local_28;
    local_40 = iVar4;
    uVar8 = FUN_002581c0(iVar4,local_50);
    local_38 = FUN_00258870(iVar4,local_20,1,1,local_50,uVar8,0,0,&local_48,&local_4c);
    if (local_3c == 0) {
      local_50 = (undefined4 *)local_34;
    }
    else {
      local_50 = (undefined4 *)0x1;
    }
    for (local_30 = (((int)local_50 * local_38 + -1 + uVar8) / uVar8) * uVar8;
        (local_30 & 0x3fff) != 0; local_30 = local_30 + uVar8) {
    }
    iVar4 = FUN_001609a7(local_30,0xbe800000);
    if (iVar4 == 0) {
      return 0x8007000e;
    }
    in_ECX[0x6a1] = iVar4;
    if (local_50 != (undefined4 *)0x0) {
      puVar6 = in_ECX + 0x689;
      local_34 = (int)local_50;
      local_50 = in_ECX + 0x685;
      do {
        *local_50 = puVar6;
        FUN_0025a830(puVar6,local_48,local_4c,iVar4);
        iVar4 = iVar4 + local_38;
        local_50 = local_50 + 1;
        puVar6 = puVar6 + 6;
        local_34 = local_34 + -1;
      } while (local_34 != 0);
    }
    if (local_40 != 0x780) {
      local_14 = in_ECX[0x6a1];
      local_18 = 0;
      local_c = 0;
      local_8 = 0;
      local_4 = 0;
      local_10 = local_30;
      local_c = FUN_00252e10(in_ECX[0x685]);
      FUN_00253c30(0,&local_18);
    }
    if (local_3c != 0) {
      local_38 = FUN_00258870(local_24,local_28,1,1,local_2c,0,0,0,&local_48,&local_4c);
      iVar4 = local_44;
      iVar5 = FUN_001609a7(local_38 * local_44,0xbe800000);
      if (iVar5 == 0) {
        return 0x8007000e;
      }
      in_ECX[0x6a2] = iVar5;
      if (iVar4 != 0) {
        puVar6 = in_ECX + 0x686;
        puVar9 = in_ECX + 0x68f;
        local_24 = local_44;
        do {
          *puVar6 = puVar9;
          FUN_0025a830(puVar9,local_48,local_4c,iVar5);
          iVar5 = iVar5 + local_38;
          puVar6 = puVar6 + 1;
          puVar9 = puVar9 + 6;
          local_24 = local_24 + -1;
        } while (local_24 != 0);
        local_24 = 0;
      }
    }
    iVar4 = local_1c;
    if (param_1[8] != 0) {
      uVar3 = FUN_002581c0(local_40,local_1c);
      iVar5 = FUN_00258870(local_40,local_20,1,1,iVar4,uVar3,0,0,&local_48,&local_4c);
      for (uVar8 = ((iVar5 + -1 + uVar3) / uVar3) * uVar3; (uVar8 & 0x3fff) != 0;
          uVar8 = uVar8 + uVar3) {
      }
      iVar5 = FUN_001609a7(uVar8,0xbe800000);
      if (iVar5 == 0) {
        return 0x8007000e;
      }
      in_ECX[0x6a3] = iVar5;
      in_ECX[0x688] = in_ECX + 0x69b;
      FUN_0025a830(in_ECX + 0x69b,local_48,local_4c,iVar5);
      if (local_40 != 0x780) {
        local_c = 0;
        local_8 = 0;
        local_4 = 0;
        local_18 = 0x80000001;
        local_14 = iVar5;
        local_10 = uVar8;
        local_c = FUN_00252e10(in_ECX[0x688]);
        if (((&DAT_0025ed88)[iVar4] & 0x3c) == 0x20) {
          local_18 = 0x84000001;
        }
        FUN_00253c30(1,&local_18);
      }
    }
  }
  else {
    if (local_34 != 0) {
      param_1 = in_ECX + 0x685;
      puVar6 = in_ECX + 0x689;
      local_38 = local_34;
      do {
        puVar9 = (undefined4 *)*piVar7;
        *puVar6 = *puVar9;
        puVar6[1] = puVar9[1];
        puVar6[2] = puVar9[2];
        puVar6[3] = puVar9[3];
        puVar6[4] = puVar9[4];
        puVar6[5] = puVar9[5];
        *param_1 = (int)puVar6;
        param_1 = param_1 + 1;
        piVar7 = piVar7 + 1;
        puVar6 = puVar6 + 6;
        local_38 = local_38 + -1;
      } while (local_38 != 0);
    }
    puVar6 = (undefined4 *)piVar2[0x10];
    if (puVar6 != (undefined4 *)0x0) {
      in_ECX[0x69b] = *puVar6;
      in_ECX[0x69c] = puVar6[1];
      in_ECX[0x69d] = puVar6[2];
      in_ECX[0x69e] = puVar6[3];
      in_ECX[0x69f] = puVar6[4];
      in_ECX[0x6a0] = puVar6[5];
      in_ECX[0x688] = in_ECX + 0x69b;
    }
  }
  uVar8 = 0;
  if (in_ECX[0x684] != 0) {
    puVar6 = in_ECX + 0x685;
    do {
      FUN_00257c60(*puVar6);
      uVar8 = uVar8 + 1;
      puVar6 = puVar6 + 1;
    } while (uVar8 < (uint)in_ECX[0x684]);
  }
  if (in_ECX[0x688] != 0) {
    FUN_00257c60(in_ECX[0x688]);
  }
  puVar6 = (undefined4 *)*in_ECX;
  if ((undefined4 *)in_ECX[1] <= puVar6) {
    puVar6 = (undefined4 *)FUN_00252040();
  }
  uVar1 = in_ECX[0x684];
  *puVar6 = 0xc0120;
  puVar6[1] = 0;
  puVar6[2] = 1;
  puVar6[3] = uVar1;
  *in_ECX = puVar6 + 4;
  return 0;
}

