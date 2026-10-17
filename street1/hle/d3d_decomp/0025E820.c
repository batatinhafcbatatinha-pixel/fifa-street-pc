// callers: 0x25db90

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0025e820(void)

{
  uint *puVar1;
  int iVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  undefined4 uVar7;
  byte bVar8;
  uint uVar9;
  int *piVar10;
  int iVar11;
  undefined8 uVar12;
  char *pcVar13;
  undefined4 uVar14;
  undefined1 local_6d;
  uint local_6c;
  uint local_68;
  int local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined **local_54;
  undefined4 local_50;
  undefined **ppuStack_4c;
  undefined4 uStack_48;
  undefined1 local_40 [12];
  uint local_34;
  uint local_30;
  int local_28 [3];
  undefined2 local_1c;
  short local_1a;
  undefined4 local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  iVar2 = DAT_00261238;
  if ((DAT_00263a28 != -1) && ((*(byte *)(DAT_00261238 + 0x8d8) & 0x10) != 0)) {
    DAT_0026119c = DAT_00263a28;
    DAT_00263a28 = -1;
  }
  uVar9 = *(uint *)(DAT_00261238 + 0x8d4);
  uVar4 = uVar9 & 0xff000000;
  if (0x28000000 < uVar4) {
    if (uVar4 == 0x29000000) {
      *(uint *)(DAT_00261238 + 0x8d8) = *(uint *)(DAT_00261238 + 0x8d8) & 0xffffffef | 0x20;
      iVar11 = FUN_0015b2ca(DAT_00263a24,1000);
      if (iVar11 == 0x102) {
                    /* WARNING: Subroutine does not return */
        FUN_0025af90(s_p_Wait_for_image_write_timed_out_002600a6 + 2);
      }
      DAT_00263a3c = DAT_00263a3c & 0xfffffffe;
      (*(code *)0x800000bb)(DAT_00263a24);
      DAT_00263a24 = -1;
      local_50 = 0;
      uStack_48 = 0x40;
      ppuStack_4c = &PTR_DAT_002dbde0;
      iVar11 = (*(code *)0x800000be)(&DAT_00263a24,0x80110000,&local_50,&DAT_00263a10,0,0,0,1,0xc);
      if (iVar11 < 0) {
        pcVar13 = s_Unable_to_re_open_movie_cache_fi_002600e8;
LAB_0025ed4c:
                    /* WARNING: Subroutine does not return */
        FUN_0025af90(pcVar13,iVar11);
      }
      DAT_00263a18 = 0;
      DAT_00263a1c = 0;
      *(undefined4 *)(iVar2 + 0x8d4) = 0x2a000000;
    }
    else if (uVar4 == 0x41000000) {
      _DAT_00263a34 = **(int **)(DAT_00261238 + 0x8dc);
      if (_DAT_00263a34 != 1) {
        *(undefined4 *)(DAT_00261238 + 0x8d4) = 0x44000004;
        return;
      }
      _sprintf((char *)(*(int **)(DAT_00261238 + 0x8dc))[1],s__s__s_0026011c,s_1_00_5849_1_00260124,
               "Release");
      *(undefined4 *)(iVar2 + 0x8d4) = 0x43000000;
      return;
    }
    return;
  }
  if (uVar4 == 0x28000000) {
    while (uVar9 != 0) {
      FUN_0015b2dc(1);
      uVar9 = *(uint *)(iVar2 + 0x8d4);
    }
  }
  else if (uVar4 != 0) {
    if (uVar4 != 0x21000000) {
      if (uVar4 != 0x23000000) {
        return;
      }
      if ((DAT_00263a2c < '\0') && (DAT_00263a24 != 0)) {
        local_6d = 1;
        (*(code *)0x800000e2)(DAT_00263a24,&DAT_00263a10,&local_6d,1,0xd);
        (*(code *)0x800000bb)(DAT_00263a24);
        DAT_00263a24 = 0;
      }
      *(uint *)(iVar2 + 0x8d8) = *(uint *)(iVar2 + 0x8d8) & 0xffffffcf;
      *(undefined4 *)(iVar2 + 0x8d4) = 0x24000000;
      if (*(int *)(iVar2 + 0x8dc) != 0) {
        FUN_00160a47(*(int *)(iVar2 + 0x8dc),0xa1800000);
      }
      *(undefined4 *)(iVar2 + 0x8dc) = 0;
      return;
    }
    puVar1 = *(uint **)(DAT_00261238 + 0x8dc);
    _DAT_00263a2c = *puVar1;
    DAT_00263a3c = puVar1[2];
    uVar9 = puVar1[1];
    _DAT_00263a38 = puVar1[3];
    DAT_00263a28 = -1;
    FUN_0015b2fb(&local_60);
    uVar12 = __allmul(uVar9,0,local_60,local_5c);
    uVar12 = __alldiv(uVar12,1000,0);
    DAT_00263a04 = (undefined4)((ulonglong)uVar12 >> 0x20);
    DAT_00263a00 = (undefined4)uVar12;
    DAT_00263a08 = 0;
    DAT_00263a0c = 0;
    local_68 = FUN_00253230(0xffffffff);
    uVar9 = *(uint *)(local_68 + 0x10);
    local_6c = _DAT_00263a2c & 4;
    bVar8 = (byte)_DAT_00263a2c & 3;
    if (uVar9 == 0) {
      uVar4 = 1 << (*(byte *)(local_68 + 0xf) & 0xf);
      uVar9 = 1 << ((byte)(*(uint *)(local_68 + 0xc) >> 0x14) & 0xf);
    }
    else {
      uVar4 = (uVar9 >> 0xc & 0xfff) + 1;
      uVar9 = (uVar9 & 0xfff) + 1;
    }
    FUN_0025e3a0(uVar9 >> bVar8,uVar4 >> bVar8,(local_6c != 0) * '\x02' + '\x1c',local_40,0,0);
    if (local_30 == 0) {
      iVar11 = 1 << ((byte)(local_34 >> 0x18) & 0xf);
    }
    else {
      iVar11 = (local_30 >> 0xc & 0xfff) + 1;
    }
    iVar5 = FUN_00252e10(local_40);
    DAT_00263a30 = iVar5 * iVar11 + 0x40;
    local_28[0] = 0x28;
    if (local_30 == 0) {
      local_28[1] = 1 << ((byte)(local_34 >> 0x14) & 0xf);
      local_28[2] = 1 << ((byte)(local_34 >> 0x18) & 0xf);
    }
    else {
      local_28[1] = (local_30 & 0xfff) + 1;
      local_28[2] = (local_30 >> 0xc & 0xfff) + 1;
    }
    local_1c = 1;
    local_18 = 0;
    local_1a = (ushort)(local_6c != 0) * 8 + 0x10;
    local_10 = 0;
    local_c = 0;
    local_8 = 0;
    local_14 = DAT_00263a30;
    local_4 = FUN_00252e10(local_40);
    FUN_00257ca0(local_68);
    piVar6 = (int *)FUN_001609a7(iVar5 * iVar11 + 0x23fU & 0xfffffe00,0xae800000);
    *(int **)(iVar2 + 0x8dc) = piVar6;
    if (piVar6 == (int *)0x0) {
      *(undefined4 *)(iVar2 + 0x8d4) = 0x25000003;
      return;
    }
    cVar3 = DAT_00263a2c;
    piVar10 = local_28;
    for (iVar11 = 10; iVar11 != 0; iVar11 = iVar11 + -1) {
      *piVar6 = *piVar10;
      piVar10 = piVar10 + 1;
      piVar6 = piVar6 + 1;
    }
    if (cVar3 < '\0') {
      DAT_00263a24 = -1;
      iVar11 = FUN_0015d7f0(&DAT_00260174,&local_68,0,0);
      if (iVar11 == 0) {
        uVar7 = FUN_0015b30f();
                    /* WARNING: Subroutine does not return */
        FUN_0025af90(s_GetDiskFreeSpaceEx_failed_with___0026014c,uVar7);
      }
      if ((-1 < local_64) && ((0 < local_64 || (0x40000000 < local_68)))) {
        local_68 = 0x40000000;
        local_64 = 0;
      }
      uVar14 = 0;
      uVar7 = 0;
      local_58 = 0;
      local_50 = 0x40;
      local_54 = &PTR_DAT_002dbde0;
      iVar11 = (*(code *)0x800000be)
                         (&DAT_00263a24,0x40100100,&local_58,&DAT_00263a10,&local_68,0,0,0,0x48);
      if (iVar11 < 0) {
        pcVar13 = s_createfile_error___st__08x_00260130;
        goto LAB_0025ed4c;
      }
      DAT_00263a18 = 0;
      DAT_00263a1c = 0;
      FUN_0015d32d(DAT_00263a24,uVar7,uVar14,0,0);
      FUN_0015d1af(DAT_00263a24);
      FUN_0015d32d(DAT_00263a24,DAT_00263a18,DAT_00263a1c,0,0);
    }
    DAT_00263a20 = 0;
    *(uint *)(iVar2 + 0x8d8) = *(uint *)(iVar2 + 0x8d8) | 0x10;
    *(undefined4 *)(iVar2 + 0x8d4) = 0x22000000;
    return;
  }
  FUN_0025e4a0();
  return;
}

