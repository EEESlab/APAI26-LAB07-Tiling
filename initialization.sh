#!/bin/bash
# Prepares the lab. The Makefile runs it before every build, so you do not need to run it by hand.
# It only acts when something is missing, so running it again is harmless.

(
cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1

# Lab machines (virtual machine) only: set the memories of the simulated PULP chip (64kB of L1).
# The devcontainer simulator already has the right memory sizes.
VM_PULP_JSON=/workspaces/APAI-Docker/pulp-sdk/tools/gap-configs/configs/chips/pulp/pulp.json
if [ -f "$VM_PULP_JSON" ] && ! cmp -s patch/pulp.json "$VM_PULP_JSON"; then
    echo "Patching the PULP SDK memory configuration"
    cp patch/pulp.json "$VM_PULP_JSON"
fi
)
