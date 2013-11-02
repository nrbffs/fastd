set(VERSION_FILE "${FASTD_BINARY_DIR}/src/fastd_version.h")

add_custom_target(
    version
    COMMAND echo "#ifndef _FASTD_FASTD_VERSION_H_" > "${VERSION_FILE}.new"
    COMMAND echo "#define _FASTD_FASTD_VERSION_H_" >> "${VERSION_FILE}.new"
    COMMAND sh -c "echo \"#define FASTD_VERSION \\\"$(git --git-dir=./.git describe --dirty 2>/dev/null || echo ${FASTD_VERSION})\\\"\"" >> "${VERSION_FILE}.new"
    COMMAND echo "#endif /* _FASTD_FASTD_VERSION_H_ */" >> "${VERSION_FILE}.new"
    COMMAND cmp -s "${VERSION_FILE}" "${VERSION_FILE}.new" && rm "${VERSION_FILE}.new" || mv "${VERSION_FILE}.new" "${VERSION_FILE}"
    WORKING_DIRECTORY "${FASTD_SOURCE_DIR}"
    VERBATIM
)
