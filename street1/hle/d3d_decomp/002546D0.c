// callers: 0x254970

void FUN_002546d0(int param_1,uint *param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint local_4;
  
  iVar1 = param_1;
  iVar6 = 0;
  local_4 = 0;
  do {
    while( true ) {
      while( true ) {
        do {
          while( true ) {
            while( true ) {
              uVar2 = *param_2;
              uVar3 = uVar2 & 0xe0000000;
              param_2 = param_2 + 1;
              if (uVar3 < 0x60000001) break;
              if (uVar3 != 0x80000000) {
                return;
              }
              param_2 = param_2 + (uVar2 >> 0x19 & 0xf) * 4;
            }
            if (uVar3 != 0x60000000) break;
            iVar4 = (uVar2 & 0x1f) * 0x10 + iVar1;
            *(undefined4 *)(iVar4 + 0x18) = 0;
            *(undefined4 *)(iVar4 + 0x14) = 0;
            iVar7 = ((uVar2 & 0x1f) + 2) * 0x10;
            *(undefined4 *)(iVar4 + 0x1c) = 2;
            *(byte *)(iVar4 + 0x21) = (byte)(uVar2 >> 0x14) & 0xf;
            if ((uVar2 & 0x10000000) == 0) {
              *(undefined1 *)(iVar7 + iVar1) = 1;
            }
            else {
              *(undefined1 *)(iVar7 + iVar1) = 2;
            }
          }
        } while (uVar3 == 0);
        if (uVar3 != 0x20000000) break;
        local_4 = uVar2 & 0xf;
        iVar6 = 0;
      }
      if ((uVar2 & 0x10000000) == 0) break;
      uVar3 = uVar2 >> 0x10 & 0xf;
      if ((uVar2 & 0x8000000) == 0) {
        uVar3 = uVar3 << 2;
      }
      iVar6 = iVar6 + uVar3;
    }
    param_1._2_1_ = (byte)(uVar2 >> 0x10);
    uVar3 = (uint)param_1._2_1_;
    uVar2 = uVar2 & 0x1f;
    iVar4 = uVar2 * 0x10 + iVar1;
    *(int *)(iVar4 + 0x18) = iVar6;
    *(uint *)(iVar4 + 0x14) = local_4;
    *(uint *)(iVar4 + 0x1c) = uVar3;
    *(undefined1 *)((uVar2 + 2) * 0x10 + iVar1) = 0;
    if (param_3 != 0) {
      if ((8 < uVar2) && (uVar2 < 0xd)) {
        uVar5 = (uint)(param_1._2_1_ >> 4);
        if (uVar5 == 7) {
          uVar5 = 4;
        }
        else if (uVar3 == 0x16) {
          uVar5 = 3;
        }
        *(uint *)(iVar1 + 0x10) =
             *(uint *)(iVar1 + 0x10) | uVar5 << ((char)uVar2 * '\b' + 0xb8U & 0x1f);
      }
      if (uVar2 == 3) {
        uVar2 = *(uint *)(iVar1 + 4) | 0x400;
      }
      else if (uVar2 == 4) {
        uVar2 = *(uint *)(iVar1 + 4) | 0x800;
      }
      else if (uVar2 == 7) {
        uVar2 = *(uint *)(iVar1 + 4) | 0x1000;
      }
      else {
        if (uVar2 != 8) goto LAB_002547d2;
        uVar2 = *(uint *)(iVar1 + 4) | 0x2000;
      }
      *(uint *)(iVar1 + 4) = uVar2;
    }
LAB_002547d2:
    iVar6 = iVar6 + (uint)(byte)(&DAT_0025ee84)[param_1._2_1_ >> 4] *
                    (uint)(byte)(&DAT_0025ee7c)[uVar3 & 0xf];
  } while( true );
}

