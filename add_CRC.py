Import("env")

# access to global build environment
print env

# Add CRC from BIN
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(" ".join([
        "AfterburnerCRC.exe", "$BUILD_DIR/${PROGNAME}.bin"
    ]), "Creating CRC from $BUILD_DIR/${PROGNAME}.bin, adding to $BUILD_DIR/${PROGNAME}.bin.crc")
)
