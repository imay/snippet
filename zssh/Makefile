BUILD_DIR = ${CURDIR}/build
BUILD_SRC_DIR = ${CURDIR}/build/src
BUILD_LIB_DIR = ${CURDIR}/build/lib
BUILD_LIB_TMP_DIR = ${CURDIR}/build/lib/tmp
LIB_INSTALL_DIR = ${CURDIR}/build/install
OUTPUT_DIR = ${CURDIR}/output
OUTPUT_BIN_DIR = ${CURDIR}/output/bin
LIB_DIR = ${CURDIR}/lib
SRC_DIR = ${CURDIR}/src

CCFLAGS = -I${LIB_INSTALL_DIR}/include

all: zssh
.PHONY: all

SSH_STATIC_LIB = ${LIB_INSTALL_DIR}/lib/libssh.a
SSH_TAR = ${LIB_DIR}/libssh-0.7.0.tar.xz
libssh: ${SSH_STATIC_LIB}
.PHONY: libssh

${BUILD_LIB_DIR}:
	mkdir -p $@

${SSH_STATIC_LIB}: ${SSH_TAR} | ${BUILD_LIB_DIR}
	tar -C ${BUILD_LIB_DIR} -xmf $<
	mkdir -p ${BUILD_LIB_TMP_DIR}/libssh
	cd ${BUILD_LIB_TMP_DIR}/libssh && cmake ${BUILD_LIB_DIR}/libssh-0.7.0/ -DCMAKE_INSTALL_PREFIX=${LIB_INSTALL_DIR} -DWITH_STATIC_LIB=ON -DWITH_ZLIB=OFF -DWITH_GSSAPI=OFF -DWITH_SERVER=OFF -DWITH_PCAP=OFF
	cd ${BUILD_LIB_TMP_DIR}/libssh && make -j8 && make install

ZSSH_BINARY = ${OUTPUT_BIN_DIR}/zssh
zssh: ${ZSSH_BINARY}
.PHONY: zssh

${OUTPUT_BIN_DIR}:
	mkdir -p $@

${ZSSH_BINARY}: ${BUILD_SRC_DIR}/zssh.o | ${OUTPUT_BIN_DIR}
	gcc $< ${SSH_STATIC_LIB} -o $@ -lcrypto

${BUILD_SRC_DIR}:
	mkdir -p $@

${BUILD_SRC_DIR}/%.o: ${SRC_DIR}/%.c ${SSH_STATIC_LIB} | ${BUILD_SRC_DIR}
	gcc ${CCFLAGS} -c $< -o $@


clean:
	rm -rf ${OUTPUT_DIR} ${BUILD_SRC_DIR}
.PHONY: clean
