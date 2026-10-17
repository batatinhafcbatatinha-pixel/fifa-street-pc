// callers: 0x257380

void FUN_00256f90(int param_1)

{
  int iVar1;
  int *in_EAX;
  int in_ECX;
  uint in_EDX;
  int iVar2;
  float *pfVar3;
  
  iVar1 = DAT_00261238;
  iVar2 = DAT_002611d4;
  pfVar3 = (float *)(DAT_00261238 + 0xf00);
  *in_EAX = (param_1 + 0x4820) * 0x80;
  while( true ) {
    if ((in_EDX & 0xc) == 0) {
      in_EAX[1] = (int)(pfVar3[4] * *(float *)(in_ECX + 0x24));
      in_EAX[2] = (int)(pfVar3[5] * *(float *)(in_ECX + 0x28));
      in_EAX[3] = (int)(pfVar3[6] * *(float *)(in_ECX + 0x2c));
    }
    else {
      in_EAX[1] = *(int *)(in_ECX + 0x24);
      in_EAX[2] = *(int *)(in_ECX + 0x28);
      in_EAX[3] = *(int *)(in_ECX + 0x2c);
    }
    if ((in_EDX & 0x30) == 0) {
      in_EAX[4] = (int)(*(float *)(in_ECX + 4) * *pfVar3);
      in_EAX[5] = (int)(pfVar3[1] * *(float *)(in_ECX + 8));
      in_EAX[6] = (int)(pfVar3[2] * *(float *)(in_ECX + 0xc));
    }
    else {
      in_EAX[4] = *(int *)(in_ECX + 4);
      in_EAX[5] = *(int *)(in_ECX + 8);
      in_EAX[6] = *(int *)(in_ECX + 0xc);
    }
    if ((in_EDX & 0xc0) == 0) {
      in_EAX[7] = (int)(pfVar3[8] * *(float *)(in_ECX + 0x14));
      in_EAX[8] = (int)(pfVar3[9] * *(float *)(in_ECX + 0x18));
      in_EAX[9] = (int)(pfVar3[10] * *(float *)(in_ECX + 0x1c));
    }
    else {
      in_EAX[7] = *(int *)(in_ECX + 0x14);
      in_EAX[8] = *(int *)(in_ECX + 0x18);
      in_EAX[9] = *(int *)(in_ECX + 0x1c);
    }
    in_EAX = in_EAX + 10;
    if (iVar2 == 0) break;
    *in_EAX = (param_1 + 0x9030) * 0x40;
    in_EDX = in_EDX >> 8;
    pfVar3 = (float *)(iVar1 + 0xf44);
    iVar2 = iVar2 + -1;
  }
  return;
}

