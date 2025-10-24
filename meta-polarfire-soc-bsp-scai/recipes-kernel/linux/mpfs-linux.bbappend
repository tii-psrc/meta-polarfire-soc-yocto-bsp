FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# add kernel configuration fragment for MTD/UBI/UBIFS
SRC_URI:append:scai-navc = " file://qspi_flash.cfg \
                            "

COMPATIBLE_MACHINE:scai-navc = "scai-navc"

do_configure:prepend () {
    # issue with using ${config} variable with devtool modify
    # auto generated code uses append() to put logic with incorrect path
    # then create symlink at prepend() time to avoid configure failure

    mkdir -p "${KCONFIG_CONFIG_ROOTDIR}"
    ln -sfn "${B}/.config" "${KCONFIG_CONFIG_ROOTDIR}/.config"
}
