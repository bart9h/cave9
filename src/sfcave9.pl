#!/usr/bin/perl
use strict;
$\="\n";

# settings
my $max_time = 10000;
my $min_cave_height = 0;
my $max_cave_height = 30;
my $max_screen_height = $ENV{COLUMNS} - 15;
my $cave_change_rate = .2;
my $accel = .2;
my $stala_rate = .1;
my $stala_size = 3;
my $max_velocity = 1;

# vars
my $key = 0;
my $velocity = 0;
my $time = 1;
my $cave_height = int rand $max_cave_height;
my @tunnel;
                                    
$tunnel[0] = int rand $max_screen_height-1;

sub skill() {
	(log(($time+200)/200));
}

my $repeat = 0;
my $cave_change = 1;

sub key_state(){
	$repeat = 0 if $repeat * $velocity < 0;
	$repeat += $velocity/1000;
	
	my $dir = $key ? -1: 1;
	$key = !$key if 
		(rand() < abs($velocity/skill()/3) and $velocity * $dir < 0)
		|| (rand() < abs($repeat * skill()) and $repeat * $dir < 0);
	
	$max_cave_height = 10+30/(1+skill());
	$min_cave_height = $max_cave_height/skill();
	
	 $cave_change != $cave_change if
		(rand() > $cave_change_rate/skill()); 

		$cave_change != $cave_change,
		$cave_height = $min_cave_height
		if $cave_height < $min_cave_height;
		
		$cave_change != $cave_change,
		$cave_height = $max_cave_height
		if $cave_height > $max_cave_height;
		
		$cave_height += rand()>.5?-1:1 if $cave_change;
}

sub bound($){
	$_ = shift;
	$_ = $max_screen_height if $_ < 0;
	$_ = 0 if $_ > $max_screen_height;
	$_;
}

sub next_height(){
	key_state();
	$velocity += ($key?1:-1)*$accel;
	$tunnel[$time-1]+= $velocity;
}

sub hud() {
	sprintf ("%6.3f ",skill);
}

sub cave() {
	my $ceil = $max_screen_height-$tunnel[$time]-1-$cave_height;
	'#'x$tunnel[$time] , ' 'x$cave_height , ($ceil > 0 ? '#'x $ceil : '');
}

for ($time = 1; $time <= $max_time; $time++) {	
	$tunnel[$time] = bound(next_height());
	print hud(),cave();
	use Time::HiRes qw/sleep/;
	sleep .02;
}

