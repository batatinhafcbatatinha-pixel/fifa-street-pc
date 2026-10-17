// callers: 0x257380

undefined4 * FUN_00257240(void)

{
  float fVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 *in_EAX;
  int iVar4;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  float local_4;
  
  iVar3 = DAT_00261238;
  iVar4 = DAT_002611d4;
  fVar1 = *(float *)(DAT_00261238 + 0xf40);
  *in_EAX = 0x1809e0;
  FUN_002522e0(fVar1,&local_14,&local_10);
  local_4 = (local_14 + 1.0) - local_10;
  FUN_002522e0(fVar1 * 0.5,&local_c,&local_8);
  in_EAX[1] = local_14;
  in_EAX[2] = local_10;
  in_EAX[6] = (local_c + 1.0) - local_8;
  in_EAX[3] = local_4;
  in_EAX[4] = local_c;
  in_EAX[5] = local_8;
  for (; puVar2 = in_EAX + 7, iVar4 != 0; iVar4 = iVar4 + -1) {
    fVar1 = *(float *)(iVar3 + 0xf84);
    *puVar2 = 0x181e28;
    FUN_002522e0(fVar1,&local_14,&local_10);
    local_4 = (local_14 + 1.0) - local_10;
    FUN_002522e0(fVar1 * 0.5,&local_c,&local_8);
    in_EAX[8] = local_14;
    in_EAX[9] = local_10;
    in_EAX[10] = local_4;
    in_EAX[0xb] = local_c;
    in_EAX[0xc] = local_8;
    in_EAX[0xd] = (local_c + 1.0) - local_8;
    in_EAX = puVar2;
  }
  return puVar2;
}

