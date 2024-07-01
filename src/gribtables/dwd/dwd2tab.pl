#!/usr/bin/perl -w
# 1/2021       Public domain Wesley Ebisuzaki
#
# convert dwd shortName.def to gribtable format
#
# The "gribtable" file contains a colon separated list as follows:
#     column  1: Section 0 Discipline
#     column  2: Section 1 Master Tables Version Number
#     column  3: Section 1 Master Tables Minimum Version Number
#     column  4: Section 1 Master Tables Maximum Version Number
#     column  5: Section 1 originating centre, used for local tables
#     column  6: Section 1 Local Tables Version Number
#     column  7: Section 4 Template 4.0 Parameter category
#     column  8: Section 4 Template 4.0 Parameter number
#     column  9: Abbreviation
#     column 10: Description (parameter name)
#     column 11: Unit

open (IN,"< shortName.def");
open (OUT,"> dwd_gribtable");
while (<IN>) {
   if ($_ =~ m/^#/) {
     $var = $_;
     print "var id=$_\n";

     $def=<IN>;
     chomp $def;
     $def =~ s/^#//;
     print "def=$def";

     $name=<IN>;
     chomp $name;
     $name =~ s/^'//;
     $name =~ s/'.*$//;
     print "name=$name";
     $discipline = <IN>;
     chomp $discipline;
     $discipline =~ s/^.*discipline = //;
     $discipline =~ s/ ;.*//;
     print "dis=$discipline";
     $cat = <IN>;
     chomp $cat;
     $cat =~ s/^.*parameterCategory = //;
     $cat =~ s/ ;//;
     print "cat=$cat";
     $num = <IN>;
     chomp $num;
     $num =~ s/^.*parameterNumber = //;
     $num =~ s/ ;.*//;
     print "par=$num";
     $lastline = <IN>;
     print "last=$lastline";
     if ($lastline =~ m/}/) {
        print OUT "$discipline:10:0:255:0:255:$cat:$num:$name:$def\n";
     }
     else {
        print "no extended\n";
     }
   }
}   
