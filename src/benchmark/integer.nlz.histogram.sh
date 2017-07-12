#!/bin/bash

export outdir=out/integer.nlz
[[ -d $outdir ]] || mkdir -p "$outdir"


function measure_and_summary {
  local name=${1:-laguerre.icc}
  
  local file=$outdir/integer.nlz.$name.txt
  if [[ $force_update || ! -e $file ]]; then
    ./integer.nlz.bench1.exe > "$file"
  fi
  
  sort "$file" | gawk '
    function output() {
      if (title != "" && count != 0) {
        sub(/:$/, "", title);
        sub(/^builtin$/, "bclz", title);
        sub(/^builtinbsr$/, "bbsr", title);
        sub(/^bcpop$/, "bpopcount", title);
        sub(/^bbsr$/, "ibsr", title);
        sub(/^bbsf$/, "ibsf", title);
  
        name = title;
        sub(/^base$/, "空ループ", name);
        sub(/^shift$/, "shift1", name);
        sub(/^bsec$/, "bsec1", name);
        sub(/^kazatsuyu$/, "kazatsuyu(nlz)", name);
        sub(/^debruijn$/, "kazatsuyu(nd)", name);
        sub(/^bclz$/, "builtin(ctz)", name);
        sub(/^bctz$/, "builtin(clz)", name);
        sub(/^bpopcount$/, "builtin(popcnt)", name);
        sub(/^bffs$/, "builtin(ffs)", name);
        sub(/^ilzcnt$/, "intrinsic(lzcnt)", name);
        sub(/^itzcnt$/, "intrinsic(tzcnt)", name);
        sub(/^ibsr$/, "intrinsic(bsr)", name);
        sub(/^ibsf$/, "intrinsic(bsf)", name);
        sub(/^ipopcnt$/, "intrinsic(popcnt)", name);
        sub(/asmbsr/, "asm(bsr)", name);
  
        avg = sum / count;
        err = sqrt((sum2 / count - avg * avg) / (count == 1? 1: count - 1));
  
        n = asort(arr, arr, "@val_num_asc"); # latest version of gawk is required
        median = n % 2 == 0? 0.5 * (arr[int(n / 2)] + arr[int(n / 2 + 1)]): arr[int((n + 1) / 2)];
        min = int(arr[1]);
        max = int(arr[n]);
  
        #printf("%s %.3f %.3f %g %g %g\n", name, avg, err, min, max, median);
        printf("| %s | %.2f(%02d) | %g | %g | %g |\n", name, avg, int(err * 100 + 0.5), min, median, max);
  
        if (output_histogram_data) {
          delete hist;
          for (i = 1; i <= n; i++) hist[arr[i]]++;
          fname = "hist." title ".txt";
          for (i = min; i <= max; i++)
            print i, 0+hist[i] > fname;
        }
  
        g_medians[title] = median;
      }
  
      title = "";
      count = 0;
      sum = 0;
      sum2 = 0;
      min = -1;
      max = -1;
      delete arr;
    }
    BEGIN{
      outdir = ENVIRON["outdir"];
      output_histogram_data = 0;
      print "| name | mean(error) | min | median | max |";
      print "|:---:|:---:|:---:|:---:|:---:|";
      print "# name mean error min max median";
    }
  
    $1 != title {output(); title = $1;}
    sub(/us$/, "", $2) {
      sum += $2;
      sum2 += $2 * $2;
      if (min < 0 || $2 < min) min = $2;
      if (max < 0 || $2 > max) max = $2;
      arr[count++] = $2;
    }
    END{
      output();
  
      fname = outdir "/graph.'"$name"'.txt"
      base = g_medians["base"];
      shift = g_medians["shift"] - base;
      n = split("shift shift4 shift8 bsec bsec2 bsec3 kazatsuyu debruijn debruijn2 frexp double float bclz ilzcnt ibsr asmbsr bctz bffs itzcnt ibsf bpopcount ipopcnt", g_titles);
      for (i = 1; i <= n; i++) {
        k = g_titles[i];
        time = g_medians[k] - base;
        if (time >= 0)
          ratio = time / shift;
        else {
          time = "NaN";
          ratio = "NaN";
        }
        print k, ratio, time > fname;
      }
    }
  '
}

if [[ $1 == force ]]; then
  if [[ $HOSTNAME == padparadscha ]]; then
    touch integer.nlz.bench1.cpp
    CXXKEY=c make
    force_update=1 measure_and_summary pad.clang
    touch integer.nlz.bench1.cpp
    CXXKEY=g make
    force_update=1 measure_and_summary pad.gcc
    touch integer.nlz.bench1.cpp
    CXXKEY=i make
    force_update=1 measure_and_summary pad.icc
  elif [[ ${HOSTNAME%%.*} == laguerre01 ]]; then
    touch integer.nlz.bench1.cpp
    CXXKEY=c35 make
    force_update=1 measure_and_summary laguerre.clang
    touch integer.nlz.bench1.cpp
    CXXKEY=g710 make
    force_update=1 measure_and_summary laguerre.gcc
    touch integer.nlz.bench1.cpp
    CXXKEY=i13 make
    force_update=1 measure_and_summary laguerre.icc
  elif [[ $HOSTNAME == magnate2016 ]]; then
    touch integer.nlz.bench1.cpp
    CXXKEY=c make
    force_update=1 measure_and_summary mag.clang
    touch integer.nlz.bench1.cpp
    CXXKEY=g make
    force_update=1 measure_and_summary mag.gcc
    touch integer.nlz.bench1.cpp
    CXXKEY=v1910 make
    force_update=1 measure_and_summary mag.msc
  fi
fi

# measure_and_summary pad.gcc
# measure_and_summary pad.clang
# measure_and_summary pad.icc
# measure_and_summary mag.gcc
# measure_and_summary mag.clang
# measure_and_summary mag.msc
# measure_and_summary laguerre.gcc
# measure_and_summary laguerre.clang
# measure_and_summary laguerre.icc
