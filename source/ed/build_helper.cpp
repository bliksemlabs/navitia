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

#include "build_helper.h"
#include "ed/connectors/gtfs_parser.h"

namespace pt = boost::posix_time;

namespace ed {

VJ & VJ::frequency(uint32_t start_time, uint32_t end_time, uint32_t headway_secs) {
    navitia::type::StopTime* first_st = vj->stop_time_list.front();
    size_t nb_trips = std::ceil((end_time - start_time)/headway_secs);
    for(navitia::type::StopTime* st : vj->stop_time_list) {
        st->set_is_frequency(true);
        st->start_time = start_time+(st->arrival_time - first_st->arrival_time);
        st->end_time = start_time + nb_trips * headway_secs + (st->departure_time - first_st->departure_time);
        st->headway_secs = headway_secs;
    }
    return *this;
}


VJ::VJ(builder & b, const std::string &line_name, const std::string &validity_pattern, const std::string &/*block_id*/, bool wheelchair_boarding, const std::string& uri) : b(b){
    vj = new navitia::type::VehicleJourney();
    vj->idx = b.data->pt_data->vehicle_journeys.size();
    b.data->pt_data->vehicle_journeys.push_back(vj);

    auto it = b.lines.find(line_name);
    if(it == b.lines.end()){

        navitia::type::Line* line = new navitia::type::Line();
        line->idx = b.data->pt_data->lines.size();
        line->uri = line_name;
        b.lines[line_name] = line;
        line->name = line_name;
        b.data->pt_data->lines.push_back(line);
        navitia::type::Route* route = new navitia::type::Route();
        route->idx = b.data->pt_data->routes.size();
        route->uri = line_name + ":" + std::to_string(b.data->pt_data->routes.size());
        b.data->pt_data->routes.push_back(route);
        line->route_list.push_back(route);
        route->line = line;
        navitia::type::JourneyPattern* jp = new navitia::type::JourneyPattern();
        jp->idx = b.data->pt_data->journey_patterns.size();
        jp->uri = route->uri + ":0";
        b.data->pt_data->journey_patterns.push_back(jp);
        route->journey_pattern_list.push_back(jp);
        jp->route = route;
        //add physical mode
        if (! b.data->pt_data->physical_modes.empty()) {
            auto mode = b.data->pt_data->physical_modes.front();
            jp->physical_mode = mode;
        }
        else {
            jp->physical_mode = new navitia::type::PhysicalMode();
            jp->physical_mode->idx = b.data->pt_data->physical_modes.size();
            jp->physical_mode->uri = "physical_mode:0";
            b.data->pt_data->physical_modes.push_back(jp->physical_mode);
        }
        jp->physical_mode->journey_pattern_list.push_back(jp);

        vj->journey_pattern = jp;
    } else {
        //@TODO va surement falloir créer un nouveau journeypattern
        //vj-> = it->second;
        vj->journey_pattern = it->second->route_list.front()->journey_pattern_list.front();
    }
    vj->journey_pattern->vehicle_journey_list.push_back(vj);

    nt::ValidityPattern* vp = new nt::ValidityPattern(b.begin, validity_pattern);
    auto find_vp_predicate = [&](nt::ValidityPattern* vp1) { return vp->days == vp1->days;};
    auto it_vp = std::find_if(b.data->pt_data->validity_patterns.begin(),
                        b.data->pt_data->validity_patterns.end(), find_vp_predicate);
    if(it_vp != b.data->pt_data->validity_patterns.end()) {
        delete vp;
        vj->validity_pattern = *(it_vp);
    } else {
         b.data->pt_data->validity_patterns.push_back(vp);
         vj->validity_pattern = vp;
    }
    b.vps[validity_pattern] = vj->validity_pattern;
    vj->adapted_validity_pattern = vj->validity_pattern;


        if(wheelchair_boarding){
            vj->set_vehicle(navitia::type::hasVehicleProperties::WHEELCHAIR_ACCESSIBLE);
        }
    vj->uri = uri;
    if(!b.data->pt_data->physical_modes.empty())
        if (vj->journey_pattern != NULL)
            vj->journey_pattern->physical_mode = b.data->pt_data->physical_modes.front();

    if(!b.data->pt_data->companies.empty())
        vj->company = b.data->pt_data->companies.front();
}


VJ& VJ::operator()(const std::string &stopPoint,const std::string& arrivee, const std::string& depart,
            uint16_t local_traffic_zone, bool drop_off_allowed, bool pick_up_allowed){
    return (*this)(stopPoint, pt::duration_from_string(arrivee).total_seconds(),
            pt::duration_from_string(depart).total_seconds(), local_traffic_zone,
            drop_off_allowed, pick_up_allowed);
}

VJ & VJ::operator()(const std::string & sp_name, int arrivee, int depart, uint16_t local_trafic_zone,
                    bool drop_off_allowed, bool pick_up_allowed){
    navitia::type::StopTime* st = new navitia::type::StopTime();
    auto it = b.sps.find(sp_name);
    navitia::type::StopPoint* sp = nullptr;
    navitia::type::JourneyPatternPoint* jpp = nullptr;
    if(it == b.sps.end()){
        sp = new navitia::type::StopPoint();
        sp->idx = b.data->pt_data->stop_points.size();
        sp->name = sp_name;
        sp->uri = sp_name;
        if(!b.data->pt_data->networks.empty())
            sp->network = b.data->pt_data->networks.front();

        b.sps[sp_name] = sp;
        b.data->pt_data->stop_points.push_back(sp);
        auto sa_it = b.sas.find(sp_name);
        if(sa_it == b.sas.end()) {
            navitia::type::StopArea* sa = new navitia::type::StopArea();
            sa->idx = b.data->pt_data->stop_areas.size();
            sa->name = sp_name;
            sa->uri = sp_name;
            sa->set_property(navitia::type::hasProperties::WHEELCHAIR_BOARDING);
            sp->stop_area = sa;
            b.sas[sp_name] = sa;
            b.data->pt_data->stop_areas.push_back(sa);
            sp->set_properties(sa->properties());
            sa->stop_point_list.push_back(sp);
        } else {
            sp->stop_area = sa_it->second;
            sp->coord.set_lat(sp->stop_area->coord.lat());
            sp->coord.set_lon(sp->stop_area->coord.lon());
            sp->set_properties(sa_it->second->properties());
            sa_it->second->stop_point_list.push_back(sp);
        }
    } else {
        sp = it->second;
        auto find_jpp = std::find_if(sp->journey_pattern_point_list.begin(),
                                     sp->journey_pattern_point_list.end(),
                                     [&](navitia::type::JourneyPatternPoint* jpp){return jpp->journey_pattern == vj->journey_pattern;});
        if(find_jpp != sp->journey_pattern_point_list.end())
            jpp = *find_jpp;
    }
    if(jpp == nullptr) {
        jpp = new navitia::type::JourneyPatternPoint();
        jpp->idx = b.data->pt_data->journey_pattern_points.size();
        vj->journey_pattern->journey_pattern_point_list.push_back(jpp);
        jpp->stop_point = sp;
        sp->journey_pattern_point_list.push_back(jpp);
        jpp->journey_pattern = vj->journey_pattern;
        b.data->pt_data->journey_pattern_points.push_back(jpp);
    }
    //on construit un nouveau journey pattern point à chaque fois
    st->journey_pattern_point = jpp;

    if(depart == -1) depart = arrivee;
    st->arrival_time = arrivee;
    st->departure_time = depart;
    st->vehicle_journey = vj;
    jpp->order = vj->stop_time_list.size();
    st->local_traffic_zone = local_trafic_zone;
    jpp->drop_off_allowed = drop_off_allowed;
    jpp->pick_up_allowed = pick_up_allowed;

    vj->stop_time_list.push_back(st);
    b.data->pt_data->stop_times.push_back(st);
    return *this;
}

SA::SA(builder & b, const std::string & sa_name, double x, double y, bool wheelchair_boarding) : b(b) {
    sa = new navitia::type::StopArea();
    sa->idx = b.data->pt_data->stop_areas.size();
    b.data->pt_data->stop_areas.push_back(sa);
    sa->name = sa_name;
    sa->uri = sa_name;
    sa->coord.set_lon(x);
    sa->coord.set_lat(y);
    if(wheelchair_boarding)
        sa->set_property(types::hasProperties::WHEELCHAIR_BOARDING);
    b.sas[sa_name] = sa;

    auto sp = new navitia::type::StopPoint();
    sp->idx = b.data->pt_data->stop_points.size();
    b.data->pt_data->stop_points.push_back(sp);
    sp->name = "stop_point:"+ sa_name;
    sp->uri = sp->name;
    if(wheelchair_boarding)
        sp->set_property(navitia::type::hasProperties::WHEELCHAIR_BOARDING);
    sp->coord.set_lon(x);
    sp->coord.set_lat(y);

    sp->stop_area = sa;
    b.sps[sp->name] = sp;
    sa->stop_point_list.push_back(sp);
}

SA & SA::operator()(const std::string & sp_name, double x, double y, bool wheelchair_boarding){
    navitia::type::StopPoint * sp = new navitia::type::StopPoint();
    sp->idx = b.data->pt_data->stop_points.size();
    b.data->pt_data->stop_points.push_back(sp);
    sp->name = sp_name;
    sp->uri = sp_name;
    if(wheelchair_boarding)
        sp->set_property(navitia::type::hasProperties::WHEELCHAIR_BOARDING);
    sp->coord.set_lon(x);
    sp->coord.set_lat(y);

    sp->stop_area = this->sa;
    b.sps[sp_name] = sp;
    return *this;
}


VJ builder::vj(const std::string &line_name, const std::string &validity_pattern, const std::string & block_id, const bool wheelchair_boarding, const std::string& uri){
    return vj("base_network", line_name, validity_pattern, block_id, wheelchair_boarding, uri);
}

VJ builder::vj(const std::string &network_name, const std::string &line_name,
               const std::string &validity_pattern, const std::string & block_id,
               const bool wheelchair_boarding, const std::string& uri){
    auto res = VJ(*this, line_name, validity_pattern, block_id, wheelchair_boarding, uri);
    auto vj = this->data->pt_data->vehicle_journeys.back();
    auto it = this->nts.find(network_name);
    if(it == this->nts.end()){
        navitia::type::Network* network = new navitia::type::Network();
        network->idx = this->data->pt_data->networks.size();
        network->uri = network_name;
        network->name = network_name;
        this->nts[network_name] = network;
        this->data->pt_data->networks.push_back(network);
        vj->journey_pattern->route->line->network = network;
    } else {
        vj->journey_pattern->route->line->network = it->second;
    }
    if(block_id != "") {
        block_vjs.insert(std::make_pair(block_id, vj));
    }
    return res;
}

SA builder::sa(const std::string &name, double x, double y, const bool wheelchair_boarding){
    return SA(*this, name, x, y, wheelchair_boarding);
}


void builder::connection(const std::string & name1, const std::string & name2, float length) {
    navitia::type::StopPointConnection* connexion = new navitia::type::StopPointConnection();
    connexion->idx = data->pt_data->stop_point_connections.size();
    if(sps.count(name1) == 0 || sps.count(name2) == 0)
        return ;
    connexion->departure = (*(sps.find(name1))).second;
    connexion->destination = (*(sps.find(name2))).second;

//connexion->connection_kind = types::ConnectionType::Walking;
    connexion->duration = length;
    connexion->display_duration = length;

    data->pt_data->stop_point_connections.push_back(connexion);
}

 void builder::generate_dummy_basis() {
    navitia::type::Company *company = new navitia::type::Company();
    company->idx = this->data->pt_data->companies.size();
    company->name = "base_company";
    company->uri = "base_company";
    this->data->pt_data->companies.push_back(company);

    navitia::type::Network *network = new navitia::type::Network();
    network->idx = this->data->pt_data->networks.size();
    network->name = "base_network";
    network->uri = "base_network";
    this->data->pt_data->networks.push_back(network);
    this->nts.insert({network->uri, network});

    navitia::type::CommercialMode *commercial_mode = new navitia::type::CommercialMode();
    commercial_mode->idx = this->data->pt_data->commercial_modes.size();
    commercial_mode->name = "Tram";
    commercial_mode->uri = "0x0";
    this->data->pt_data->commercial_modes.push_back(commercial_mode);

    commercial_mode = new navitia::type::CommercialMode();
    commercial_mode->idx = this->data->pt_data->commercial_modes.size();
    commercial_mode->name = "Metro";
    commercial_mode->uri = "0x1";
    this->data->pt_data->commercial_modes.push_back(commercial_mode);

    for(navitia::type::CommercialMode *mt : this->data->pt_data->commercial_modes) {
        navitia::type::PhysicalMode* mode = new navitia::type::PhysicalMode();
        mode->idx = mt->idx;
        mode->name = mt->name;
        mode->uri = mt->uri;
        this->data->pt_data->physical_modes.push_back(mode);
    }
 }

 void builder::build(navitia::type::PT_Data & /*pt_data*/) {
    /*data.normalize_uri();
    data.complete();
    data.clean();
    data.sort();
    //data.transform(pt_data);
    pt_data->build_uri();*/
}

 void builder::build_relations(navitia::type::PT_Data & data){
     for(navitia::type::StopPoint* sp : data.stop_points){
         sp->journey_pattern_point_list.clear();
     }
     for(navitia::type::JourneyPatternPoint* jpp : data.journey_pattern_points){
         jpp->stop_point->journey_pattern_point_list.push_back(jpp);
     }
 }


 void builder::build_blocks() {
     std::string prev_block = "";

     std::vector<navitia::type::VehicleJourney*> vehicle_journeys;
     for(auto block_vj : block_vjs) {
         if(prev_block != "" && prev_block != block_vj.first) {
             vehicle_journeys.resize(0);
         } else {
             vehicle_journeys.push_back(block_vj.second);
         }
         prev_block = block_vj.first;
     }

     std::sort(vehicle_journeys.begin(), vehicle_journeys.end(),
             [](const navitia::type::VehicleJourney* vj1, const navitia::type::VehicleJourney* vj2) {
             return vj1->stop_time_list.back()->arrival_time <=
                         vj2->stop_time_list.front()->departure_time;

             }
      );

     navitia::type::VehicleJourney* prev_vj = nullptr;
     for(auto it=vehicle_journeys.begin(); it!=vehicle_journeys.end(); ++it) {
         auto vj = *it;
         if(prev_vj) {
             prev_vj->next_vj = vj;
             vj->prev_vj = prev_vj;
         }
         prev_vj = vj;
     }
 }

 void builder::finish() {
     build_blocks();
     for(navitia::type::VehicleJourney* vj : this->data->pt_data->vehicle_journeys) {
         if(!vj->prev_vj) {
             vj->stop_time_list.front()->journey_pattern_point->drop_off_allowed = false;
         }
         if(!vj->next_vj) {
            vj->stop_time_list.back()->journey_pattern_point->pick_up_allowed = false;
         }
     }
 }

/*
1. Initilise the first admin in the list to all stop_area and way
2. Used for the autocomplete functional tests.
*/
 void builder::manage_admin() {
     if (!data->geo_ref->admins.empty()) {
         navitia::georef::Admin * admin = data->geo_ref->admins[0];
        for(navitia::type::StopArea* sa : data->pt_data->stop_areas){
            sa->admin_list.clear();
            sa->admin_list.push_back(admin);
        }

        for(navitia::georef::Way * way : data->geo_ref->ways) {
            way->admin_list.clear();
            way->admin_list.push_back(admin);
        }
     }
 }

 void builder::build_autocomplete() {
    data->pt_data->build_autocomplete(*(data->geo_ref));
    data->geo_ref->build_autocomplete_list();
 }
}
