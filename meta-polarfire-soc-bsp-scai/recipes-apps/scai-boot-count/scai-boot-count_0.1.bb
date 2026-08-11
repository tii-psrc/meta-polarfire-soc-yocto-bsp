DESCRIPTION = "SCAI Boot Count Logger"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://scai_boot_count.c \
	   file://Makefile \
	   file://scai-boot-count-rootfs.service \
	   file://scai-boot-count-mtd-a.service \
	   file://scai-boot-count-mtd-b.service \
	"

inherit systemd

S = "${WORKDIR}"

do_compile() {
	oe_runmake
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 scai_boot_count ${D}${bindir}

	install -d ${D}${systemd_system_unitdir}
	install -m 0644 ${WORKDIR}/scai-boot-count-rootfs.service ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/scai-boot-count-mtd-a.service ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/scai-boot-count-mtd-b.service ${D}${systemd_unitdir}/system
}

SYSTEMD_SERVICE:${PN} = " \
		scai-boot-count-rootfs.service \
		scai-boot-count-mtd-a.service \
		scai-boot-count-mtd-b.service \
		"

SYSTEMD_AUTO_ENABLE:${PN} = "enable"

REQUIRED_DISTRO_FEATURES= "systemd"
