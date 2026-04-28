FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# add kernel configuration fragment for MTD/UBI/UBIFS
SRC_URI:append:scai-navc = " file://qspi_flash.cfg \
                            "

SRC_URI:append:scai-dpu = " file://qspi_flash.cfg \
                            file://uio.cfg \
                            "
#                            file://iwave.cfg \

# we overwrite this variable that comes from MCHP BSP bb file
COMPATIBLE_MACHINE = "(icicle-kit|mpfs-video-kit|mpfs-disco-kit|scai-navc|scai-dpu)"




do_configure:prepend () {
    # issue with using ${config} variable with devtool modify
    # auto generated code uses append() to put logic with incorrect path
    # then create symlink at prepend() time to avoid configure failure

    mkdir -p "${KCONFIG_CONFIG_ROOTDIR}"
    ln -sfn "${B}/.config" "${KCONFIG_CONFIG_ROOTDIR}/.config"
}
