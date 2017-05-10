# Benchmarking sqeazy

This directory contains a Snakefile that is used to run a benchmark sweep on a given folder containing 3D uint8 or uint16 tiff stacks. For more information on snakemake and the Snakefile syntax, please consult the [public documentation](https://snakemake.readthedocs.io/en/stable/)

## How to run 

Once you dial in to either `sqy` or `ffmpeg`, you can run the snakemake workflow. The following assumes that all required packages are available in the environment of your terminal:

```
$ snakemake -d /path/to/corpus/original/ --cluster "sbatch -t 00:10:00 -c {threads} -o {TGTDIR}/{rule}-{wildcards.stem}-%A.log -J {rule}-{wildcards.stem}" -j 50
```

This command will issue 50 jobs at any point in time that are executed on the cluster (running SLURM here).

Note for `ffmpeg` subfolder, your job needs to land on nodes that have a supported GPU:

```
$ snakemake -d /path/to/corpus/original/ --cluster "sbatch -t 00:10:00 -p gpu -c {threads} -o {TGTDIR}/{rule}-{wildcards.stem}-%A.log -J {rule}-{wildcards.stem}" -j 50
```

