DESCRIPTION = "Minimal initramfs image"
LICENSE = "MIT"

IMAGE_NAME_SUFFIX:scai-navc = ""
IMAGE_NAME_SUFFIX:scai-dpu = ""

inherit core-image

#INITRAMFS_MAXSIZE = 262144
# 설치용 모듈 전부 제거
INITRAMFS_SCRIPTS = "\
    initramfs-module-debug \
    initramfs-module-udev \
    initramfs-module-rootfs \
    initramfs-module-overlayroot \
"
#INITRAMFS_SCRIPTS = "\
#    initramfs-module-debug \
#    initramfs-module-udev \
#    initramfs-module-rootfs \
#    initramfs-module-exec \
#   initramfs-module-overlayroot \
#"
#IMAGE_INSTALL:append = " \
#    initramfs-framework \
#    initramfs-module-exec \
#    initramfs-module-mdev \
#    initramfs-module-udev \
#    initramfs-module-rootfs \
#    initramfs-module-debug \
#    busybox \
#"
#IMAGE_INSTALL += "mtd-utils mtd-utils-ubifs mtd-utils-tests mtd-utils-misc"
#PACKAGE_INSTALL:append = " mtd-utils mtd-utils-ubifs mtd-utils-tests mtd-utils-misc"
#PACKAGE_INSTALL_ATTEMPTONLY:remove = "mtd-utils-ubifs"

PACKAGE_INSTALL:append = " \
    mtd-utils \
    mtd-utils-ubifs \
    mtd-utils-misc \
    mtd-utils-tests \
    lrzsz \
    rsync \
"
#PACKAGE_INSTALL_ATTEMPTONLY:remove = " \
#    mtd-utils \
#    mtd-utils-ubifs \
#    mtd-utils-misc \
#    mtd-utils-tests \
#"

#BAD_RECOMMENDATIONS += "grub grub-common grub-editenv"



#IMAGE_INSTALL = "\
#    busybox \
#    udev \
#    initramfs-framework-base \
#    initramfs-module-udev \
#"
#IMAGE_INSTALL:remove = "\
#    initramfs-module-install \
#    initramfs-module-install-efi \
#    initramfs-module-setup-live \
#"


#IMAGE_LINGUAS = ""
IMAGE_FSTYPES = "cpio.gz"

#IMAGE_FEATURES = ""

INITRAMFS_IMAGE = "core-image-minimal-initramfs"
INITRAMFS_IMAGE_BUNDLE = "0"

COMPATIBLE_HOST:append = "|riscv.*-(linux.*|freebsd.*)"

#RDEPENDS:${PN}:remove = "grub"
#DEPENDS:${PN}:remove = "parted e2fsprogs-mke2fs"


DISTRO_FEATURES:append = " sysvinit"

#IMAGE_INSTALL:remove = "systemd systemd-udev grub*"
#IMAGE_INSTALL:remove = "initramfs-module-install"

#KERNEL_FIT_RAMDISK = "1"
#KERNEL_FIT_RAMDISK_IMAGE = "core-image-initramfs"
#KERNEL_FIT_RAMDISK_IMAGE_FSTYPE = "cpio.gz"

