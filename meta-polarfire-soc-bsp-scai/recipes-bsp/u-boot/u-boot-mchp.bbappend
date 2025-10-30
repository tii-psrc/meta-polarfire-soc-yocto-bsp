FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# we overwrite this variable that comes from MCHP BSP bb file
COMPATIBLE_MACHINE = "(icicle-kit|mpfs-video-kit|mpfs-disco-kit|scai-navc|scai-dpu)"

SRC_URI:append:scai-navc = "file://${UBOOT_ENV}.cmd \
                             file://${MACHINE}.cfg \
                             file://uEnv.txt \
                            "

SRC_URI:append:scai-dpu   = "file://${UBOOT_ENV}.cmd \
                             file://${MACHINE}.cfg \
                             file://uEnv.txt \
                            "

do_configure:append () {
    ln -sf "${B}/${config}/.config" "${B}/.config"
}