case $1 in
  0)
    make
    ;;
  1)
    ./emulator 2>&1  | tee --append logs.txt
    ;;
esac
