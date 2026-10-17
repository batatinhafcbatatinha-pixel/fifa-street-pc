// callers: 0x25db90

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0025e3f0(void)

{
  if ((((*(byte *)(DAT_00261238 + 0x8d8) & 0x10) != 0) && ((_DAT_00263a2c & 0x70) != 0)) &&
     (DAT_00263a28 == -1)) {
    DAT_00263a28 = DAT_0026119c;
    switch(_DAT_00263a2c >> 4 & 7) {
    case 1:
      DAT_0026119c = 1;
      return;
    case 2:
      DAT_0026119c = 2;
      return;
    case 3:
      DAT_0026119c = 4;
      return;
    case 4:
      DAT_0026119c = 0x80000000;
      return;
    case 5:
      DAT_0026119c = 0x80000001;
      return;
    case 6:
      DAT_0026119c = 0x80000002;
      return;
    case 7:
      DAT_0026119c = -0x7ffffffc;
    }
  }
  return;
}

