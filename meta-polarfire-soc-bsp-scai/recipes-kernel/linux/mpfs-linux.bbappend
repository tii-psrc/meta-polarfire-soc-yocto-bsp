FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# add kernel configuration fragment for MTD/UBI/UBIFS
SRC_URI:append:scai-navc = " file://qspi_flash.cfg \
                            "

COMPATIBLE_MACHINE:scai-navc = "scai-navc"
