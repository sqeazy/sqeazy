platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
   platform='linux'
else
   platform='osx'
fi

if [[ $platform == 'osx' ]];then
    gradle build -Phdf5_path=/usr/local/ -Plz4_path=/usr/local/ -Pboost_path=/usr/local/ -Pffmpeg_path=/usr/local/ -Ptiff_path=/usr/local
else
    gradle build
fi
