
TEMPLATE = subdirs

ROOT_DUNNARTLIB_DIR = dunnart-libs/

SUBDIRS = \
        $${ROOT_DUNNARTLIB_DIR}libavoid \
        $${ROOT_DUNNARTLIB_DIR}libvpsc \
        $${ROOT_DUNNARTLIB_DIR}libcola \
        $${ROOT_DUNNARTLIB_DIR}libtopology \
        $${ROOT_DUNNARTLIB_DIR}libdunnartcanvas \
        easyNet-app

CONFIG += ordered



