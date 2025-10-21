FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

COMPATIBLE_MACHINE:scai-navc = "scai-navc"

SRC_URI:append:scai-navc = "file://${UBOOT_ENV}.cmd \
                             file://${MACHINE}.cfg \
                             file://uEnv.txt \
                            "

COMPATIBLE_MACHINE:scai-dpu = "scai-dpu"

SRC_URI:append:scai-dpu   = "file://${UBOOT_ENV}.cmd \
                             file://${MACHINE}.cfg \
                             file://uEnv.txt \
                            "

do_configure:append () {
    ln -sf "${B}/${config}/.config" "${B}/.config"
}