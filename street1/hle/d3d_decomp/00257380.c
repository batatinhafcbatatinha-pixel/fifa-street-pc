// callers: 0x2578d0

void FUN_00257380(undefined4 *param_1,uint param_2)

{
  int *piVar1;
  undefined4 *puVar2;
  uint *puVar3;
  uint *puVar4;
  uint uVar5;
  uint uVar6;
  float10 fVar7;
  int *local_2c;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  uint local_c;
  uint local_8;
  uint local_4;
  
  puVar2 = param_1;
  puVar3 = (uint *)*param_1;
  if ((uint *)param_1[1] <= puVar3) {
    puVar3 = (uint *)FUN_00252040();
  }
  if ((param_2 & 0x1000) != 0) {
    if ((DAT_0026113c == 0) && ((*(byte *)(param_1 + 2) & 0x40) == 0)) {
      uVar5 = 0;
    }
    else {
      uVar5 = 1;
    }
    if ((((*(byte *)(param_1[0x1e5] + 4) & 0x12) == 0) && (DAT_00261138 != 0)) &&
       ((param_1[0x64a] & 0xffffffef) != 1)) {
      uVar6 = 1;
      if (uVar5 != 0) {
        if ((DAT_00261140 != 0) && (param_1[499] != 0)) {
          uVar6 = 0x10001;
        }
        puVar3 = (uint *)FUN_00257240();
      }
      *puVar3 = 0x40294;
      puVar3[1] = uVar6;
      puVar3[2] = 0x40314;
      puVar3[3] = 1;
      puVar3[4] = 0x403b8;
      puVar3[5] = 1;
      uVar5 = FUN_00256270();
      param_1[500] = uVar5;
      puVar3[6] = 0x40298;
      puVar3[7] = uVar5;
      puVar3 = (uint *)FUN_00257080(uVar5);
    }
    else {
      *puVar3 = 0x40314;
      puVar3[1] = 0;
      puVar3[2] = 0x403b8;
      puVar3[3] = uVar5;
      puVar3[4] = 0x40294;
      puVar3[5] = 0x20001;
      puVar3 = puVar3 + 6;
    }
  }
  if ((param_2 & 0xff8000) != 0) {
    uVar5 = param_1[499];
    *puVar3 = 0x403bc;
    puVar3[1] = uVar5;
    puVar3 = puVar3 + 2;
    local_2c = param_1 + 0x1eb;
    param_1 = (undefined4 *)0x0;
    uVar5 = 0x41024;
    do {
      piVar1 = (int *)*local_2c;
      if (piVar1 != (int *)0x0) {
        if ((param_2 & 0x10000 << ((byte)param_1 & 0x1f)) == 0) {
          if ((char)(param_2 >> 8) < '\0') {
            *puVar2 = puVar3;
            if ((uint *)puVar2[1] <= puVar3) {
              FUN_00252040();
            }
            puVar3 = (uint *)FUN_00256f90(param_1);
          }
        }
        else {
          *puVar2 = puVar3;
          if ((uint *)puVar2[1] <= puVar3) {
            FUN_00252040();
          }
          puVar4 = (uint *)FUN_00256f90(param_1);
          if (*piVar1 == 3) {
            FUN_00252180(&local_24,piVar1 + 0x1b,0,puVar2 + 0x318);
            FUN_00252410(&local_24);
            local_10 = local_1c - 1.0;
            local_18 = local_24;
            local_14 = local_20;
            FUN_00252410(&local_18);
            *puVar4 = uVar5;
            puVar4[1] = 0x7149f2ca;
            puVar4[2] = uVar5 + 0x140004;
            puVar4[3] = (uint)local_18;
            puVar4[4] = (uint)local_14;
            puVar4[5] = (uint)local_10;
            puVar4[6] = (uint)local_24;
            puVar4[7] = (uint)local_20;
            puVar4[8] = (uint)local_1c;
            puVar3 = puVar4 + 9;
          }
          else {
            puVar4[1] = piVar1[0x13];
            *puVar4 = uVar5;
            FUN_00252180(&local_c,piVar1 + 0xd,0x3f800000,puVar2 + 0x318);
            puVar4[2] = uVar5 + 0x140038;
            puVar4[3] = local_c;
            puVar4[4] = local_8;
            puVar4[5] = local_4;
            puVar4[6] = piVar1[0x15];
            puVar4[7] = piVar1[0x16];
            puVar4[8] = piVar1[0x17];
            puVar3 = puVar4 + 9;
            if (*piVar1 == 2) {
              FUN_00252180(&local_24,piVar1 + 0x1b,0,puVar2 + 0x318);
              fVar7 = (float10)FUN_002523b0(local_24 * local_24 +
                                            local_20 * local_20 + local_1c * local_1c);
              fVar7 = fVar7 * (float10)(float)piVar1[0x21];
              *puVar3 = uVar5 + 0x18001c;
              puVar4[10] = piVar1[0x1e];
              puVar4[0xb] = piVar1[0x1f];
              puVar4[0xc] = piVar1[0x20];
              puVar3 = puVar4 + 0x11;
              puVar4[0xd] = (uint)(float)((float10)local_24 * fVar7);
              puVar4[0xe] = (uint)(float)((float10)local_20 * fVar7);
              puVar4[0xf] = (uint)(float)(fVar7 * (float10)local_1c);
              puVar4[0x10] = piVar1[0x22];
            }
          }
        }
      }
      param_1 = (undefined4 *)((int)param_1 + 1);
      local_2c = local_2c + 1;
      uVar5 = uVar5 + 0x80;
    } while (uVar5 < 0x41424);
  }
  *puVar2 = puVar3;
  return;
}

