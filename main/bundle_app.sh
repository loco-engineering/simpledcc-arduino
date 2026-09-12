#!/usr/bin/env bash

# bundle_app.sh
# Bundles the web app files into a C header file (web_app_data.h).
# Replaces bundle_web.py.

set -e

MAIN_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_PUBLIC_DIR="$(cd "${MAIN_DIR}/../../app/public" && pwd)"
OUTPUT_FILE="${MAIN_DIR}/web_app_data.h"

if [ ! -d "${APP_PUBLIC_DIR}" ]; then
    echo "Error: web app public directory not found at ${APP_PUBLIC_DIR}"
    exit 1
fi

cat << 'EOF' > "${OUTPUT_FILE}"
#pragma once

typedef struct {
    const char* path;
    const char* mime;
    const unsigned char* data;
    unsigned int size;
} web_file_t;

EOF

FILE_RECORDS=""
FILE_COUNT=0
INDEX_HTML_RECORD=""

FILES=$(ls -1 "${APP_PUBLIC_DIR}" | sort)

for filename in $FILES; do
    filepath="${APP_PUBLIC_DIR}/${filename}"
    if [ ! -f "${filepath}" ]; then
        continue
    fi
    
    var_name=$(echo "${filename}" | sed 's/\./_/g' | sed 's/-/_/g')
    
    mime="text/plain"
    if [[ "${filename}" == *.html ]]; then mime="text/html"
    elif [[ "${filename}" == *.js ]]; then mime="application/javascript"
    elif [[ "${filename}" == *.css ]]; then mime="text/css"
    elif [[ "${filename}" == *.json ]]; then mime="application/json"
    fi
    
    # Write binary data array
    echo -n "static const unsigned char ${var_name}_data[] = {" >> "${OUTPUT_FILE}"
    
    if command -v xxd >/dev/null 2>&1; then
        cat "${filepath}" | xxd -i >> "${OUTPUT_FILE}"
    else
        od -v -t x1 "${filepath}" | awk '{$1=""; print $0}' | tr -s ' ' '\n' | grep -v '^$' | awk '{printf "0x%s, ", $1}' | sed 's/, $//' >> "${OUTPUT_FILE}"
    fi
    
    echo "};" >> "${OUTPUT_FILE}"
    
    # get size
    size=$(wc -c < "${filepath}" | tr -d ' ')
    
    # append to records string
    FILE_RECORDS="${FILE_RECORDS}    {\"/$filename\", \"$mime\", ${var_name}_data, $size},\n"
    FILE_COUNT=$((FILE_COUNT + 1))
    
    # if it's index.html, we also map the root path "/"
    if [ "${filename}" = "index.html" ]; then
        INDEX_HTML_RECORD="    {\"/\", \"$mime\", ${var_name}_data, $size},\n"
        FILE_COUNT=$((FILE_COUNT + 1))
    fi
done

echo "" >> "${OUTPUT_FILE}"
echo "static const web_file_t web_files[] = {" >> "${OUTPUT_FILE}"
printf "%b" "${FILE_RECORDS}" >> "${OUTPUT_FILE}"
if [ -n "${INDEX_HTML_RECORD}" ]; then
    printf "%b" "${INDEX_HTML_RECORD}" >> "${OUTPUT_FILE}"
fi
echo "};" >> "${OUTPUT_FILE}"
echo "#define WEB_FILES_COUNT ${FILE_COUNT}" >> "${OUTPUT_FILE}"

echo "Successfully generated ${OUTPUT_FILE} with ${FILE_COUNT} files."
