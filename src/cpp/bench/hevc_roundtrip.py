import timeit
import os
import re
import shutil
import subprocess
import argparse
import sys

def produce_shorthand(_string):

    
    value = "_"

    if _string:
        as_list = [ item.strip('-') for item in  _string.split()]
        for item in as_list:
            value += item[0]
    else:
        value += "d"

    value += "rt"
    return value
    
if __name__ == '__main__':
    
    descr = """ 
    perform hevc roundtrip and print results
    """
    parser = argparse.ArgumentParser(description=descr)

    parser.add_argument('inputfiles', 
                        nargs = "+",
                        help='.tif files to process'
                        )

    parser.add_argument('-x,--x265_args', dest='hevc_args', action='store', type=str,
                        default="", help='arguments to x265 (input and output file will be inserted, e.g. --lossless --preset ultrafast')

    parser.add_argument('-o,--output_stats', dest='output_file', action='store', type=str,
                        default="hevc_roundtrip.log", help='file to append stats to')

    
    parser.add_argument('-v,--verbose', dest='verbose', action='store_true', default=False, help='verbose stdout (default: False)')
    
    parser.add_argument('-n,--nrepeats', dest='nrepeats', action='store',type=int,
                        default=10, help='how many times the commands are to be repeated (default: 10)')

    parser.add_argument('-s,--shorthand', dest='shorthand', action='store',type=str,
                        default='', help='shorthand to insert to filenames')
    
    parser.add_argument('-a,--sqy', dest='sqy_path', action='store',type=str,
                        default='./sqy', help='location of the sqy app')
    args_parsed = parser.parse_args()

    if hasattr(args_parsed,"help"):
        parser.print_help()
        sys.exit(1)

    if not (os.path.exists(args_parsed.sqy_path) and os.path.isfile(args_parsed.sqy_path)):
        print args_parsed.sqy_path, " does not exist"
        sys.exit(1)

    if not hasattr(args_parsed,"inputfiles"):
        print "no input files given"
        sys.exit(1)

        
    shorthand = produce_shorthand(args_parsed.hevc_args)
    if hasattr(args_parsed,"shorthand") and args_parsed.shorthand:
        shorthand = args_parsed.shorthand
        
    results = {}
    
    for tiffile in args_parsed.inputfiles:
        tif_tuple = os.path.splitext(tiffile)
        y4mfile = tif_tuple[0] + '.y4m'
        lutfile = tif_tuple[0] + '.lut'
        hevcfile = tif_tuple[0] + shorthand + '.hevc'
        if not os.path.exists(y4mfile):
            sqy_cnv_cmd = "%s convert %s %s" % (args_parsed.sqy_path,tiffile,y4mfile)
            try:
                subprocess.check_call(sqy_cnv_cmd.split())
            except Exception as exc:
                print "%s failed with %s" % (sqy_cnv_cmd,exc)
                continue

        results[tiffile] = [os.stat(tiffile).st_size/(1024*1024)]
        shutil.copy(lutfile, tif_tuple[0] + shorthand + '.lut')

        compress_cmd = "x265 %s %s %s" % (args_parsed.hevc_args,y4mfile,hevcfile)
        try:
            t = timeit.Timer("subprocess.check_output("+str(compress_cmd.split())+")",setup="import subprocess")
            time_sec = t.timeit(1)
        except Exception as exc:
            print "\n%s failed with %s" % (compress_cmd,exc)
            continue
        else:
            results[tiffile].append(os.stat(hevcfile).st_size/(1024.*1024.))
            results[tiffile].append(time_sec)

        rty4mfile = tif_tuple[0] + shorthand + '.y4m'
        rttiffile = tif_tuple[0] + shorthand + '.tif'
        ffmpeg_cmd = "ffmpeg -y -i %s %s" % (hevcfile,rty4mfile)
        try:
            subprocess.check_call(ffmpeg_cmd.split())
        except Exception as exc:
            print "%s failed with %s" % (ffmpeg_cmd,exc)
            continue

        sqy_rt_cmd = "%s convert %s %s" % (args_parsed.sqy_path,rty4mfile,rttiffile)
        try:
            subprocess.check_call(sqy_rt_cmd.split())
        except Exception as exc:
            print "%s failed with %s" % (sqy_rt_cmd,exc)
            continue

        sqy_cmp_cmd = "%s compare -m 'mse,nrmse' %s %s" % (args_parsed.sqy_path,rttiffile,tiffile)
        try:
            cmp_out = subprocess.check_output(sqy_cmp_cmd.split())
        except Exception as exc:
            print "%s failed with %s" % (sqy_cmp_cmd,exc)
            continue

        for item in cmp_out.split():
            results[tiffile].append(float(item))

        results[tiffile].append(shorthand)


    stats = "%30s %20s %20s %20s %20s %20s %20s\n" % ("fname","size_MB","compressed_MB","time_s","mse","nrmse","id")
    
    for k,v in results.iteritems():
        if len(v) == 6:
            stats+=("%30s %20i %20f %20f %20f %20f %20s\n" % (os.path.basename(k),v[0],v[1],v[2],v[3],v[4],v[5]))
        else:
            if len(v)>0:
                stats += (os.path.basename(k)+"\t"+"\t".join([ str(i) for i in v])+"\n")

    ofile = open(args_parsed.output_file,"wa")
    ofile.writelines(stats)            
    ofile.close()
    print "written ",args_parsed.output_file
