
void FUN_001a0790(int param_1,int *param_2)

{
  int iVar1;
  
  if (((&DAT_002f49a0)[param_1] != *param_2) ||
     (*(int *)(&DAT_002f49e0 + param_1 * 4) != param_2[9])) {
    (&DAT_002f49a0)[param_1] = *param_2;
    iVar1 = param_2[9];
    *(int *)(&DAT_002f49e0 + param_1 * 4) = iVar1;
    FUN_00254ce0(param_1,(&DAT_002f49a0)[param_1],iVar1);
  }
  return;
}

