#!/usr/bin/perl

use strict;
use warnings;
use utf8;

my $prog='extr1';

my $filename=$ARGV[0];
if(!$filename){
  print STDERR "$prog: the filename is not specified!\n";
  exit 1;
}

sub file_get_contents {
  my $filaname=shift;
  my $fh;
  if(!open($fh,"< $filename")){
    print STDERR "$prog: failed to open the file '$filename'.\n";
    exit 1;
  }
  my $content = do { local $/; <$fh> };
  close($fh);
  return $content;
}
my $content=file_get_contents($filename);

my $fbase=$filename;
#$fbase =~  s/(?:(?!^)\.[[:alnum:]]+)?$//;
$fbase =~ s/\.(?=[^\/]+$)/_/g;

my $oconf = {
  fname => "$fbase.mconf",
  isfirst => 1,
  handle => undef
};
open(${$oconf}{handle},"> ${$oconf}{fname}");

sub file_print {
  my $obj=shift;
  my $value=shift;
  my $handle=${$obj}{handle}; # 何故かスカラー変数に代入していないと駄目
  if(${$obj}{isfirst}){
    ${$obj}{isfirst}=0;
    print $handle "P '/* config by $filename */'\nP ''\n\n";
  }
  print $handle "$value\n";
}

# my $fconf="$fbase.mconf";
# open(my $hconf,"> $fconf");

my $fwiki="$fbase.lwiki";
open(my $hwiki,"> $fwiki");

while($content =~ m/'(?:[^\\']|\\.)*'|"(?:[^"\\]|\\.)*"|\/\/(?:\?([[:alnum:]_]+)\b[[:space:]]*(.+?)|.*?)(?:[\r\n]|$)|\/\*(?:\?([[:alnum:]_]+)[[:space:]]*([\s\S]+?)|[\s\S]*?)\*\//g) {
  my ($type,$value);
  if($1){
    # print 'line comment -> ',$1,"\n";
    $type=$1;
    $value=$2;
  }elsif($3){
    # print 'multiline comment -> ',$3,"\n";
    $type=$3;
    $value=$4;
    $value =~ s/^[[:space:]]*\* ?//gm;
    $value =~ s/[[:space:]]+$//;
  }

  if($type){
    if($type eq 'mconf'){
      #print $hconf "$value\n";
      file_print($oconf,$value);
    }elsif($type eq 'lwiki'){
      print $hwiki "$value\n";
    }
  }
}

# close($hconf);
close(${$oconf}{handle});
close($hwiki);
