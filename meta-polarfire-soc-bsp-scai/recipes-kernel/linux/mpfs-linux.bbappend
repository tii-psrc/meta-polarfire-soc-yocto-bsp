FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:scai-navc = " file://qspi_flash.cfg \
                            "

COMPATIBLE_MACHINE:scai-navc = "scai-navc"
