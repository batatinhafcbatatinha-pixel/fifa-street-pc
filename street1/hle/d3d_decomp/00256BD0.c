// callers: 0x2578d0

void FUN_00256bd0(uint *param_1)

{
  uint uVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint *puVar7;
  uint *local_c;
  
  if (((*(byte *)(param_1[0x1e5] + 4) & 0x12) == 0) && ((param_1[0x64a] & 0xffffffef) != 1)) {
    uVar6 = 0x40420;
    local_c = &DAT_00260e10;
    puVar7 = param_1 + 0x343;
    do {
      uVar4 = local_c[-7];
      puVar2 = (uint *)*param_1;
      if ((uint *)param_1[1] <= puVar2) {
        puVar2 = (uint *)FUN_00252040();
      }
      if (uVar4 == 0) {
        *puVar2 = uVar6;
        puVar2[1] = 0;
        *param_1 = (uint)(puVar2 + 2);
      }
      else {
        if ((*local_c & 0xffff0000) == 0) {
          uVar3 = *(uint *)(param_1[0x1e5] + 0x10) >> ((byte)((*local_c & 0xffff) << 3) & 0x1f) &
                  0xff;
          if (uVar3 == 0) {
            uVar3 = 2;
          }
        }
        else {
          uVar3 = 3;
        }
        *puVar2 = uVar6;
        uVar4 = (uVar3 << 4 | uVar4 & 0xff) << 4 | uVar4 >> 8 & 1;
        puVar2[1] = 1;
        puVar2[2] = (0x3ff9b4 - (int)param_1) + (int)puVar7;
        if (uVar4 < 0x321) {
          if (uVar4 == 800) {
            uVar4 = puVar7[1];
            uVar3 = puVar7[-3];
            uVar5 = puVar7[-7];
            puVar2[3] = puVar7[-0xb];
            puVar2[4] = uVar5;
            puVar2[5] = uVar3;
            puVar2[6] = uVar4;
            uVar4 = puVar7[2];
            uVar3 = puVar7[-2];
            uVar5 = puVar7[-6];
            puVar2[7] = puVar7[-10];
            puVar2[8] = uVar5;
            puVar2[9] = uVar3;
            puVar2[10] = uVar4;
            puVar2[0xb] = 0;
            puVar2[0xc] = 0;
            puVar2[0xd] = 0;
            puVar2[0xe] = 0;
            goto LAB_00256f0e;
          }
                    /* WARNING (jumptable): Sanity check requires truncation of jumptable */
                    /* WARNING: Could not find normalized switch variable to match jumptable */
          switch(*(undefined1 *)(uVar4 + 0x256d44)) {
          case 0:
            uVar4 = puVar7[-3];
            uVar3 = puVar7[-7];
            puVar2[3] = puVar7[-0xb];
            puVar2[4] = uVar3;
            puVar2[5] = 0;
            puVar2[6] = uVar4;
            uVar4 = puVar7[-2];
            uVar3 = puVar7[-6];
            puVar2[7] = puVar7[-10];
            puVar2[8] = uVar3;
            puVar2[9] = 0;
            puVar2[10] = uVar4;
            puVar2[0xb] = 0;
            puVar2[0xc] = 0;
            puVar2[0xd] = 0;
            puVar2[0xe] = 0;
            goto LAB_00256f0e;
          case 1:
            uVar4 = puVar7[-3];
            uVar3 = puVar7[-7];
            puVar2[3] = puVar7[-0xb];
            puVar2[4] = uVar3;
            puVar2[5] = 0;
            puVar2[6] = uVar4;
            uVar4 = puVar7[-2];
            uVar3 = puVar7[-6];
            puVar2[7] = puVar7[-10];
            puVar2[8] = uVar3;
            puVar2[9] = 0;
            puVar2[10] = uVar4;
            uVar5 = puVar7[-9];
            uVar4 = puVar7[-1];
            uVar3 = puVar7[-5];
            puVar2[0xd] = 0;
            puVar2[0xc] = uVar3;
            goto LAB_00256f08;
          case 2:
            uVar4 = puVar7[-3];
            uVar3 = puVar7[-7];
            puVar2[3] = puVar7[-0xb];
            puVar2[4] = uVar3;
            puVar2[5] = 0;
            puVar2[6] = uVar4;
            uVar4 = puVar7[-2];
            uVar3 = puVar7[-6];
            puVar2[7] = puVar7[-10];
            puVar2[8] = uVar3;
            puVar2[9] = 0;
            puVar2[10] = uVar4;
            puVar2[0xb] = 0;
            puVar2[0xc] = 0;
            puVar2[0xd] = 0;
            puVar2[0xe] = 0;
            uVar4 = puVar7[-1];
            uVar3 = puVar7[-9];
            puVar2[0x10] = puVar7[-5];
            puVar2[0xf] = uVar3;
            puVar2[0x11] = 0;
            puVar2[0x12] = uVar4;
            break;
          case 3:
            uVar4 = puVar7[-3];
            uVar3 = puVar7[-7];
            puVar2[3] = puVar7[-0xb];
            puVar2[4] = uVar3;
            puVar2[5] = 0;
            puVar2[6] = uVar4;
            uVar4 = puVar7[-2];
            uVar3 = puVar7[-6];
            puVar2[7] = puVar7[-10];
            puVar2[8] = uVar3;
            puVar2[9] = 0;
            puVar2[10] = uVar4;
            uVar4 = puVar7[-1];
            uVar3 = puVar7[-5];
            puVar2[0xb] = puVar7[-9];
            puVar2[0xc] = uVar3;
            puVar2[0xd] = 0;
            puVar2[0xe] = uVar4;
            uVar4 = *puVar7;
            uVar3 = puVar7[-4];
            puVar2[0xf] = puVar7[-8];
            puVar2[0x10] = uVar3;
            puVar2[0x11] = 0;
            puVar2[0x12] = uVar4;
          }
        }
        else {
          uVar3 = puVar7[1];
          uVar5 = puVar7[-3];
          if (uVar4 != 0x330) {
            uVar1 = puVar7[-0xb];
            puVar2[4] = puVar7[-7];
            puVar2[3] = uVar1;
            puVar2[5] = uVar5;
            puVar2[6] = uVar3;
            uVar3 = puVar7[2];
            uVar5 = puVar7[-2];
            uVar1 = puVar7[-6];
            puVar2[7] = puVar7[-10];
            puVar2[8] = uVar1;
            puVar2[9] = uVar5;
            puVar2[10] = uVar3;
            if (uVar4 == 0x331) {
              puVar2[0xb] = 0;
              puVar2[0xc] = 0;
              puVar2[0xd] = 0;
              puVar2[0xe] = 0;
              uVar4 = puVar7[3];
              uVar3 = puVar7[-1];
              uVar5 = puVar7[-5];
              puVar2[0xf] = puVar7[-9];
              puVar2[0x10] = uVar5;
              puVar2[0x11] = uVar3;
              puVar2[0x12] = uVar4;
            }
            else {
              uVar4 = puVar7[3];
              uVar3 = puVar7[-1];
              uVar5 = puVar7[-5];
              puVar2[0xb] = puVar7[-9];
              puVar2[0xc] = uVar5;
              puVar2[0xd] = uVar3;
              puVar2[0xe] = uVar4;
              uVar4 = puVar7[4];
              uVar3 = *puVar7;
              uVar5 = puVar7[-4];
              puVar2[0xf] = puVar7[-8];
              puVar2[0x10] = uVar5;
              puVar2[0x11] = uVar3;
              puVar2[0x12] = uVar4;
            }
            goto LAB_00256f1e;
          }
          uVar4 = puVar7[-0xb];
          puVar2[4] = puVar7[-7];
          puVar2[3] = uVar4;
          puVar2[5] = uVar5;
          puVar2[6] = uVar3;
          uVar4 = puVar7[2];
          uVar3 = puVar7[-2];
          uVar5 = puVar7[-6];
          puVar2[7] = puVar7[-10];
          puVar2[8] = uVar5;
          puVar2[9] = uVar3;
          puVar2[10] = uVar4;
          uVar5 = puVar7[-9];
          uVar4 = puVar7[3];
          uVar3 = puVar7[-1];
          puVar2[0xc] = puVar7[-5];
          puVar2[0xd] = uVar3;
LAB_00256f08:
          puVar2[0xb] = uVar5;
          puVar2[0xe] = uVar4;
LAB_00256f0e:
          puVar2[0xf] = 0;
          puVar2[0x10] = 0;
          puVar2[0x11] = 0;
          puVar2[0x12] = 0x3f800000;
        }
LAB_00256f1e:
        *param_1 = (uint)(puVar2 + 0x13);
      }
      uVar6 = uVar6 + 4;
      local_c = local_c + 0x20;
      puVar7 = puVar7 + 0x10;
    } while (uVar6 < 0x40430);
  }
  return;
}

