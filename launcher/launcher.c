/*
 * "Play FIFA Street.exe": starts bin\fifastreet_recomp.exe from the folder it
 * sits in, with the same settings Jogar-FIFA-Street.cmd uses, the game's
 * output in logs\, and no console window. Checks that the game files are
 * there first.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

static const char *const k_env[][2] = {
    /* 100% native: no model of any console device (NV2A, APU/DSP, AC97, USB, flash). */
    { "RECOMP_NO_HW", "1" },        { "RECOMP_HLE_PRIMARY", "1" },  { "RECOMP_HLE_NOGPU", "1" },
    { "RECOMP_DS_NATIVE", "1" },    { "RECOMP_INPUT_NATIVE", "1" }, { "RECOMP_VBLANK", "1" },
    { "RECOMP_TITLE_KEVENTS", "1" },{ "RECOMP_GUEST_LOCK", "1" },   { "RECOMP_GUEST_SMP", "1" },
    { "RECOMP_KEYBOARD", "1" },
};

static void msg(const wchar_t *text)
{
    MessageBoxW(NULL, text, L"FIFA Street", MB_ICONWARNING | MB_OK);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show)
{
    wchar_t root[MAX_PATH], path[MAX_PATH], exe[MAX_PATH], *slash;
    SECURITY_ATTRIBUTES sa = { sizeof sa, NULL, TRUE };
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    HANDLE out, err;
    DWORD code = 0;
    size_t i;
    (void)inst; (void)prev; (void)cmd; (void)show;

    GetModuleFileNameW(NULL, root, MAX_PATH);
    slash = wcsrchr(root, L'\\');
    if (slash)
        *slash = 0;
    swprintf(exe, MAX_PATH, L"%ls\\bin\\fifastreet_recomp.exe", root);
    swprintf(path, MAX_PATH, L"%ls\\game_files\\default.xbe", root);
    if (GetFileAttributesW(exe) == INVALID_FILE_ATTRIBUTES) {
        msg(L"bin\\fifastreet_recomp.exe not found. Extract the whole zip again.\n\n"
            L"bin\\fifastreet_recomp.exe não encontrado. Extraia o zip inteiro de novo.");
        return 1;
    }
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) {
        msg(L"Game files not found.\nCopy the contents of your FIFA Street (Xbox) disc into the "
            L"\"game_files\" folder (default.xbe must be at game_files\\default.xbe). See README.txt.\n\n"
            L"Arquivos do jogo não encontrados.\nCopie o conteúdo do seu disco de FIFA Street (Xbox) "
            L"para a pasta \"game_files\" (o default.xbe precisa ficar em game_files\\default.xbe). Veja LEIA-ME.txt.");
        return 1;
    }

    for (i = 0; i < sizeof k_env / sizeof k_env[0]; i++)
        SetEnvironmentVariableA(k_env[i][0], k_env[i][1]);

    swprintf(path, MAX_PATH, L"%ls\\logs", root);
    CreateDirectoryW(path, NULL);
    swprintf(path, MAX_PATH, L"%ls\\logs\\stdout.log", root);
    out = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    swprintf(path, MAX_PATH, L"%ls\\logs\\stderr.log", root);
    err = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    ZeroMemory(&si, sizeof si);
    si.cb = sizeof si;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = out;
    si.hStdError = err;
    if (!CreateProcessW(exe, NULL, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, root, &si, &pi)) {
        msg(L"Could not start the game. / Não foi possível abrir o jogo.");
        return 1;
    }
    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    if (out != INVALID_HANDLE_VALUE) CloseHandle(out);
    if (err != INVALID_HANDLE_VALUE) CloseHandle(err);
    if (code != 0)
        msg(L"The game closed with an error. The details are in the \"logs\" folder.\n\n"
            L"O jogo fechou com erro. Os detalhes estão na pasta \"logs\".");
    return (int)code;
}
