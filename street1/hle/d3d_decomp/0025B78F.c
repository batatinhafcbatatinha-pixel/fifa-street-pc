// callers: 0x25a606 0x25c6db

void FUN_0025b78f(uint param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int *in_ECX;
  uint uVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  iVar1 = *in_ECX;
  uVar2 = *(undefined4 *)(iVar1 + 0x2500);
  iVar3 = in_ECX[0x4a];
  uVar4 = *(undefined4 *)(iVar1 + 0x3200);
  uVar5 = *(undefined4 *)(iVar1 + 0x3250);
  *(undefined4 *)(iVar1 + 0x2500) = 0;
  *(undefined4 *)(iVar1 + 0x3200) = 0;
  *(undefined4 *)(iVar1 + 0x3250) = 0;
  uVar6 = *(uint *)(iVar1 + 0x3204) & 0x1f;
  puVar8 = (undefined4 *)(uVar6 * 0x40 + iVar3 + iVar1);
  *puVar8 = *(undefined4 *)(iVar1 + 0x3240);
  puVar8[1] = *(undefined4 *)(iVar1 + 0x3244);
  puVar8[2] = *(undefined4 *)(iVar1 + 0x3248);
  puVar8[3] = *(undefined4 *)(iVar1 + 0x322c);
  puVar8[4] = *(undefined4 *)(iVar1 + 0x3228);
  puVar8[5] = *(undefined4 *)(iVar1 + 0x3224);
  puVar8[6] = *(undefined4 *)(iVar1 + 0x3280);
  puVar8[7] = *(undefined4 *)(iVar1 + 0x3254);
  puVar8[8] = *(undefined4 *)(iVar1 + 0x3268);
  puVar8[9] = *(undefined4 *)(iVar1 + 0x3264);
  puVar8[10] = *(undefined4 *)(iVar1 + 0x3260);
  puVar8[0xb] = *(undefined4 *)(iVar1 + 0x326c);
  puVar8[0xc] = *(undefined4 *)(iVar1 + 0x324c);
  if ((*(uint *)(iVar1 + 0x3204) & 0x100) != 0) {
    uVar6 = 1 << (sbyte)uVar6;
    uVar7 = ~uVar6 & *(uint *)(iVar1 + 0x2508);
    if (*(int *)(iVar1 + 0x3240) != *(int *)(iVar1 + 0x3244)) {
      uVar7 = uVar7 | uVar6;
    }
    *(uint *)(iVar1 + 0x2508) = uVar7;
  }
  *(uint *)(iVar1 + 0x3204) = param_1 & 0x1f;
  uVar6 = 1 << ((byte)param_1 & 0x1f);
  if (((in_ECX[0x41] & uVar6) != 0) && (param_1 != 1)) {
    *(uint *)(iVar1 + 0x3204) = *(uint *)(iVar1 + 0x3204) | 0x100;
  }
  puVar8 = (undefined4 *)(param_1 * 0x40 + iVar3 + iVar1);
  *(undefined4 *)(iVar1 + 0x3240) = *puVar8;
  *(undefined4 *)(iVar1 + 0x3244) = puVar8[1];
  *(undefined4 *)(iVar1 + 0x3248) = puVar8[2];
  *(undefined4 *)(iVar1 + 0x322c) = puVar8[3];
  *(undefined4 *)(iVar1 + 0x3228) = puVar8[4];
  *(undefined4 *)(iVar1 + 0x3224) = puVar8[5];
  *(undefined4 *)(iVar1 + 0x3280) = puVar8[6];
  *(undefined4 *)(iVar1 + 0x3254) = puVar8[7];
  *(undefined4 *)(iVar1 + 0x3268) = puVar8[8];
  *(undefined4 *)(iVar1 + 0x3264) = puVar8[9];
  *(undefined4 *)(iVar1 + 0x3260) = puVar8[10];
  *(undefined4 *)(iVar1 + 0x326c) = puVar8[0xb];
  *(undefined4 *)(iVar1 + 0x324c) = puVar8[0xc];
  if (((in_ECX[0x41] & uVar6) != 0) && (param_1 != 1)) {
    *(undefined4 *)(iVar1 + 0x3220) = 1;
  }
  *(undefined4 *)(iVar1 + 0x204c) = 0x1fffff;
  *(undefined4 *)(iVar1 + 0x3250) = uVar5;
  *(undefined4 *)(iVar1 + 0x3200) = uVar4;
  *(undefined4 *)(iVar1 + 0x2500) = uVar2;
  return;
}

