#!/usr/bin/env python3
#
#  Copyright (c) 2004-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the BSD-style license found in the
#  LICENSE file in the root directory of this source tree. An additional grant
#  of patent rights can be found in the PATENTS file in the same directory.
#

from fboss.cli.commands import commands as cmds
from fboss.cli.utils import utils


class L2TableCmd(cmds.FbossCmd):
    def run(self):
        self._client = self._create_agent_client()
        resp = self._client.getL2Table()
        port_map = self._client.getAllPortInfo()

        if not resp:
            print("No L2 Entries Found")
            return
        resp = sorted(resp, key=lambda x: (x.port, x.vlanID, x.mac))
        tmpl = "{:18} {:>17}  {}"

        print(tmpl.format("MAC Address", "Port/Trunk", "VLAN"))
        for entry in resp:
            if entry.trunk:
                port_data = f"{entry.trunk} (Trunk)"
            else:
                port_info = port_map.get(entry.port, None)
                if not port_info:
                    # Skip ports for which we could not lookup port_info
                    # This is typically the entries associated with CPU
                    # port, all of which just point to the MAC address
                    # we assigned to they configured vlans.
                    continue
                port_data = port_info.name if port_info.name else entry.port

            print(tmpl.format(entry.mac, port_data, entry.vlanID))
