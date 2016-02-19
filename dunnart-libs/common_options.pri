EASYNETBASE = $${PWD}/..

macx {
  DESTDIR = $$EASYNETBASE/dll
}

CONFIG( debug, debug|release ) {
    DLLDESTDIR = $$EASYNETBASE/dll_debug
} else {
    DLLDESTDIR = $$EASYNETBASE/dll_release
}

