#! /bin/sh
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/autotests/'` -o $podir/akonadi_unifiedmailbox_agent.pot
