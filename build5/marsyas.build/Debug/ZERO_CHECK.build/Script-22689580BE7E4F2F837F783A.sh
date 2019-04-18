#!/bin/sh
make -C /Users/nao/git/marsyas/build5 -f /Users/nao/git/marsyas/build5/CMakeScripts/ZERO_CHECK_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all
