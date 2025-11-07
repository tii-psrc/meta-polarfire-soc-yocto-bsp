# Install machine-specific /etc/issue file


# NAVC
FILESEXTRAPATHS:prepend:scai-navc := "${THISDIR}/${PN}/growfs:"
FILESEXTRAPATHS:prepend:scai-navc := "${THISDIR}/${PN}/scai-navc:"
SRC_URI:append:scai-navc = " file://issue-scai"

#do_install:append:scai-navc () {
#    install -m 0644 ${WORKDIR}/issue ${D}${sysconfdir}/issue
#}


# DPU
FILESEXTRAPATHS:prepend:scai-dpu := "${THISDIR}/${PN}/growfs:"
FILESEXTRAPATHS:prepend:scai-dpu := "${THISDIR}/${PN}/scai-dpu:"
SRC_URI:append:scai-dpu = " file://issue-scai"

#do_install:append:scai-dpu () {
#    install -m 0644 ${WORKDIR}/issue ${D}${sysconfdir}/issue
#}


# QEMU
FILESEXTRAPATHS:prepend:qemuriscv64 := "${THISDIR}/${PN}/growfs:"
FILESEXTRAPATHS:prepend:qemuriscv64 := "${THISDIR}/${PN}/qemuriscv64:"
SRC_URI:append:qemuriscv64 = " file://issue-scai"





do_install_myfilesissue_scai () {
	install -m 644 ${WORKDIR}/issue*  ${D}${sysconfdir}
	install -m 644 ${WORKDIR}/issue-scai  ${D}${sysconfdir}/issue
    if [ -n "${DISTRO_NAME}" ]; then
		printf "${DISTRO_NAME} " >> ${D}${sysconfdir}/issue
		printf "${DISTRO_NAME} " >> ${D}${sysconfdir}/issue.net
		if [ -n "${DISTRO_VERSION}" ]; then
			distro_version_nodate="${@d.getVar('DISTRO_VERSION').replace('snapshot-${DATE}','snapshot').replace('${DATE}','')}"
			printf "%s " $distro_version_nodate >> ${D}${sysconfdir}/issue
			printf "%s " $distro_version_nodate >> ${D}${sysconfdir}/issue.net
		fi
		printf "\\\n \\\l\n" >> ${D}${sysconfdir}/issue
		echo >> ${D}${sysconfdir}/issue
		echo "%h"    >> ${D}${sysconfdir}/issue.net
		echo >> ${D}${sysconfdir}/issue.net
 	fi
}


# Custom issue file installation for SpacecraftAI
BASEFILESISSUEINSTALL = "do_install_myfilesissue_scai"
