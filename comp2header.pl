#!/usr/bin/env perl
use strict; use warnings;

my @skip = (
    qr/^#include\s+"libdtb0\.h"\s*$/,              # remove { #include "libdtb0.h" }
    qr/^#include\s+"libdtb\.h"\s*$/,               # remove { #include "libdtb.h" }
    qr/^#include\s+"llist\.h"\s*$/,                # remove { #include "llist.h" }
    qr/^#include\s+"libdtb_kernel_api\.h"\s*$/,    # remove { #include "libdtb_kernel_api.h" }
    qr/^\s*#include\s*<stdint\.h>\s*$/,            # remove { #include <stdint.h> }
    qr/^\s*#include\s*<stddef\.h>\s*$/,            # remove { #include <stddef.h> }
    qr/^\s*#include\s*<stdbool\.h>\s*$/,           # remove { #include <stdbool.h> }
    qr/^\s*#pragma\s+once\s*$/,                    # remove { #pragma once }
    qr/^\s*\/\//,                                  # remove single line comments { // }
    qr/\/\*.*\*\//,                                # remove single line comments { /* */ }
    qr/^\s*$/                                      # remove empty lines
);

sub emit_file {
    my ($out, $path) = @_;
    open my $in, '<', $path or die "failed to open $path: $!";
    LINE: while (my $line = <$in>) {
        for my $re (@skip) {
            next LINE if $line =~ $re;
        }
        print $out $line;
    }
    close $in;
}

sub emit_string {
    my ($out, $text) = @_;
    print $out $text;
    print $out "\n";
}

sub step (&) { $_[0] }

my @common_steps = (
    # preamble
    step { my ($out) = @_; emit_string($out, "// libdtb -- Single Header Build -- Generated With comp2header.pl") },
    step { my ($out) = @_; emit_string($out, "// Read the included README.md for how to use this library") },
    step { my ($out) = @_; emit_string($out, "// Find the source code at https://github.com/EvalynGoemer/libdtb.git") },
    step { my ($out) = @_; emit_string($out, "/*") },
    step { my ($out) = @_; emit_string($out, "==== Licenced under 0BSD and dedicated to the public domain where applicable ====") },
    step { my ($out) = @_; emit_string($out, "==== 0BSD Licence ====") },
    step { my ($out) = @_; emit_file($out,   "LICENCE") },
    step { my ($out) = @_; emit_string($out, "==== Public Domain ====") },
    step { my ($out) = @_; emit_file($out,   "PUBLIC_DOMAIN") },
    step { my ($out) = @_; emit_string($out, "*/") },

    # includes
    step { my ($out) = @_; emit_string($out, "#pragma once") },
    step { my ($out) = @_; emit_string($out, "#include <stdint.h>") },
    step { my ($out) = @_; emit_string($out, "#include <stddef.h>") },
    step { my ($out) = @_; emit_string($out, "#include <stdbool.h>") },
    step { my ($out) = @_; emit_string($out, "#define LIBDTB_SINGLE_HEADER") },
);

my @libdtb_steps = (
    # include the headers
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb0.h") },

    step { my ($out) = @_; emit_string($out, "#ifndef LIBDTB_FREESTANDING") },
    step { my ($out) = @_; emit_file($out,   "./lib/llist.h") },
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb_kernel_api.h") },
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb.h") },
    step { my ($out) = @_; emit_string($out, "#endif") },

    # include the source code
    step { my ($out) = @_; emit_string($out, "#ifdef LIBDTB_IMPL") },
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb0.c") },
    step { my ($out) = @_; emit_string($out, "#endif") },

    step { my ($out) = @_; emit_string($out, "#if defined(LIBDTB_IMPL) && !defined(LIBDTB_FREESTANDING)") },
    step { my ($out) = @_; emit_file($out,   "./lib/llist.c") },
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb.c") },
    step { my ($out) = @_; emit_string($out, "#endif") },
);

my @libdtb0_steps = (
    # include the headers
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb0.h") },

    # include the source code
    step { my ($out) = @_; emit_string($out, "#ifdef LIBDTB_IMPL") },
    step { my ($out) = @_; emit_file($out,   "./lib/libdtb0.c") },
    step { my ($out) = @_; emit_string($out, "#endif") },
);

open my $out1, '>', 'libdtb_singleheader.h' or die $!;
$_->($out1) for @common_steps;
$_->($out1) for @libdtb_steps;
close $out1 or die "failed to save file";

open my $out2, '>', 'libdtb0_singleheader.h' or die $!;
$_->($out2) for @common_steps;
$_->($out2) for @libdtb0_steps;
close $out2 or die "failed to save file";
