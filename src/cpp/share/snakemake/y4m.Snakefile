import os
import glob

JOBDIR=os.path.abspath(os.path.curdir)
TGTDIR=os.path.abspath(os.path.curdir+"/../y4m/")

ALLTIFFS=glob.glob(JOBDIR+"/*.tif")
ALLSTEMS=[ os.path.splitext(os.path.basename(item))[0] for item in ALLTIFFS ]

if not config or not ("sqy-app" in config.keys()):
    config["sqy-app"] = "/home/steinbac/development/sqeazy/src/cpp/build/src/sqy"

if not config or not ("setup-cmd" in config.keys()):
    config["setup-cmd"] = "module load ffmpeg/3.0.7-x264-hevc lz4/1.7.5 hdf5/1.8.17"


rule all:
    input:
        [ "{dstfolder}/{stem}_unweighted.y4m".format(dstfolder=TGTDIR,stem=item) for item in ALLSTEMS ], [ "{dstfolder}/{stem}_weighted_power_3_1.y4m".format(dstfolder=TGTDIR,stem=item) for item in ALLSTEMS ]
    message:
        " creating {input} "

rule unweighted_quantisation:
    input:
        "{stem}.tif"
    output:
        "{TGTDIR}/{stem}_unweighted.y4m"
    shell:
        "{config[setup-cmd]} && {config[sqy-app]} convert {input} {output}"

rule weighted_quantisation:
    input:
        "{stem}.tif"
    output:
        "{TGTDIR}/{stem}_weighted_power_3_1.y4m"
    shell:
        "{config[setup-cmd]} && {config[sqy-app]} convert -q \"quantiser(weighting_function=power_3_1)\" {input} {output}"


rule clean:
    input:
        [ "{dstfolder}/{stem}_unweighted.y4m".format(dstfolder=TGTDIR,stem=item) for item in ALLSTEMS ], [ "{dstfolder}/{stem}_weighted_power_3_1.y4m".format(dstfolder=TGTDIR,stem=item) for item in ALLSTEMS ]
    shell:
        "rm -vf {input}"
