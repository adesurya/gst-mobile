#!/bin/sh
# uskee.org
#

#------------------------------------------------------------------------

#
# Pls define the following variables by correspoding shell VARIBLEs:
# (*) sdk:      android sdk dir
# (*) target:   android target platform
# (*) src:      java source dir
# (*) build:    object dest dir
# (*) extra:    extra depended jar
#


#
# set target object name
OBJ_NAME=k2player

#
# set java source dir and dest dir, NOTE: should be absolute dir
SRC_DIR=`pwd`/src
DST_DIR=/tmp/build

#
# set extra depended jar, NOTE: should be absolute dir
EXTRA_DIR=`pwd`

#
# set android target platform and sdk dir
TARGET=android-17
SDK_DIR=/opt/zdisk/zerox/android/android-sdk-linux-i386

#-------------------------------------------------------------------------


#
# generate build-jar.xml for ant
BUILD_XML=/tmp/build-jar.xml
cat > $BUILD_XML <<EOF
<project name="${OBJ_NAME}" basedir="." default="main">
<property name="src.dir"     value="\${src}"/>
<property name="build.dir"   value="\${build}"/>
<property name="android.dir"  value="\${sdk}/platforms/\${target}/" />  
<property name="extra.dir"  value="\${extra}"/>

<property name="classes.dir" value="\${build.dir}/classes"/>
<property name="jar.dir"     value="\${build.dir}/jar"/>

<target name="clean">
<delete dir="\${build.dir}"/>
</target>

<target name="compile">
<mkdir dir="\${classes.dir}"/>
<javac srcdir="\${src.dir}" destdir="\${classes.dir}" includeantruntime="false" target="1.6">
<classpath>
<fileset dir="\${android.dir}" includes="*.jar" />
<fileset dir="\${extra.dir}" includes="*.jar" />
</classpath>
</javac>
</target>

<target name="jar" depends="compile">
<mkdir dir="\${jar.dir}"/>
<jar destfile="\${jar.dir}/\${ant.project.name}.jar" basedir="\${classes.dir}"/>
</target>

<target name="main" depends="clean,jar"/>
</project>
EOF


#
# run ant
ant -Dsdk=$SDK_DIR -Dtarget=$TARGET \
    -Dsrc=$SRC_DIR -Dbuild=$DST_DIR \
    -Dextra=$EXTRA_DIR \
    -f $BUILD_XML
cp $DST_DIR/jar/$OBJ_NAME.jar libs/

exit 0

