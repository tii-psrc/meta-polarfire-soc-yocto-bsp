
do_install:append:scai-dpu() {

  rm -f ${D}${nonarch_libdir}/systemd/systemd-growfs
  rm -f ${D}${systemd_system_unitdir}/systemd-growfs@.service
  rm -f ${D}${systemd_system_unitdir}/systemd-growfs-root.service

  rm -f ${D}${bindir}/systemd-repart
  rm -rf ${D}${nonarch_libdir}/systemd/repart
  rm -rf ${D}${sysconfdir}/repart.d
  rm -f ${D}${systemd_system_unitdir}/systemd-repart.service
  rm -f ${D}${systemd_system_unitdir}/sysinit.target.wants/systemd-repart.service
  rm -f ${D}${systemd_system_unitdir}/initrd-root-fs.target.wants/systemd-repart.service

  rm -f ${D}${nonarch_libdir}/systemd/systemd-makefs

}

do_install:append:scai-navc() {

  rm -f ${D}${nonarch_libdir}/systemd/systemd-growfs
  rm -f ${D}${systemd_system_unitdir}/systemd-growfs@.service
  rm -f ${D}${systemd_system_unitdir}/systemd-growfs-root.service

  rm -f ${D}${bindir}/systemd-repart
  rm -rf ${D}${nonarch_libdir}/systemd/repart
  rm -rf ${D}${sysconfdir}/repart.d
  rm -f ${D}${systemd_system_unitdir}/systemd-repart.service
  rm -f ${D}${systemd_system_unitdir}/sysinit.target.wants/systemd-repart.service
  rm -f ${D}${systemd_system_unitdir}/initrd-root-fs.target.wants/systemd-repart.service

  rm -f ${D}${nonarch_libdir}/systemd/systemd-makefs

}

