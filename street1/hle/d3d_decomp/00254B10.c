// callers: 0x1a0700

void FUN_00254b10(void)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  int in_ECX;
  int iVar11;
  undefined8 *in_EDX;
  
  while( true ) {
    puVar9 = DAT_00261240;
    if (DAT_00261240 + 0x13 < DAT_00261244) break;
    FUN_00252040();
  }
  uVar1 = *in_EDX;
  uVar2 = in_EDX[1];
  uVar3 = in_EDX[2];
  uVar4 = in_EDX[3];
  uVar5 = in_EDX[4];
  uVar6 = in_EDX[5];
  uVar7 = in_EDX[6];
  uVar8 = in_EDX[7];
  puVar10 = DAT_00261240 + 0x13;
  *DAT_00261240 = 0x41ea4;
  DAT_00261240 = puVar10;
  puVar9[1] = in_ECX;
  iVar11 = in_ECX * 0x10;
  puVar9[2] = 0x400b80;
  *(undefined8 *)(puVar9 + 3) = uVar1;
  *(undefined8 *)(&DAT_00260180 + iVar11) = uVar1;
  *(undefined8 *)(puVar9 + 5) = uVar2;
  *(undefined8 *)(&DAT_00260188 + iVar11) = uVar2;
  *(undefined8 *)(puVar9 + 7) = uVar3;
  *(undefined8 *)(&DAT_00260190 + iVar11) = uVar3;
  *(undefined8 *)(puVar9 + 9) = uVar4;
  *(undefined8 *)(&DAT_00260198 + iVar11) = uVar4;
  *(undefined8 *)(puVar9 + 0xb) = uVar5;
  *(undefined8 *)(&DAT_002601a0 + iVar11) = uVar5;
  *(undefined8 *)(puVar9 + 0xd) = uVar6;
  *(undefined8 *)(&DAT_002601a8 + iVar11) = uVar6;
  *(undefined8 *)(puVar9 + 0xf) = uVar7;
  *(undefined8 *)(&DAT_002601b0 + iVar11) = uVar7;
  *(undefined8 *)(puVar9 + 0x11) = uVar8;
  *(undefined8 *)(&DAT_002601b8 + iVar11) = uVar8;
  return;
}

