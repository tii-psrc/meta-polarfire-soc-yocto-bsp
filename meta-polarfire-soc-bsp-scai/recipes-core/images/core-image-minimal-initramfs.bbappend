DESCRIPTION = "Minimal initramfs image"
LICENSE = "MIT"

IMAGE_NAME_SUFFIX:scai-navc = ""
IMAGE_NAME_SUFFIX:scai-dpu = ""

inherit core-image

#INITRAMFS_MAXSIZE = 262144
INITRAMFS_SCRIPTS = "\
    initramfs-module-debug \
    initramfs-module-udev \
    initramfs-module-rootfs \
    initramfs-module-overlayroot \
"

PACKAGE_INSTALL:append = " \
    mtd-utils \
    mtd-utils-ubifs \
    mtd-utils-misc \
    mtd-utils-tests \
    lrzsz \
    rsync \
"

IMAGE_LINGUAS = ""
IMAGE_FSTYPES = "cpio.gz"


INITRAMFS_IMAGE = "core-image-minimal-initramfs"
INITRAMFS_IMAGE_BUNDLE = "0"

COMPATIBLE_HOST:append = "|riscv.*-(linux.*|freebsd.*)"


DISTRO_FEATURES:append = " sysvinit"
