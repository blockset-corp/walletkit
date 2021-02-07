explore notes
-------------

Can be run by editing local_run.sh

The LD_PRELOAD env variable is used to either chain signals handlers in JVM in case
of wanting to add a signal handler in the native, or for introducing libasan when
running with compile/link time AddressSanitizer options

To run the explore there are 3 options:

    - testnet vs mainnet (-m for mainnet)
    - number of system instances (-n)
    - total time to run the walletmanagers until shutdown (-s)

Logs for each instance go into run/

Account dbs go locally into dbs/
