#!/bin/sh
set -eu

MTD_NAME_ARRAY=("iWave_nand.68000000" "iWave_nand.68800000")
VOL_NAME_ARRAY=("3dplus_mtd_a" "3dplus_mtd_b")
MOUNT_POINT_ARRAY=("/opt/scai_mtd_a" "/opt/scai_mtd_b")

log() {
    echo "[3dplus-mtd] $*"
}

find_ubi() {
    for u in /sys/class/ubi/ubi*; do
        [ -f "$u/mtd_num" ] || continue
        [ "$(cat "$u/mtd_num")" = "$MTD_NUM" ] && {
            basename "$u"
            return 0
        }
    done
    return 1
}

log "Loading iwave_nand.ko ... "
modprobe iwave_nand
log "Done."

for i in "${!MTD_NAME_ARRAY[@]}"; do
    MTD_NAME="${MTD_NAME_ARRAY[$i]}"
    VOL_NAME="${VOL_NAME_ARRAY[$i]}"
    MOUNT_POINT="${MOUNT_POINT_ARRAY[$i]}"

    log "MTD=${MTD_NAME}, VOL=${VOL_NAME}, MOUNT=${MOUNT_POINT}"

    MTD_NUM=$(awk -F: "/\"$MTD_NAME\"/ {gsub(\"mtd\",\"\",\$1); print \$1}" /proc/mtd)
    [ -z "$MTD_NUM" ] && { log "MTD not found"; continue; }

    if UBI_DEV=$(find_ubi); then
        log "Already attached: $UBI_DEV"
    else
        log "Attaching MTD $MTD_NUM"
        ubiattach /dev/ubi_ctrl -m "$MTD_NUM" || true
        sleep 1

        if ! UBI_DEV=$(find_ubi); then
            log "UBI not present -> formatting"
            ubiformat /dev/mtd$MTD_NUM -y
            ubiattach /dev/ubi_ctrl -m "$MTD_NUM" || true
            sleep 1
            UBI_DEV=$(find_ubi)
        fi
    fi

    found=0
    for v in /sys/class/ubi/${UBI_DEV}_*; do
        [ -e "$v/name" ] || continue
        if [ "$(cat "$v/name")" = "$VOL_NAME" ]; then
            log "UBI Volume(${VOL_NAME}) present."
            found=1
            break
        fi
    done

    if [ $found -eq 0 ]; then
        log "UBI Volume not present -> making volume(${VOL_NAME})"
        ubimkvol /dev/$UBI_DEV -N "$VOL_NAME" -m
    fi

    mkdir -p "$MOUNT_POINT"

    if ! mountpoint -q "$MOUNT_POINT"; then
        log "Mounting $VOL_NAME"
        mount -t ubifs $UBI_DEV:$VOL_NAME "$MOUNT_POINT"
    fi

    if [ -r "/proc/${MTD_NAME}/poll_mode" ]; then
        log "poll_mode before = $(cat /proc/${MTD_NAME}/poll_mode)"

        echo 0 > "/proc/${MTD_NAME}/poll_mode"

        log "poll_mode after = $(cat /proc/${MTD_NAME}/poll_mode)"
    fi


    log "$MTD_NAME $VOL_NAME $MOUNT_POINT Done"
done

exit 0
