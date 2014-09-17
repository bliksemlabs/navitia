/* Copyright © 2001-2014, Canal TP and/or its affiliates. All rights reserved.
  
This file is part of Navitia,
    the software to build cool stuff with public transport.
 
Hope you'll enjoy and contribute to this project,
    powered by Canal TP (www.canaltp.fr).
Help us simplify mobility and open public transport:
    a non ending quest to the responsive locomotion way of traveling!
  
LICENCE: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
   
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.
   
You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
  
Stay tuned using
twitter @navitia 
IRC #navitia on freenode
https://groups.google.com/d/forum/navitia
www.navitia.io
*/

#pragma once
#include "type/data.h"
#include "kraken/data_manager.h"
#include "kraken/kraken_zmq.h"

#include "ed/build_helper.h"
#include <zmq.hpp>
#include "utils/zmq_compat.h"
#include <boost/thread.hpp>
#include "kraken/configuration.h"

/**
 * Pratically the same behavior as a real kraken, but with already loaded data
 */
struct mock_kraken {

    mock_kraken(ed::builder& d, const std::string& name) {
        DataManager<navitia::type::Data> data_manager;
        data_manager.current_data.reset(d.data.release());

        boost::thread_group threads;
        // Prepare our context and sockets
        zmq::context_t context(1);
        zmq::socket_t clients(context, ZMQ_ROUTER);
        const std::string zmq_socket = "ipc:///tmp/" + name;
        clients.bind(zmq_socket.c_str());
        zmq::socket_t workers(context, ZMQ_DEALER);
        workers.bind("inproc://workers");

        navitia::kraken::Configuration conf;
        // Launch only one thread for the tests
        threads.create_thread(std::bind(&doWork, std::ref(context), std::ref(data_manager), conf));

        // Connect work threads to client threads via a queue
        do {
            try {
                zmq::device(ZMQ_QUEUE, clients, workers);
            } catch(zmq::error_t){}//lors d'un SIGHUP on restore la queue
        } while(true);
    }
};
