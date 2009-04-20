#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp -o -name \*.cc` -o $podir/kwin3_deKorator.pot
rm -f rc.cpp

