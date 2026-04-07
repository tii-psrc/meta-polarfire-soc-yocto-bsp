DESCRIPTION = "w25 rootfs image"
LICENSE = "MIT"

IMAGE_NAME_SUFFIX:scai-navc = ""
IMAGE_NAME_SUFFIX:scai-dpu = ""

inherit core-image

MPFS_MTDPARTS = "spi0.0:2m(payload),30m(ubi),2m(payload),30m(ubi),2m(payload),30m(ubi),2m(payload),30m(ubi)"

IMAGE_INSTALL = "u-boot-mchp"
IMAGE_INSTALL:remove = "kernel kernel-image"

python nuke_rootfs() {
    import os, shutil, bb

    rootfs = d.getVar('IMAGE_ROOTFS')

    for entry in os.listdir(rootfs):
        path = os.path.join(rootfs, entry)

        # /boot만 유지
        if entry == "boot":
            continue

        try:
            if os.path.islink(path):
                os.unlink(path)
            elif os.path.isdir(path):
                shutil.rmtree(path)
            else:
                os.remove(path)
        except Exception as e:
            bb.warn(f"[WARN] Failed to remove {path}: {e}")

    bb.note("[INFO] Rootfs cleaned (except /boot)")

}

copy_bundle_fitimage_to_rootfs() {
    # 1. rootfs /boot 생성
    install -d ${IMAGE_ROOTFS}/boot

    # 2. bundle fitImage 파일 경로
    FITIMAGE=fitImage-${INITRAMFS_IMAGE_NAME}-${MACHINE}

    # 3. target 파일 이름 정의
    TARGET_ROOTFS_PATH=${IMAGE_ROOTFS}/boot/

    bbnote "Copying bundled fitImage to rootfs: ${FITIMAGE} -> ${TARGET_ROOTFS_PATH}"

    # 4. 복사
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${FITIMAGE} ${TARGET_ROOTFS_PATH}

    cd ${TARGET_ROOTFS_PATH}
    ln -snf ${FITIMAGE} ${TARGET_ROOTFS_PATH}fitImage 

    install -m 0644 ${DEPLOY_DIR_IMAGE}/payload.bin ${TARGET_ROOTFS_PATH}
		sha1sum ${TARGET_ROOTFS_PATH}/payload.bin > ${TARGET_ROOTFS_PATH}/payload.bin.sha1
}

do_rootfs[postfuncs] += "nuke_rootfs copy_bundle_fitimage_to_rootfs"



IMAGE_FSTYPES:append = " ubifs ubimg mtd"
IMAGE_CLASSES:append = " image_type-ubimage image_type_mtd"

do_image_mtd[depends] += " \
    ${PN}:do_image_ubifs \
    ${PN}:do_image_ubimg \
"
