#include "stdafx.h"

#include "TypeDefsS.h"

QX_REGISTER_CPP( CSQLComputer );

namespace qx
{
    template<> void register_class( QxClass< CSQLComputer >& t )
    {
        t.id( &CSQLComputer::id, "id" );

        t.data( &CSQLComputer::uniqueid, "uniqueid" );
        t.data( &CSQLComputer::hw_cpu_name, "hw_cpu_name" );
        t.data( &CSQLComputer::hw_board_name, "hw_board_name" );
        t.data( &CSQLComputer::hw_disk_sn, "hw_disk_sn" );
        t.data( &CSQLComputer::sw_os_name, "sw_os_name" );
        t.data( &CSQLComputer::etc_netbios_name, "etc_netbios_name" );
        t.data( &CSQLComputer::network_ipv4, "network_ipv4" );
        t.data( &CSQLComputer::network_mac, "network_mac" );

        t.data( &CSQLComputer::hw_extra_data, "hw_extra_data" );
        t.data( &CSQLComputer::sw_extra_data, "sw_extra_data" );
    }
}

