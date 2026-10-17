
void FUN_00194900(void)

{
  char cVar1;
  int *in_ECX;
  int iVar2;
  
  cVar1 = *(char *)(in_ECX[0x10] + 0x2c);
  if (DAT_002f34b0 != cVar1) {
    DAT_002f34b0 = cVar1;
    FUN_00251440(cVar1);
  }
  if (*(int *)(&DAT_002d5e60 + *in_ECX * 4) != *(int *)(in_ECX[0x10] + 0x30)) {
    *(int *)(&DAT_002d5e60 + *in_ECX * 4) = *(int *)(in_ECX[0x10] + 0x30);
    iVar2 = *in_ECX;
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)iVar2 & 0x1f);
    (&DAT_00260dc8)[iVar2 * 0x20] = *(undefined4 *)(&DAT_002d5e60 + iVar2 * 4);
  }
  if (DAT_002d5190 != '\0') {
    if ((&DAT_002f3450)[*in_ECX] != *(int *)(in_ECX[0x10] + 0x24)) {
      (&DAT_002f3450)[*in_ECX] = *(undefined4 *)(in_ECX[0x10] + 0x24);
      FUN_00253800(*in_ECX,*(undefined4 *)(in_ECX[0x10] + 0x24));
    }
  }
  iVar2 = in_ECX[0xe];
  if ((iVar2 != 0) && ((&DAT_002f3470)[*in_ECX] != iVar2)) {
    (&DAT_002f3470)[*in_ECX] = iVar2;
    FUN_002539b0(*in_ECX,in_ECX[0xe]);
  }
  if (*(int *)(&DAT_002d5da0 + *in_ECX * 4) != in_ECX[2]) {
    *(int *)(&DAT_002d5da0 + *in_ECX * 4) = in_ECX[2];
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260da0)[*in_ECX * 0x20] = in_ECX[2];
  }
  if (*(int *)(&DAT_002d5dc0 + *in_ECX * 4) != in_ECX[3]) {
    *(int *)(&DAT_002d5dc0 + *in_ECX * 4) = in_ECX[3];
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260da4)[*in_ECX * 0x20] = in_ECX[3];
  }
  if (*(int *)(&DAT_002d5de0 + *in_ECX * 4) != in_ECX[4]) {
    *(int *)(&DAT_002d5de0 + *in_ECX * 4) = in_ECX[4];
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260da8)[*in_ECX * 0x20] = in_ECX[4];
  }
  iVar2 = DAT_002d5fb0;
  if (DAT_002d5fb0 == -1) {
    if (*(int *)(&DAT_002d5e00 + *in_ECX * 4) == in_ECX[5]) goto LAB_00194afc;
    *(int *)(&DAT_002d5e00 + *in_ECX * 4) = in_ECX[5];
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260dac)[*in_ECX * 0x20] = in_ECX[5];
    iVar2 = in_ECX[5];
  }
  else {
    if (*(int *)(&DAT_002d5e00 + *in_ECX * 4) == DAT_002d5fb0) goto LAB_00194afc;
    *(int *)(&DAT_002d5e00 + *in_ECX * 4) = DAT_002d5fb0;
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260dac)[*in_ECX * 0x20] = iVar2;
  }
  DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
  (&DAT_00260db0)[*in_ECX * 0x20] = iVar2;
LAB_00194afc:
  if (*(int *)(&DAT_002d5e40 + *in_ECX * 4) != in_ECX[9]) {
    *(int *)(&DAT_002d5e40 + *in_ECX * 4) = in_ECX[9];
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260dc0)[*in_ECX * 0x20] = in_ECX[9];
  }
  iVar2 = *in_ECX * 0x10;
  if ((((*(float *)(&DAT_002f34b8 + iVar2) != (float)in_ECX[10]) ||
       (*(float *)(&DAT_002f34bc + iVar2) != (float)in_ECX[0xb])) ||
      (*(float *)(&DAT_002f34c0 + iVar2) != (float)in_ECX[0xc])) ||
     (*(float *)(&DAT_002f34c4 + iVar2) != (float)in_ECX[0xd])) {
    *(int *)(&DAT_002f34b8 + iVar2) = in_ECX[10];
    *(int *)(&DAT_002f34bc + *in_ECX * 0x10) = in_ECX[0xb];
    *(int *)(&DAT_002f34c0 + *in_ECX * 0x10) = in_ECX[0xc];
    *(int *)(&DAT_002f34c4 + *in_ECX * 0x10) = in_ECX[0xd];
    FUN_002509d0(*in_ECX,0x16,in_ECX[10]);
    FUN_002509d0(*in_ECX,0x17,in_ECX[0xb]);
    FUN_002509d0(*in_ECX,0x19,in_ECX[0xc]);
    FUN_002509d0(*in_ECX,0x18,in_ECX[0xd]);
  }
  iVar2 = *in_ECX;
  if (DAT_002f39cc == 0.0) {
    if (*(float *)(&DAT_002f3490 + iVar2 * 4) != (float)in_ECX[6]) {
      *(int *)(&DAT_002f3490 + iVar2 * 4) = in_ECX[6];
      DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
      (&DAT_00260db8)[*in_ECX * 0x20] = in_ECX[6];
    }
  }
  else if (*(float *)(&DAT_002f3490 + iVar2 * 4) != DAT_002f39cc) {
    *(float *)(&DAT_002f3490 + iVar2 * 4) = DAT_002f39cc;
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260db8)[*in_ECX * 0x20] = DAT_002f39cc;
  }
  if (*(int *)(&DAT_002d5e20 + *in_ECX * 4) != in_ECX[7]) {
    *(int *)(&DAT_002d5e20 + *in_ECX * 4) = in_ECX[7];
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)*in_ECX & 0x1f);
    (&DAT_00260db4)[*in_ECX * 0x20] = in_ECX[7];
  }
  return;
}

