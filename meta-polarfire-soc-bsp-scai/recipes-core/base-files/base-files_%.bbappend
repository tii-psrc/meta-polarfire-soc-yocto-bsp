# Install machine-specific /etc/issue file


# NAVC
FILESEXTRAPATHS:prepend:scai-navc := "${THISDIR}/${PN}/growfs:"
FILESEXTRAPATHS:prepend:scai-navc := "${THISDIR}/${PN}/scai-navc:"
SRC_URI:append:scai-navc = " file://issue"

do_install:append:scai-navc () {
    install -m 0644 ${WORKDIR}/issue ${D}${sysconfdir}/issue
}


# DPU
FILESEXTRAPATHS:prepend:scai-dpu := "${THISDIR}/${PN}/growfs:"
FILESEXTRAPATHS:prepend:scai-dpu := "${THISDIR}/${PN}/scai-dpu:"
SRC_URI:append:scai-dpu = " file://issue"

do_install:append:scai-dpu () {
    install -m 0644 ${WORKDIR}/issue ${D}${sysconfdir}/issue
}


# QEMU
FILESEXTRAPATHS:prepend:qemuriscv64 := "${THISDIR}/${PN}/qemuriscv64:"
SRC_URI:append:qemuriscv64 = " file://issue"

do_install:append:qemuriscv64 () {
    install -m 0644 ${WORKDIR}/issue ${D}${sysconfdir}/issue
}


