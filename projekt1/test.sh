#!/bin/sh

# IOS proj1, neoficialni testy
# autor: JohnyK (Discord)
#
# spusteni vsech testu:
# $ ./test.sh
#
# spusteni konkretniho testu:
# $ ./test.sh -t nazev
#
# vypisovat vystup z wana:
# $ ./test.sh -v

# Nastaveni skriptu

# nazev skriptu
WANA=./wana

# adresar s log soubory. Pokud jsou logy v adresari, pouzij napr ./logs/
LOGS_DIR=./

# adresar s testy
TESTS_DIR=./tests

# pouzity interpret (sh, bash, ..)
SHELL=sh

# zakomentovat :)
export POSIXLY_CORRECT=yes

# ------------------------------------------------------------------------------

RED='\033[1;31m'
GREEN='\033[1;32m'
NC='\033[0m' # No Color
USE_COLOR=1
TOTAL_CNT=0
ERROR_CNT=0

ARGS_NAME=args
CHECK_NAME=check.sh
TMPFILE=__wanatest.tmp

export CAT=cat
export DIFF=diff
export SORT=sort
export LC_ALL=POSIX

print_ok() {
  TEXT=$1
  if [ "$USE_COLOR" -eq 1 ] ; then
    printf "[${GREEN}OK${NC}] $TEXT\n"
  else
    printf "[OK] $TEXT\n"
  fi
}

print_err() {
  TEXT=$1
  if [ "$USE_COLOR" -eq 1 ] ; then
    printf "[${RED}FAILED${NC}] $TEXT\n"
  else
    printf "[FAILED] $TEXT\n"
  fi
}

test() {
  export TEST_PATH=$1

  if [ ! -d "$TEST_PATH" ]; then
    echo "TEST NOT EXIST"
    return
  fi

  TOTAL_CNT=$(expr $TOTAL_CNT + 1)
  ARGS=$(sed "s|{LOGS_DIR}|${LOGS_DIR}|" < ${TEST_PATH}/${ARGS_NAME})

  printf "Test: $(basename ${TEST_PATH}) $ ${WANA} $ARGS "
  eval "${SHELL} ${WANA} ${ARGS}" > ${TMPFILE}
  WANA_RETURN_CODE=$?
  printf "[$WANA_RETURN_CODE] "

  ${TEST_PATH}/${CHECK_NAME} "$WANA_RETURN_CODE" < $TMPFILE > /dev/null
  if [ $? -eq 0 ]; then
    print_ok ""
  else
    print_err ""
    ERROR_CNT=$(expr $ERROR_CNT + 1)
  fi

  if [ -n "$VERBOSE" ] ; then
    cat "$TMPFILE"
  fi

  rm ${TMPFILE}
}

while getopts ":vt:" opt;
do
case $opt in
    v)      VERBOSE=1
            ;;
    t)      SPECIFIC_TEST=$OPTARG
            ;;
   \?)      echo "Invalid option: -$OPTARG" >&2
            ;;
esac
done

if [ -z "$SPECIFIC_TEST" ] ; then
  # all tests
  for file in $(ls ${TESTS_DIR});
  do
    test "$TESTS_DIR/${file}"
  done
else
  # perform specific test
  test "$TESTS_DIR/${SPECIFIC_TEST}"
fi

echo "--------------------------------"
echo "Test count: $TOTAL_CNT, Error count: ${ERROR_CNT}"
