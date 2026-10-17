// callers: 0x252fb0 0x258d70

uint FUN_00258200(int param_1,int param_2)

{
  uint uVar1;
  
                    /* WARNING (jumptable): Sanity check requires truncation of jumptable */
                    /* WARNING: Could not find normalized switch variable to match jumptable */
  switch((&DAT_002582e5)[*(byte *)(param_1 + 0xd)]) {
  case 0:
    uVar1 = 1;
    goto LAB_0025821b;
  case 1:
    uVar1 = 3;
    goto LAB_0025821b;
  case 2:
    uVar1 = 8;
    goto LAB_0025821b;
  case 3:
    uVar1 = 4;
LAB_0025821b:
    uVar1 = uVar1 | (*(uint *)(param_1 + 0xc) & 0xf00000 | 0x2000) >> 4 |
                    *(uint *)(param_1 + 0xc) & 0xf000000;
    goto LAB_00258247;
  case 4:
    uVar1 = 3;
    break;
  case 5:
    uVar1 = 8;
    break;
  case 6:
    uVar1 = 9;
    break;
  case 7:
    uVar1 = 10;
    break;
  case 8:
    uVar1 = 1;
    break;
  case 9:
    uVar1 = 4;
  }
  uVar1 = uVar1 | 0x100;
LAB_00258247:
  if (param_2 == 0) {
    if (((&DAT_0025ed88)[*(byte *)(param_1 + 0xd)] & 0x3c) == 0x20) {
switchD_00258262_caseD_2a:
      return uVar1 | 0x20;
    }
switchD_00258262_caseD_2c:
    return uVar1 | 0x10;
  }
  switch(*(undefined1 *)(param_2 + 0xd)) {
  case 0x2a:
  case 0x2b:
  case 0x2e:
  case 0x2f:
    goto switchD_00258262_caseD_2a;
  case 0x2c:
  case 0x2d:
  case 0x30:
  case 0x31:
    goto switchD_00258262_caseD_2c;
  default:
    return uVar1;
  }
}

