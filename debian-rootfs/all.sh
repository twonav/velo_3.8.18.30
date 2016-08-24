#!/bin/bash
bash clean.sh distclean
bash prepare.sh
bash patch.sh
bash configure.sh
bash compile.sh
bash applychanges.sh
bash install.sh
