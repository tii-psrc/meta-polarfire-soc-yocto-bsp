IMAGE_NAME_SUFFIX:scai-navc = ""
IMAGE_NAME_SUFFIX:scai-dpu = ""

IMAGE_INSTALL += "lrzsz"

IMAGE_FEATURES += "ssh-server-openssh"
IMAGE_INSTALL += "mtd-utils mtd-utils-ubifs mtd-utils-tests mtd-utils-misc"

python remove_boot_dir() {
    import os, shutil, bb
    boot_dir = d.getVar('IMAGE_ROOTFS') + "/boot"
    if os.path.exists(boot_dir):
        bb.note("[INFO] Removing boot directory from rootfs (Python)")
        shutil.rmtree(boot_dir)
}
do_rootfs[postfuncs] += "remove_boot_dir"


