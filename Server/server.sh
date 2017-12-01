case $1 in
  0)
    make
    ;;
  1)
  ./server  | tee -a logs.txt
  ;;
esac
