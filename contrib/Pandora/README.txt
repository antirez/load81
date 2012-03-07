
This is a build-tree for making a PND file for the Open Pandora game console.  It includes a 
PickleLauncher binary pre-built for the OpenPandora.  The purpose of this project is to
create an easy-to-use distribution of LOAD81 for Open Pandora users.  With this build-tree
it should be very easy to make a distribution of LOAD81 as new features get added.

References:

	Open Pandora console		http://openpandora.org/
	PND file info 				http://pandorawiki.org/Pnd
	PickleLauncher				http://pandorawiki.org/PickleLauncher
								http://sourceforge.net/projects/picklelauncher/

Requirements:
+ An Open Pandora console
+ the CDEV tools PND file, available here:
	http://repo.openpandora.org/?page=detail&app=cdevtools.freamon.40n8e
	
How to use:

1. Check out the LOAD81 sources onto your Open Pandora console somewhere sensible.
2. Execute the CDEV tools and get into the cdev shell (see CDEV docs)
3. Build LOAD81 on your Open Pandora (should be as simple as typing 'make' in the main LOAD81
sources tree) & verify that the load81 binary is created for Open Pandora
4. 'cd contrib/Pandora' and type 'make' or 'make clean && make pnd-file'

This will then produce the LOAD81.pnd file, bundled up and packaged with PickleLauncher for
selecting files, the load81 executable itself, and the load81 examples/ tree for easy use.  This
PND file can then be shared on repo's/forums/etc. for Open Pandora users to get their LOAD81
habits formed!  :)

Good luck!  


--
Questions / Comments about this contrib: seclorum@me.com (torpor on OPB forums)
Questions / Comments about LOAD81: antirez@gmail.com
Questions / Comments about PickleLauncher: antirez@gmail.com

