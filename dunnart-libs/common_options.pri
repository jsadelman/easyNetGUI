EASYNETBASE = $${PWD}/..

CONFIG( debug, debug|release ) {
    DLLDESTDIR = $$EASYNETBASE/dll_debug
} else {
    DLLDESTDIR = $$EASYNETBASE/dll_release
}

macx {
  DESTDIR = $$EASYNETBASE/dll
  DLLDESTDIR = $$EASYNETBASE/dll
}


