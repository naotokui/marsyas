# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.ibt_.Debug:
/Users/nao/git/_public/marsyas/build/bin/Debug/ibt_.app/Contents/MacOS/ibt_:\
	/Release/libmarsyas.a
	/bin/rm -f /Users/nao/git/_public/marsyas/build/bin/Debug/ibt_.app/Contents/MacOS/ibt_


PostBuild.ibt_.Release:
/Users/nao/git/_public/marsyas/build/bin/Release/ibt_.app/Contents/MacOS/ibt_:\
	/Release/libmarsyas.a
	/bin/rm -f /Users/nao/git/_public/marsyas/build/bin/Release/ibt_.app/Contents/MacOS/ibt_


PostBuild.ibt_.MinSizeRel:
/Users/nao/git/_public/marsyas/build/bin/MinSizeRel/ibt_.app/Contents/MacOS/ibt_:\
	/Release/libmarsyas.a
	/bin/rm -f /Users/nao/git/_public/marsyas/build/bin/MinSizeRel/ibt_.app/Contents/MacOS/ibt_


PostBuild.ibt_.RelWithDebInfo:
/Users/nao/git/_public/marsyas/build/bin/RelWithDebInfo/ibt_.app/Contents/MacOS/ibt_:\
	/Release/libmarsyas.a
	/bin/rm -f /Users/nao/git/_public/marsyas/build/bin/RelWithDebInfo/ibt_.app/Contents/MacOS/ibt_


PostBuild.ibt_.Profile:
/Users/nao/git/_public/marsyas/build/bin/Profile/ibt_.app/Contents/MacOS/ibt_:\
	/Release/libmarsyas.a
	/bin/rm -f /Users/nao/git/_public/marsyas/build/bin/Profile/ibt_.app/Contents/MacOS/ibt_




# For each target create a dummy ruleso the target does not have to exist
/Release/libmarsyas.a:
