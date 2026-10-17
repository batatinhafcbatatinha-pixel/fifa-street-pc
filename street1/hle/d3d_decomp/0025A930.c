// callers: 

int FUN_0025a930(void)

{
  uint in_ECX;
  uint in_EDX;
  uint uVar1;
  uint unaff_EDI;
  
                    /* WARNING (jumptable): Sanity check requires truncation of jumptable */
                    /* WARNING: Could not find normalized switch variable to match jumptable */
  switch(unaff_EDI & 0xf) {
  case 1:
    uVar1 = 0xc;
    if ((in_EDX & 0x10) == 0) break;
  case 0:
    uVar1 = 4;
    break;
  case 2:
    if (*(int *)(&DAT_002621c8 + (in_EDX & 3) * 4) == 0) {
      uVar1 = 0xffffffff;
    }
    else {
      uVar1 = (in_EDX & 3) + 8;
    }
    break;
  case 3:
    uVar1 = 1;
    break;
  case 4:
    DAT_00261248 = DAT_00261248 | 0x40;
    uVar1 = 5;
    break;
  case 5:
    uVar1 = 0xd;
  }
  return ((in_ECX | in_EDX | unaff_EDI) >> 1 & 0x10 | ((in_ECX ^ unaff_EDI) & 0x10) << 1 |
          in_ECX & 0x40 | uVar1) << ((byte)(in_ECX >> 0xd) & 0x18);
}

