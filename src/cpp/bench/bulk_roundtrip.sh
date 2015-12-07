
if [[ -z $1 && -z $2 ]];then
    echo -e "usage : \t $0 <sqy-python-script> <sqy-app> <tif-file(s)>"
else

cmds[1]="--preset ultrafast --lossless"
cmds[2]="--preset ultrafast"
cmds[3]="--preset ultrafast --pmode --pme"
cmds[4]="--preset ultrafast --ssim"
cmds[5]="--preset ultrafast --psnr"

shid[1]=_lrt
shid[2]=_drt
shid[3]=_dprt
shid[4]=_prt
shid[5]=_srt

self_base=x265_roundtrip

ofiles[1]=$self_base${shid[1]}".log"
ofiles[2]=$self_base${shid[2]}".log"
ofiles[3]=$self_base${shid[3]}".log"
ofiles[4]=$self_base${shid[4]}".log"
ofiles[5]=$self_base${shid[5]}".log"

PYTHON_APP=$1
SQY_APP=$2

shift
shift
FILES=${@}

for i in `seq 1 5`;
do
    python $PYTHON_APP -x "${cmds[$i]}" -a $SQY_APP -s ${shid[$i]} -o ${ofiles[$i]} $FILES
done

fi
