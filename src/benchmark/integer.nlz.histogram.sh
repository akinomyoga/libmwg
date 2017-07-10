#!/bin/bash

name=${1:-laguerre.icc}

file=integer.nlz.$name.txt
if [[ ! -e $file ]]; then
  ./integer.nlz.bench1.exe > "$file"
fi

sort "$file" | gawk '
  function output() {
    if (title != "" && count != 0) {

      sub(/:$/, "", title);
      sub(/builtin/, "builtin(clz)", title);
      sub(/bctz/, "builtin(ctz)", title);
      sub(/asmbsr/, "asm(bsr)", title);
      sub(/^shift$/, "shift1", title);
      sub(/^bsec$/, "bsec1", title);
      sub(/^kazatsuyu$/, "kazatsuyu(nlz)", title);
      sub(/^debruijn$/, "kazatsuyu(nd)", title);
      sub(/^base$/, "空ループ", title);

      avg = sum / count;
      err = sqrt((sum2 / count - avg * avg) / (count == 1? 1: count - 1));

      n = asort(arr, arr, "@val_num_asc"); # latest version of gawk is required
      median = n % 2 == 0? 0.5 * (arr[int(n / 2)] + arr[int(n / 2 + 1)]): arr[int((n + 1) / 2)];
      min = arr[1];
      max = arr[n];

      #printf("%s %.3f %.3f %g %g %g\n", title, avg, err, min, max, median);
      printf("| %s | %.2f(%02d) | %g | %g | %g |\n", title, avg, int(err * 100 + 0.5), min, median, max);
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
  END{output();}
'
