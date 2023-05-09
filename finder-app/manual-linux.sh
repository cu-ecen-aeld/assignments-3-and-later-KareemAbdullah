#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e

OUTDIR=$1

if [ -z "${OUTDIR}" ]; then
    OUTDIR=/tmp/aeld
    echo "No outdir specified, using ${OUTDIR}"
fi

#colors for debug msgs
# red=$(tput setaf 1)
# green=$(tput setaf 2)
# reset=$(tput sgr0)

KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]; then
    echo "Using default directory ${OUTDIR} for output"
else
    OUTDIR=$1
    echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
    echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
    git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    #fix kernel build issue
    sed -i '/YYLTYPE yylloc/d' scripts/dtc/dtc-lexer.l

    # Add kernel build steps here
    echo " cleaning kernel repo  "
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper # deep clean
    echo " **************** creating kernel defcofing ****************  "
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig # defconfig
    if [ $? -ne 0 ]; then
        echo "failed to create kernel defconfig "
        exit $?
    fi

    echo " **************** building kernel image****************  "
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all -j$(nproc) # vmlinux
    if [ $? -ne 0 ]; then
        echo " failed to build kernel  "
        exit $?
    fi
    # commanded for now
    # echo " **************** building kernel modules ****************  "
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules #modules
    # echo " **************** building device trees ****************  "
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs # device trees
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]; then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm -rf ${OUTDIR}/rootfs
fi

# Create necessary base directories
mkdir -p rootfs
cd rootfs
mkdir -p bin sbin dev etc home sys tmp usr var lib lib64 proc
mkdir -p usr/bin usr/sbin usr/lib var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]; then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}

else
    cd busybox

fi

if [ ! -f busybox ]; then
    #  Configure busybox
    echo " **************** cleaning busybox repo ****************  "
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} distclean # deep clean
    echo " **************** creating busybox defcofing ****************  "
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig # defconfig
    if [ $? -ne 0 ]; then
        echo " failed to create busybox defcofing  "
        exit $?
    fi
    #  Make and install busybox
    echo " **************** building busybox ****************  "
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j$(nproc)
    if [ $? -ne 0 ]; then
        echo " failed to build busybox  "
        exit $?
    fi
fi
echo " **************** installing busybox ****************  "
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
if [ $? -ne 0 ]; then
    echo " failed to install busybox  "
    exit $?
fi
# Add library dependencies to rootfs
echo "Library dependencies "
CROSS_COMPILER_PATH=$(${CROSS_COMPILE}gcc -print-sysroot)
PROG_INTERPERTER_LIB=$(${CROSS_COMPILE}readelf -a busybox | grep -oP "(?<=program interpreter: /lib/).*(?=])") # getting lib name
SHARED_LIBS=$(${CROSS_COMPILE}readelf -a busybox | grep -oP "Shared library:\K[^;]*" | tr -d "[]")
echo $PROG_INTERPERTER_LIB
for LIB_NAME in $SHARED_LIBS; do
    echo $LIB_NAME
    LIB_PATH=$(find ${CROSS_COMPILER_PATH} -iname "${LIB_NAME}")
    cp ${LIB_PATH} ${OUTDIR}/rootfs/lib64
done

PROG_INTERPERTER_LIB_PATH=$(find ${CROSS_COMPILER_PATH} -iname "${PROG_INTERPERTER_LIB}")

cp ${PROG_INTERPERTER_LIB_PATH} ${OUTDIR}/rootfs/lib/

# Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1

# Clean and build the writer utility
cd ${FINDER_APP_DIR}
echo "  **************** building writer app ****************  "
make clean
make CROSS_COMPILE=${CROSS_COMPILE}
# Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer finder.sh finder-test.sh autorun-qemu.sh ${OUTDIR}/rootfs/home
mkdir -p ${OUTDIR}/rootfs/home/conf
cp conf/username.txt conf/assignment.txt ${OUTDIR}/rootfs/home/conf

#Chown the root directory
sudo chown -R root:root ${OUTDIR}/rootfs
# Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
echo "  **************** creating initramfs.cpio.gz ****************  "
find . | cpio --create --verbose --format newc --owner root:root >${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio
