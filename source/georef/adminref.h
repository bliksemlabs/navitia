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
#include <unordered_map>
#include "type/type.h"



namespace nt = navitia::type;

namespace navitia {

    namespace georef {
        typedef boost::geometry::model::polygon<navitia::type::GeographicalCoord> polygon_type;

        struct Admin : nt::Header, nt::Nameable {
            /**
              http://wiki.openstreetmap.org/wiki/Key:admin_level#admin_level
              Level = 2  : Pays
              Level = 4  : Région
              Level = 6  : Département
              Level = 8  : Commune
              Level = 10 : Quartier
            */
            int level;
            std::string post_code;
            std::string insee;
            nt::GeographicalCoord coord;
            polygon_type boundary;
            std::vector<const Admin*> admin_list;
            std::vector<const nt::StopArea*> main_stop_areas;

            Admin():level(-1){}
            Admin(int lev):level(lev){}
            template<class Archive> void serialize(Archive & ar, const unsigned int ) {
                ar & idx & level & post_code & insee & name & uri & coord & admin_list & main_stop_areas;
            }
        };
    }
}
