# ARP Assignment - drone simulation

This is the code for the Advanced and Robot Programming course


## Installation

this installation commands should work for Debian-based distributions, they have
been tested on Ubuntu Noble

### Install the required libraries and executables (required)

```bash
sudo apt install -y libcjson-dev libncurses-dev konsole
```

### Install the DDS library (optional)

This library is needed for the split execution required by the assigment 2.

To install follow the steps in the [official guide](https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html)

Version 3.2.2 has been used in testing

## Running assignment 1

Place yourself in the `master` branch, if you just cloned this repository you should already be here

```bash
git switch master
```

Compile the project (make sure you have all the packets listed in the section above):

```bash
make clean && make
```

Configure the project to your liking by modifying (or creating) the `appsettings.json` file.

Here is an example configuration:

```json
{
	"n_obstacles": 16,
	"n_targets": 9,

	"force_applied_N": 1.0,

	"drone_mass_kg": 1.0,
	"viscous_coefficient_Nms": 1.0,

	"max_obstacle_distance_m": 4.0,
	"min_obstacle_distance_m": 1.0,
	"obstacle_repulsion_coeff": 50.0,

	"max_target_distance_m": 4.0,
	"target_caught_distance_m": 2.0,
	"target_attraction_coeff": 1.0
}
```

be sure that:
- all configuration lines are present
- the number of targets and obstacles do not exceed the `MAX_OBSTACLES` and `MAX_TARGETS` defines
    - both currently set to 20
    - can be modified in the `obstacle.h` and `targets.h` respectively
    - do not set a number too high since it will increase the message size significantly, possibly hindering performace

Run the program with: 
```bash
./bin/spawner
```

the program will generate logs in the `simulation.log` file

⚠️ **BEWARE** it's possible that if your machine is slower than the one used in testing some processes may not respect 
their deadline and be cosidered dead by the watchdog
    you can tune process period by modifying the array `process_periods` in the `config.h` file

## Running assignment 2

Place yourself in the `ASS2` branch, if you just cloned this repository you should already be here

```bash
git switch ASS2
```

Compile the project (make sure you have all the packets listed in the section above, especially the fast-DDS library):

```bash
make clean && make
```

repeat the same operation in another folder or in another machine

Configure the project to your liking by modifying (or creating) the `appsettings.json` files.

Here is an example configuration for program 1:

```json
{
	"n_obstacles": 16,
	"n_targets": 9,

	"force_applied_N": 1.0,

	"drone_mass_kg": 1.0,
	"viscous_coefficient_Nms": 1.0,

	"max_obstacle_distance_m": 4.0,
	"min_obstacle_distance_m": 0.2,
	"obstacle_repulsion_coeff": 50.0,

	"max_target_distance_m": 4.0,
	"target_caught_distance_m": 2.0,
	"target_attraction_coeff": 1.0,

	"split_execution": true,
	"active_processes": [0, 1, 2],

	"publisher_ip" : [127, 0, 0, 1],
	"publisher_port": 11812,

	"subscriber_ip": [127, 0, 0, 1],
	"subscriber_port": 11814,

	"publisher_server_ip": [127, 0, 0, 1],
	"publisher_server_port": 11813
}	
```

Here is an example configuration for program 2:
```json
{
	"n_obstacles": 16,
	"n_targets": 9,

	"force_applied_N": 1.0,

	"drone_mass_kg": 1.0,
	"viscous_coefficient_Nms": 1.0,

	"max_obstacle_distance_m": 4.0,
	"min_obstacle_distance_m": 0.2,
	"obstacle_repulsion_coeff": 50.0,

	"max_target_distance_m": 3.0,
	"target_caught_distance_m": 1.0,
	"target_attraction_coeff": 1.0,

	"split_execution": true,
	"active_processes": [3, 4],

	"publisher_ip" : [127, 0, 0, 1],
	"publisher_port": 11813,

	"subscriber_ip": [127, 0, 0, 1],
	"subscriber_port": 11815,

	"publisher_server_ip": [127, 0, 0, 1],
	"publisher_server_port": 11812
}
```

as you can see from the ip set both the programs are expected to be run from the same PC
    if you want them to run from different IPs set the ip accordingly
        remember that `publisher_ip` and `subscriber_ip` should be the ip of the PC the program is run in, while `publisher_server_ip` should be the ip of the other PC

`active_processes` specifies which processes should run on the configured PC
    be sure that each of the processes (0 to 4) are on one and only one PC

All rules for the Assignment 1 configuration are aplied here as well

**Note**: A configuration with: `"active_processes": [0, 1, 2, 3, 4]` will act exactly like Assignent 1