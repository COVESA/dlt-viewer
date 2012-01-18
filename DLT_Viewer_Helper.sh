#!/bin/bash

QEXTSERIALPORTURL="http://downloads.sourceforge.net/project/qextserialport/qextserialport/1.1/qextserialport-1.1.tar.gz"
QEXTSERIALPORTTGZ="qextserialport-1.1.tar.gz"
QEXTSERIALPORTDIR="qextserialport"
QEXTSERIALPORTPROBACKUP="qextserialport/qextserialport.pro.bak"
QEXTSERIALPORTPRO="qextserialport/qextserialport.pro"

OPTIONS="Backup_of_qextserialportpro Download_qextserialport Unzip_qextserialport Restore_qextserialportpro Create_makefile Run_make Run_dlt_viewer (Optional)Install_on_system  Quit"

function checkStatus {
		if [ $? == 0 ]; then
			echo "- Done"
		else
			echo "- Error"
		fi
}

function createbackup {
	echo "- Create backup of $QEXTSERIALPORTPRO"
	mv $QEXTSERIALPORTPRO $QEXTSERIALPORTPROBACKUP
	checkStatus
}

function downloadqextserialport {
	echo "- Download $QEXTSERIALPORTURL"
	cd $QEXTSERIALPORTDIR
	if [ -f $QEXTSERIALPORTTGZ ]; then
		echo "- $QEXTSERIALPORTTGZ exists - download skipped"
	else
		echo "- $QEXTSERIALPORTTGZ doesn't exist - start download"
		wget $QEXTSERIALPORTURL
		checkStatus
	fi
	cd ..
}

function unziqextserialport {
	echo "- Unzip of $QEXTSERIALPORTTGZ"
	cd $QEXTSERIALPORTDIR
	tar xvzf $QEXTSERIALPORTTGZ
	checkStatus
	cd ..
}

function restorebackup {
	echo "- Restore backup of $QEXTSERIALPORTPROBACKUP"
	cp $QEXTSERIALPORTPROBACKUP $QEXTSERIALPORTPRO
	checkStatus
}


function checkbuilddirectory {
	echo "- Check build directory"
	if [ -d build ]; then
		read -p "- Build directory already exists. Do you want to clear the directory (y/n)?"
		if [ "$REPLY" == "y" ]; then
			rm -fr build/*
		fi
	else
		mkdir build
	fi
}


function createmakefile {
	echo "- Generate makefile "
	checkbuilddirectory
	cd build
	echo "- Run qmake-qt4"
	qmake-qt4 ../BuildDltViewer.pro
	cd ..
}

function runmake {
	echo "- Run make"
	cd build
	make
	cd ..
}

function installonsystem {
	cd build
	read -p "- Do you want to install the DLT Viewer and libraries on the system (y/n)?"
	if [ "$REPLY" == "y" ]; then
		sudo make install
		sudo ldconfig
	fi
	cd ..
}

function runviewer {
	cd build/release
	./dlt_viewer
	cd ../..
}

select opt in $OPTIONS; do
	if [ "$opt" = "Quit" ]; then
		echo "- Exit $0"
		exit
	elif [ "$opt" = "Backup_of_qextserialportpro" ]; then
		createbackup
	elif [ "$opt" = "Download_qextserialport" ]; then
		downloadqextserialport
	elif [ "$opt" = "Unzip_qextserialport" ]; then
		unziqextserialport
	elif [ "$opt" = "Restore_qextserialportpro" ]; then
		restorebackup
	elif [ "$opt" = "Create_makefile" ]; then
		createmakefile
	elif [ "$opt" = "Run_make" ]; then
		runmake
	elif [ "$opt" = "(Optional)Install_on_system" ]; then
		installonsystem
	elif [ "$opt" = "Run_dlt_viewer" ]; then
		runviewer
	else
		echo "- Bad option"
	fi
done
