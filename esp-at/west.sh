path=`pwd`
project=${path}/samples/esp-at

BOARD=native_posix_64

west build -p --build-dir ${path}/build \
    --board ${BOARD} ${project} --no-sysbuild \
    -DCONF_FILE:STRING="${project}/prj.conf;" \

