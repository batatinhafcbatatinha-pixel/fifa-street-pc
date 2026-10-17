
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_0019cf60(void)

{
  int in_ECX;
  
  *(undefined1 *)(in_ECX + 0x14) = 1;
  DAT_002f3a68 = in_ECX;
  FUN_001a0ed0("EAGL::TAR",FUN_00196270,&LAB_00196580);
  FUN_001a0ed0(&DAT_002b2af0,FUN_0018e7a0,FUN_0018e970);
  FUN_001a0ed0("Model",&LAB_00192400,&LAB_00190e50);
  FUN_001a0ed0(&DAT_002b2afc,&LAB_0018e4e0,&LAB_0018e500);
  FUN_001a0ed0("IndexPushBuffer",FUN_0018e540,FUN_0018e590);
  FUN_001a0f90("EAGL::GeoPrimState",&LAB_00193a10,&LAB_00193c30);
  FUN_001a0f90("EAGL::TAR",FUN_00195ea0,FUN_001965a0);
  FUN_001a2b90();
  FUN_00190320("&EAGL::RenderMethodConstants::gReflectionConstants",&DAT_002d4d58);
  FUN_00190320("&EAGL::RenderMethodConstants::gSkinningConstants",&DAT_002d4d68);
  FUN_00190320("&EAGL::RenderMethodConstants::gZeroOneTwoThree",&DAT_002d4d78);
  FUN_00190320("&EAGL::RenderMethodConstants::gShadowColour",&DAT_002d4d88);
  FUN_00190320("&EAGL::RenderMethodConstants::gEnvMapConstants",&DAT_002d4d98);
  _DAT_002f39d4 = FUN_0025e390(0);
  return _DAT_002f39d4 != 0;
}

