DESCRIPTION = "3Dplus MTD systemd script for auto-format and auto-mount"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://3dplus-mtd.service \
	   file://3dplus-mtd.sh \
	   file://iwave_nand.conf \
	"

inherit systemd

RDEPENDS:${PN} += "bash"

S = "${WORKDIR}"

do_compile() {
	:
}

do_install() {
	install -d ${D}/opt

	install -d ${D}${systemd_system_unitdir}
	install -m 0644 ${WORKDIR}/3dplus-mtd.service ${D}${systemd_unitdir}/system

	install -d ${D}${sbindir}
	install -m 0755 ${WORKDIR}/3dplus-mtd.sh ${D}${sbindir}/

	install -d ${D}${sysconfdir}/modprobe.d
	install -m 0644 ${WORKDIR}/iwave_nand.conf ${D}${sysconfdir}/modprobe.d/iwave_nand.conf
}

SYSTEMD_SERVICE:${PN} = "3dplus-mtd.service"

SYSTEMD_AUTO_ENABLE:${PN} = "enable"

FILES:${PN} += "/opt"
FILES:${PN} += "/lib/systemd/system"
FILES:${PN} += "/usr/sbin/3dplus-mtd.sh"
FILES:${PN} += "${sysconfdir}/modprobe.d/iwave_nand.conf"

REQUIRED_DISTRO_FEATURES= "systemd"
