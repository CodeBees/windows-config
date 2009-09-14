#!/bin/perl

use strict;
use File::Glob qw(:globally :nocase);
use File::Basename;



my @dsw_slns;
my @sub_projs;
my @projs_to_compile;
my @configs_to_compile;
my @targets_to_compile;
my $is_proj_vc6;
my $vc_program;
my $sln_file;
my $cwd;
my $sln_dir;

main();

sub main
{
    chomp($cwd = `pwd`);
    die "Can't compile in a UNC directory!\n" if ($cwd =~ m#^//(?!/)#);
    $cwd =~ s#/+#/#g;

    getSlnFile();
    $is_proj_vc6 = 1 if ($sln_file =~ m/\.dsw$/i);
    if ($is_proj_vc6) {
        $vc_program = "vc6.com";
    } else {
        my @version_line = grep {/Format Version (.*?)\.00/} `head -n 5 "$sln_file"`;
        die "$sln_file has no version info!" if (scalar @version_line != 1);
        $version_line[0] =~ m/Format Version (.*?)\.00/;
        $vc_program = "vc".($1-1).".com";
    }

    getAllSubProjs();

    my $arg = shift @ARGV;
    if (not $arg) {
        @projs_to_compile = @sub_projs[0];
    } elsif ($arg eq "all") {
        @projs_to_compile = @sub_projs;
    } else {
        @projs_to_compile = ($arg);
    }

    $arg = shift @ARGV;
    if (not $arg) {
        @configs_to_compile = qw(release);
    } elsif ($arg eq "all") {
        @configs_to_compile = qw(release debug);
    } else {
        @configs_to_compile = ($arg);
    }

    @targets_to_compile = @ARGV;

    doCompile();
}

sub doCompile
{
    foreach my $p (@projs_to_compile) {
        print "make: Entering directory ", getProjDir($p), "\n";
        
        foreach my $c (@configs_to_compile) {
            chdir ($sln_dir);
            #subprocess.check_call(('vc6.com', slnFile, '/make', "%s - win32 %s" % (p, c)) + options)
            my $vc_out;
            if ($is_proj_vc6) {
                open($vc_out, "-|", $vc_program, basename($sln_file), "/make", "$p - win32 $c", @targets_to_compile);
            } else {
                #subprocess.check_call((vcExec, slnFile, '/Project', p, cmd, c) + options)
                my $cmd = shift @targets_to_compile;
                $cmd = "/Build" if (not $cmd);

                open($vc_out, "-|", $vc_program, basename($sln_file), "/Project", $p, $cmd, $c, @targets_to_compile);
            }
            handleVcOutput($vc_out);
            close $vc_out;
            die "child $vc_program exited with $?" if ($?);
        }
        print "make: Leaving directory ", getProjDir($p), "\n";
    }
}

sub canonic_files
{
    (my $base, my @dirs) = @_;
    die "base is empty!" if (not $base);
    
    $" = ", ";
    my $dir;
    my @existing_dirs;
    my @good_base;

    foreach my $dir (@dirs) {
        if (not -d $dir) {
            @existing_dirs = (@existing_dirs, canonic_files(basename($dir), dirname($dir)));

        } else {
            @existing_dirs = (@existing_dirs, $dir);
        }
    }

    foreach my $dir (@existing_dirs) {
        opendir my $dh, $dir or next;
        my @matches = map {$_ = "$dir/$_"} grep {uc $base eq uc $_} readdir($dh);
        if (scalar @matches == 0) {
            @good_base = (@good_base, "$dir/$base");
        } else {
            @good_base = (@good_base, @matches);
        }
        closedir $dh;
        return @good_base;                      
    }
}

sub handleVcOutput($)
{

# #sample output from vc8, c++ files
# #1>.\findexec.cpp(31) : fatal error C1083: Cannot open include file: 'bhjdebug.h': No such file or directory

# #sample output from vc9, c# files
# #D:\profiles\Program.cs(15,29): error CS0029: Cannot implicitly convert type 'System.Diagnostics.Process[]' to 'System.Diagnostics.Process'

    (my $vc_out) = @_;
    our %err_file_canonic;

    while (<$vc_out>) {
        s/^[0-9]+>//;
        s/\):/) :/g;
        s/\(([0-9]+),[0-9]+\)/($1)/;
        s/error CS([0-9]+):/error C$1:/;
        #------ Build started: Project: AlibImpl, Configuration: Release Win32 ------
        if (/^-+Configuration: (.*?) / or /^------ Build started: Project: (.*?), /) {
            chdir($sln_dir);
            my $proj_dir = getProjDir($1);
            chomp($proj_dir = `cygpath -au "$proj_dir"`);
            die "proj_dir is empty!" if (not $proj_dir);
            my @proj_dir = canonic_files(".", $proj_dir);
            die "found more than 1 proj_dir" if (scalar @proj_dir != 1);
            chdir $proj_dir[0];
        }
        if (m/(.*?)(\([0-9]+(?:,[0-9]+)?\).*:.*)/) {
            
            chomp(my $key = `cygpath -au "$1"`);
            $key = uc($key);
            my $err_file;
            if ($err_file_canonic{$key}) {
                $err_file = $err_file_canonic{$key};
            } else {
                chomp($err_file = `cygpath -au "$1"`);
                my @err_file = canonic_files(basename($err_file), dirname($err_file));
                die "File `$1' doesn't exist!" if (scalar @err_file != 1);
                
                $err_file = $err_file[0];
                $err_file_canonic{$key} = $err_file;
            }            

            print $err_file, $2, "\n";
        } else {
            print $_;
        }
    }

}

sub getProjDir($)
{
    (my $proj) = (@_);
    open (SLN_FILE, $sln_file) or die "can't open $sln_file";
    my @proj_dir;

    #vc6: Project: "Alchemy"=.\Alchemy\Alchemy.dsp - Package Owner=<4>
    #vc789: Project("{8BC9CEB8-...") = "Alchemy", "Alchemy\Alchemy.vcproj", "{A0D5CB27...
    if ($is_proj_vc6) {
        @proj_dir = map {m/^Project: "$proj"=(.*?) - /; $_ = $1;} grep {/^Project: "$proj"=/i} <SLN_FILE>;
    } else {
        @proj_dir = map {my @fields = split(/"/); $_ = $fields[5];} grep {/^Project\(.*"$proj",/i} <SLN_FILE>;
    }

    close SLN_FILE;
    chomp(my $proj_dir = `cygpath -au "$proj_dir[0]"`);
    return dirname($proj_dir);
    
}

sub getSlnFile
{
    while (1) {
        @dsw_slns = glob("*.dsw *.sln");
        die "Found more than 1 .dsw or .sln files" if (scalar @dsw_slns > 1);
        
        die "Can't find any .dsw or .sln files" if (not @dsw_slns and `pwd` =~ m#^/$#);
        
        last if @dsw_slns; #found exactly 1 .dsw or .sln file

        chdir "..";
    }

    chomp($sln_file = `readlink -f $dsw_slns[0]`);
    chomp($sln_dir = `pwd`);
}

sub getAllSubProjs
{
    open PROJ_FILE, $sln_file;
    if ($is_proj_vc6) {
        #sample line: Project: "Alchemy"=.\Alchemy\Alchemy.dsp - Package Owner=<4>
        map {push @sub_projs, (split('"'))[1]} grep {/^Project:/} <PROJ_FILE>;
    } else {
        #sample line: Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "findexec", ...
        map {push @sub_projs, (split('"'))[3]} grep {/^Project/} <PROJ_FILE>;
    }
    close PROJ_FILE;
}
