#!/usr/bin/perl

use Data::Dumper;

open(OUT, "> out.dot") || die("Can't open out.dot");

print OUT "digraph libcdeps {\n";
print OUT " ratio=0.75;\n";
print OUT " edge [color=grey70];\n";
#print OUT " nodesep=1;\n";
#print OUT " rankdir=LR;\n";
#print OUT " center=1;\n";
print OUT " root=libc_start;\n";
print OUT " overlap=scale;\n";

$nodestyle = "[shape=box fontsize=28 style=filled fillcolor=LightCyan color=blue fontname=\"Helvetica\"]";
$endnodestyle = "[shape=box fontsize=28 style=bold fontname=\"Helvetica\" color=red]";


$start="libc-start";

$verbose = 0;

%node_requires = ();
%node_provides = ();
%sym_provided_by = ();
%anti_recurse = ();

$nodes_seen = 0;

sub read_dirs {
    my $fname;
    my $nodename;
    print "Reading inputs\n";
    while (glob("deps/*/in")) {
	$fname = $_;
	$nodename = $fname;
	$nodename =~ s|^deps/||;
	$nodename =~ s|/in$||;
	open(IN, "< $fname");
	my @syms = ();
	while (<IN>) {
	    chomp;
	    push @syms, $_;
	}
	if (($#syms+1) >= 0) {
		print "$nodename has ".($#syms+1)." deps\n" if ($verbose);
  	        # OK... hash name -> array of syms
		$node_requires{$nodename} = \@syms;
	}
    }

    print "Reading outputs\n";
    while (glob("deps/*/out")) {
	$fname = $_;
	$nodename = $fname;
	$nodename =~ s|^deps/||;
	$nodename =~ s|/out$||;
	open(IN, "< $fname");
	my @syms = ();
	while (<IN>) {
	    chomp;
	    push @syms, $_;
	    $sym_provided_by{$_} = $nodename;
	}
	if (($#syms+1) > 0) {
		print "$nodename has ".($#syms+1)." syms\n" if ($verbose);
  	        # OK... hash name -> array of syms
		$node_provides{$nodename} = \@syms;
	}
    }
}

sub do_deps {
    my $nodename = shift;
    my $depth = shift;
    my @unique_deps = ();
    my %seen = ();
    my $nn = $nodename;
    $nn =~ s/-/_/g;

    print "Syms required by $nodename:\n" if ($verbose);
    foreach (@{$node_requires{$nodename}}) {
	my $by = $sym_provided_by{$_};
	if ($by) {
		print " $_  (from $by)\n" if ($verbose);
		$seen{$by}++;
	}
    }

    print "$nodename depends on: ";
    foreach (keys(%seen)) {
	my $dep_node = $_;
	print "$dep_node ";
    }
    print"\n";

    print OUT "$nn $nodestyle;\n";

    if ($depth == 0) {
	    if (keys(%seen) > 0) {
		    print OUT "$nn -> ".$nn."_deps;\n";
		    print OUT $nn."_deps $endnodestyle;\n";
	    }
	    return;
    }

    foreach (keys(%seen)) {
	my $dep_node = $_;
	my $dn = $dep_node;
	$dn =~ s/-/_/g;

	print OUT "$nn -> $dn;\n";

	if (!$anti_recurse{$dep_node}) {
		$nodes_seen++;
		# if we haven't already examined that node,
		# go look at it (and mark it examined):
	        $anti_recurse{$dep_node}++;
		do_deps($dep_node, $depth-1);
	}
    }
}

sub find_independent {
    my $independents = 0;
    foreach my $nodename (keys(%node_requires)) {
	my $depscount = 0;
	foreach my $dep (@{$node_requires{$nodename}}) {
	    print "Node $nodename requires $dep from ".$sym_provided_by{$dep}."\n" if ($verbose);
	    # This node has a dependency, but it may be self-satisfied:
	    if ($sym_provided_by{$dep} ne $nodename) {
		    # Nope.  External dep.
		    $depscount++;
	    }
	}
	if ($depscount == 0) {
		print "- Node $nodename has no external dependencies\n" if ($verbose);
		$independents++;
	}
    }
    print "$independents independent nodes found\n";
}

# Read dep. data from $PWD/deps/
read_dirs();

# Recursion depth 22 is good.  dot copes with that in finite time.  287 nodes, 1min.
# Depth 100 covers everything from libc-start.

do_deps($start, 100);
print "Seen $nodes_seen nodes\n";

print "\n";

find_independent();

print OUT "}\n";
close(OUT);