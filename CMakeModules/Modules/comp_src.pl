#!/usr/bin/perl

  @list = @ARGV;
  foreach(@list) {
    $file = $_;
    $compfile = "../../../P2/cmake/Modules/".$file;
    $diffs = `diff $file $compfile`;
    if($diffs != '') {
        print "------------------------------------\n";
        print "$file\n";
        print "$diffs\n";
        print "------------------------------------\n";
    }
  }

