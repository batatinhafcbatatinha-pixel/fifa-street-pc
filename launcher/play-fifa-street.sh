#!/bin/sh
# FIFA Street - Linux launcher. Run it from anywhere: ./play-fifa-street.sh
cd "$(dirname "$0")"
if [ ! -f game_files/default.xbe ]; then
    echo "Game files not found: copy your FIFA Street (Xbox) disc into game_files/ (see README.txt)."
    echo "Arquivos do jogo nao encontrados: copie o disco de FIFA Street (Xbox) para game_files/ (veja LEIA-ME.txt)."
    exit 1
fi
chmod +x linux/fifastreet_recomp 2>/dev/null
for v in $(env | grep -o '^RECOMP_[A-Z0-9_]*'); do unset "$v"; done
export RECOMP_PB_EXEC=1 RECOMP_VBLANK=1 RECOMP_USB=1 RECOMP_DSP=1
export RECOMP_AUDIO_NATIVE=1 RECOMP_DS_NATIVE=1
export RECOMP_AC97_READY=1 RECOMP_TITLE_KEVENTS=1 RECOMP_GUEST_LOCK=1
export RECOMP_HLE_PRIMARY=1 RECOMP_HLE_NOGPU=1 RECOMP_KEYBOARD=1
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-none}"
mkdir -p logs
./linux/fifastreet_recomp > logs/stdout.log 2> logs/stderr.log
rc=$?
[ $rc -ne 0 ] && echo "The game closed with an error ($rc). See the logs folder. / O jogo fechou com erro. Veja a pasta logs."
exit $rc
