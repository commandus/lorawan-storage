#!/bin/bash
TEMPLATE=po/lorawan-identity-service.pot
for f in $(ls po/*.po) ; do
  regex="\/(.*)\.([a-z][a-z])_..\.UTF-8\.po"
  if [[ $f =~ $regex ]]; then
    fn="${BASH_REMATCH[1]}"
    code="${BASH_REMATCH[2]}"

    case $fn in
      'lorawan-identity-service') FM='cli-main.cpp';;
      *) FM='*';;
    esac

    xgettext -k_ -o $TEMPLATE $FM
    echo -n Merge $fn $FM ${code} ..
    msgmerge -U $f $TEMPLATE
    mkdir -p locale/${code}/LC_MESSAGES
    msgfmt -o locale/${code}/LC_MESSAGES/$fn.mo $f
  fi
  echo sudo cp locale/${code}/LC_MESSAGES/\*.mo /usr/share/locale/${code}/LC_MESSAGES/
done
