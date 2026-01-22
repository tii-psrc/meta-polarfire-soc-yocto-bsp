SUMMARY = "UIO driver for ring buffer shared memory"
DESCRIPTION = "Kernel module providing UIO access to a shared ring buffer region for FPGA-Linux communication."
SECTION = "kernel"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = " \
    file://uio_ringbuf.c \
    file://Makefile \
"

S = "${WORKDIR}"

RPROVIDES:${PN} += "kernel-module-uio-ringbuf"

FILES:${PN} += "${nonarch_base_libdir}/modules/${MODULE_TARBALL_LINK_NAME}/updates/*"