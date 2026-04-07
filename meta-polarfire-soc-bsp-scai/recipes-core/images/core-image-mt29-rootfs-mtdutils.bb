DESCRIPTION = "main rootfs image"
LICENSE = "MIT"

IMAGE_NAME_SUFFIX:scai-navc = ""
IMAGE_NAME_SUFFIX:scai-dpu = ""

inherit core-image

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_ROOTFS_EXTRA_SPACE:append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "", d)}"

IMAGE_INSTALL += "lrzsz"

IMAGE_FEATURES += "ssh-server-openssh"
IMAGE_INSTALL += "mtd-utils mtd-utils-ubifs mtd-utils-tests mtd-utils-misc"

IMAGE_FEATURES:append = " tools-debug tools-profile"

# add monitoring tools
IMAGE_INSTALL:append = " stress-ng sysstat htop iotop btop"
# s-tui,netstat is not available on DPU

# add zmodem send/receive utils
IMAGE_INSTALL:append = " lrzsz"
# add mtd utils
IMAGE_INSTALL:append = " mtd-utils mtd-utils-ubifs mtd-utils-tests mtd-utils-misc"
# add base utils
IMAGE_INSTALL:append = " packagegroup-core-base-utils packagegroup-base"
IMAGE_INSTALl:remove = "kernel kernel-image"

MPFS_INITRAMFS_MTDPARTS = "spi0.0:4096m(ubi)"

#IMAGE_INSTALL:append = " ${PACKAGE_INSTALL}"
IMAGE_INSTALL:append = " packagegroup-core-full-cmdline packagegroup-base-extended"
EXTRA_IMAGE_FEATURES += "tools-debug tools-profile tools-sdk dev-pkgs dbg-pkgs"

python remove_boot_dir() {
    import os, shutil, bb
    boot_dir = d.getVar('IMAGE_ROOTFS') + "/boot"
    if os.path.exists(boot_dir):
        bb.note("[INFO] Removing boot directory from rootfs (Python)")
        shutil.rmtree(boot_dir)
}
do_rootfs[postfuncs] += "remove_boot_dir"

# remove unwanted image types
#IMAGE_TYPES_remove = "mtd ubimg"

MKUBIFS_ARGS = " -e 253952 -c 16060 -m 4096 -x zlib -F"
UBINIZE_ARGS = " -m 4096 -p 256KiB -s 4096"

IMAGE_FSTYPES = "ubifs rootfs-ubimg"
IMAGE_FSTYPES:remove = "mtd"

IMAGE_CLASSES:remove = " image_type-ubimage"
IMAGE_CLASSES:append = " image_type-rootfs-ubimage"





