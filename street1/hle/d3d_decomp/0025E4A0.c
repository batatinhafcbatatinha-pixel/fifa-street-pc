// callers: 0x25e820

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0025e4a0(void)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  sbyte sVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  bool bVar8;
  uint local_30;
  int local_2c;
  undefined4 local_28 [4];
  undefined1 local_18 [24];
  
  iVar3 = DAT_00261238;
  local_28[0] = 0x3f800000;
  local_28[1] = 0x40000000;
  local_28[2] = 0x40800000;
  local_28[3] = 0x41000000;
  if ((*(uint *)(DAT_00261238 + 0x8d8) & 0x20) == 0) {
    if ((*(uint *)(DAT_00261238 + 0x8d8) & 0x10) != 0) {
      if (DAT_00263a08 == 0 && DAT_00263a0c == 0) {
        FUN_0015b2ea(&DAT_00263a08);
      }
      else {
        FUN_0015b2ea(&local_30);
        iVar1 = (local_2c - DAT_00263a0c) - (uint)(local_30 < DAT_00263a08);
        if (iVar1 < DAT_00263a04) {
          return;
        }
        if ((iVar1 <= DAT_00263a04) && (local_30 - DAT_00263a08 < DAT_00263a00)) {
          return;
        }
        DAT_00263a08 = local_30;
        DAT_00263a0c = local_2c;
      }
      if ((DAT_00263a2c < '\0') && (iVar1 = FUN_0015b2ca(DAT_00263a24,1000), iVar1 == 0x102)) {
                    /* WARNING: Subroutine does not return */
        FUN_0025af90(s_p_Wait_for_image_write_timed_out_002600a6 + 2);
      }
      iVar1 = FUN_00253230(0xffffffff);
      uVar6 = *(uint *)(iVar1 + 0x10);
      uVar7 = _DAT_00263a2c & 3;
      if (uVar6 == 0) {
        uVar2 = 1 << (*(byte *)(iVar1 + 0xf) & 0xf);
        uVar6 = 1 << ((byte)(*(uint *)(iVar1 + 0xc) >> 0x14) & 0xf);
      }
      else {
        uVar2 = (uVar6 >> 0xc & 0xfff) + 1;
        uVar6 = (uVar6 & 0xfff) + 1;
      }
      sVar4 = (sbyte)uVar7;
      FUN_0025e3a0(uVar6 >> sVar4,uVar2 >> sVar4,((_DAT_00263a2c & 4) != 0) * '\x02' + '\x1c',
                   local_18,*(int *)(iVar3 + 0x8dc) + 0x40U & 0xfffffff,0);
      FUN_0025dd90(local_18,iVar1,local_28[uVar7]);
      FUN_00257ca0(iVar1);
      thunk_FUN_002520a0();
      iVar1 = *(int *)(iVar3 + 0x8dc);
      FUN_0015b2ea(iVar1);
      FUN_0015b2fb(iVar1 + 8);
      bVar8 = DAT_00263a2c < '\0';
      *(int *)(iVar1 + 0x18) = DAT_00263a20;
      if (bVar8) {
        uVar6 = DAT_00263a30 + 0x1ffU & 0xfffffe00;
        iVar3 = (*(code *)0x800000ec)
                          (DAT_00263a24,0,0,0,&DAT_00263a10,*(undefined4 *)(iVar3 + 0x8dc),uVar6,
                           &DAT_00263a18);
        if (-1 < iVar3) {
          bVar8 = CARRY4(DAT_00263a18,uVar6);
          DAT_00263a20 = DAT_00263a20 + 1;
          DAT_00263a18 = DAT_00263a18 + uVar6;
          DAT_00263a1c = DAT_00263a1c + (uint)bVar8;
          return;
        }
      }
      else if (*(int *)(iVar3 + 0x8d4) == 0) {
        *(undefined4 *)(iVar3 + 0x8d4) = 0x26000000;
        uVar6 = 0;
        do {
          FUN_0015b2dc(1);
          if (9999 < uVar6) {
            DAT_00263a20 = DAT_00263a20 + -1;
            *(undefined4 *)(iVar3 + 0x8d4) = 0;
            *(uint *)(iVar3 + 0x8d8) = *(uint *)(iVar3 + 0x8d8) & 0xffffffef;
          }
          uVar6 = uVar6 + 1;
        } while (*(int *)(iVar3 + 0x8d4) == 0x26000000);
        DAT_00263a20 = DAT_00263a20 + 1;
      }
    }
  }
  else {
    uVar6 = DAT_00263a30 + 0x1ffU & 0xfffffe00;
    iVar1 = (*(code *)0x800000db)
                      (DAT_00263a24,0,0,0,&DAT_00263a10,*(undefined4 *)(DAT_00261238 + 0x8dc),uVar6,
                       &DAT_00263a18);
    piVar5 = (int *)(*(int *)(iVar3 + 0x8dc) + 0x18);
    if (iVar1 < 0) {
      *piVar5 = 0;
    }
    else {
      iVar1 = FUN_0015b2ca(DAT_00263a24,1000);
      if (iVar1 == 0x102) {
                    /* WARNING: Subroutine does not return */
        FUN_0025af90(s_Wait_for_image_read_timed_out_002600c8);
      }
      bVar8 = CARRY4(DAT_00263a18,uVar6);
      DAT_00263a18 = DAT_00263a18 + uVar6;
      DAT_00263a1c = DAT_00263a1c + (uint)bVar8;
      DAT_00263a20 = DAT_00263a20 + -1;
      *piVar5 = DAT_00263a20;
    }
    if (*(int *)(iVar3 + 0x8d4) == 0) {
      *(undefined4 *)(iVar3 + 0x8d4) = 0x26000000;
      uVar6 = 0;
      do {
        FUN_0015b2dc(1);
        if (9999 < uVar6) {
          DAT_00263a20 = DAT_00263a20 + -1;
          *(undefined4 *)(iVar3 + 0x8d4) = 0;
          *(uint *)(iVar3 + 0x8d8) = *(uint *)(iVar3 + 0x8d8) & 0xffffffef;
        }
        uVar6 = uVar6 + 1;
      } while (*(int *)(iVar3 + 0x8d4) == 0x26000000);
      return;
    }
  }
  return;
}

