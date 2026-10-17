// callers: 0x255450 0x25d330 0x25d450

void FUN_00250080(int param_1,undefined4 param_2)

{
  if (param_1 < 0x5c) {
    FUN_00250050();
    (&DAT_00260fa0)[param_1] = param_2;
    return;
  }
  if (param_1 < 0x88) {
    DAT_00260d98 = DAT_00260d98 | *(uint *)("TeamHFGoalKeeperPass" + param_1 * 4 + 8);
    (&DAT_00260fa0)[param_1] = param_2;
    return;
  }
  (**(code **)(param_1 * 4 + 0x25ebc8))(param_2);
  return;
}

