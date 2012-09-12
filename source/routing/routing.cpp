#include "routing.h"

namespace navitia { namespace routing {

DateTime DateTime::inf = DateTime::infinity();
DateTime DateTime::min = DateTime::minimity();

std::ostream & operator<<(std::ostream & os, const DateTime & dt){
    os << "D=" << dt.date() << " " << dt.hour()/(3600) << ":";
    if((dt.hour()%3600)/60 < 10)
        os << "0" << (dt.hour()%3600)/60;
    else
        os << (dt.hour()%3600)/60;
    return os;
}


std::string PathItem::print(const navitia::type::PT_Data & data) const {
    std::stringstream ss;

    if(stop_points.size() < 2) {
        ss << "Section avec moins de 2 stop points, hrmmmm \n";
        return ss.str();
    }

    navitia::type::StopArea start = data.stop_areas[data.stop_points[stop_points.front()].stop_area_idx];
    navitia::type::StopArea dest = data.stop_areas[data.stop_points[stop_points.back()].stop_area_idx];
    ss << "Section de type " << (type == public_transport ? "transport en commun" : "marche") << "\n";

    if(type == public_transport && vj_idx != navitia::type::invalid_idx){
        const navitia::type::VehicleJourney & vj = data.vehicle_journeys[vj_idx];
        const navitia::type::Route & route = data.routes[vj.route_idx];
        const navitia::type::Line & line = data.lines[route.line_idx];
        ss << "Ligne : " << line.name  << " (" << line.external_code << " " << line.idx << "), "
           << "Route : " << route.name << " (" << route.external_code << " " << route.idx << "), "
           << "Vehicle journey " << vj_idx << "\n";
    }
    ss << "Départ de " << start.name << "(" << start.external_code << " " << start.idx << ") à " << departure << "\n";
    for(auto sp_idx : stop_points){
        navitia::type::StopPoint sp = data.stop_points[sp_idx];
        ss << "    " << sp.name << " (" << sp.external_code << " " << sp.idx << ")" << "\n";
    }
    ss << "Arrivée à " << dest.name << "(" << dest.external_code << " " << dest.idx << ") à " << arrival << "\n";
    return ss.str();
}

bool Verification::verif(Path path) {
    return  croissance(path) && vj_valides(path) && appartenance_rp(path) && check_correspondances(path);
}

bool Verification::croissance(Path path) {
    DateTime precdt = DateTime::min;
    for(PathItem item : path.items) {
        if(precdt > item.departure) {
            std::cout << "Erreur dans la vérification de la croissance des horaires : " << precdt  << " >  " << item.departure << std::endl;
            return false;
        }
        if(item.departure > item.arrival) {
            std::cout << "Erreur dans la vérification de la croissance des horaires : " << item.departure<< " >  "   << item.arrival  << std::endl;
            return false;
        }
        precdt = item.arrival;
    }
    return true;
}

bool Verification::vj_valides(Path path) {
    for(PathItem item : path.items) {
        if(item.type == public_transport) {
            if(!data.validity_patterns[data.vehicle_journeys[item.vj_idx].validity_pattern_idx].check(item.departure.date())) {
                std::cout << " le vj : " << item.vj_idx << " n'est pas valide le jour : " << item.departure.date() << std::endl;
                return false;
            }
        }

    }
    return true;
}

bool Verification::appartenance_rp(Path path) {
    for(PathItem item : path.items) {

        if(item.type == public_transport) {
            const navitia::type::VehicleJourney & vj = data.vehicle_journeys[item.vj_idx];

            for(auto spidx : item.stop_points) {
                if(std::find_if(vj.stop_time_list.begin(), vj.stop_time_list.end(),
                                [&](int stidx){ return (data.route_points[data.stop_times[stidx].route_point_idx].stop_point_idx == spidx);}) == vj.stop_time_list.end()) {
                    std::cout << "Le stop point : " << spidx << " n'appartient pas au vj : " << item.vj_idx << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

bool Verification::check_correspondances(Path path) {
    std::vector<navitia::type::Connection> stop_point_list;

    PathItem precitem;
    for(PathItem item : path.items) {
        navitia::type::Connection conn;
        if(item.type == walking) {
            conn.departure_stop_point_idx = item.stop_points.front();
            conn.destination_stop_point_idx =  item.stop_points.back();
            conn.duration = item.arrival - item.departure;
            stop_point_list.push_back(conn);
        }
        if(precitem.arrival != DateTime::inf) {
            if(precitem.type == public_transport && item.type == public_transport) {
                conn.departure_stop_point_idx = precitem.stop_points.back();
                conn.destination_stop_point_idx =  item.stop_points.front();
                conn.duration = item.departure - precitem.arrival;
                stop_point_list.push_back(conn);
            }
        }

        precitem = item;
    }

    for(navitia::type::Connection conn: data.connections) {

        auto it = std::find_if(stop_point_list.begin(), stop_point_list.end(),
                               [&](navitia::type::Connection p){return (p.departure_stop_point_idx == conn.departure_stop_point_idx && p.destination_stop_point_idx == conn.destination_stop_point_idx)
                               ||(p.destination_stop_point_idx == conn.departure_stop_point_idx && p.departure_stop_point_idx == conn.destination_stop_point_idx);});


    if(it != stop_point_list.end()) {
        if(it->duration != conn.duration) {
            std::cout << "Le temps de correspondance de la connection " << data.stop_points[it->departure_stop_point_idx].name << "(" << it->departure_stop_point_idx << ") -> "
                      << data.stop_points[it->destination_stop_point_idx].name << "(" << it->destination_stop_point_idx << ") ne correspond pas aux données : "
                      <<  it->duration << " != " <<  conn.duration << std::endl;
            return false;
        } else {
            stop_point_list.erase(it);
        }
    }

    if(stop_point_list.size() == 0)
        return true;
}
bool toreturn = true;
for(auto psp : stop_point_list) {
    if(data.stop_points[psp.departure_stop_point_idx].stop_area_idx != data.stop_points[psp.destination_stop_point_idx].stop_area_idx) {
        std::cout << "La correspondance " << psp.departure_stop_point_idx << " => " << psp.destination_stop_point_idx << " n'existe pas " << std::endl;
        toreturn = false;
    }

}
return toreturn;
}



                  }}