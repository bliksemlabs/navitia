#include "raptor_init.h"
namespace navitia { namespace routing {

std::vector<Departure_Type>
getDepartures(const std::vector<std::pair<type::idx_t, double> > &departs, const std::vector<std::pair<type::idx_t, double> > &destinations,
              bool clockwise, const float walking_speed, const std::vector<label_vector_t> &labels, const std::vector<std::vector< const type::JourneyPatternPoint*> > &boardings,
              const std::vector<std::vector<boarding_type> > &boarding_types,/* const type::Properties &required_properties*/
              const type::AccessibiliteParams & accessibilite_params, const type::Data &data) {
      std::vector<Departure_Type> result;

      auto pareto_front = getParetoFront(clockwise, departs, destinations, walking_speed, labels, boardings, boarding_types, accessibilite_params/*required_properties*/, data);
      result.insert(result.end(), pareto_front.begin(), pareto_front.end());

      if(!pareto_front.empty()) {
          auto walking_solutions = getWalkingSolutions(clockwise, departs, destinations, pareto_front.back(), walking_speed, labels, boardings, boarding_types, data);

          for(auto s : walking_solutions) {
              bool find = false;
              for(auto s2 : result) {
                  if(s.rpidx == s2.rpidx && s.count == s2.count) {
                      find = true;
                      break;
                  }
              }
              if(!find) {
                  result.push_back(s);
              }
          }
      }


      return result;
}


std::vector<Departure_Type>
getDepartures(const std::vector<std::pair<type::idx_t, double> > &departs, const DateTime &dep, bool clockwise, const float walking_speed, const type::Data & data) {
    std::vector<Departure_Type> result;
    for(auto dep_dist : departs) {
        for(auto journey_pattern : data.pt_data.stop_points[dep_dist.first]->journey_pattern_point_list) {
            Departure_Type d;
            d.count = 0;
            d.rpidx = journey_pattern->idx;
            d.walking_time = dep_dist.second/walking_speed;
            if(clockwise)
                d.arrival = dep + d.walking_time;
            else
                d.arrival = dep - d.walking_time;
            result.push_back(d);
        }
    }
    return result;
}

// Does the current date improves compared to best_so_far – we must not forget to take the walking duration
bool improves(const DateTime & best_so_far, bool clockwise, const DateTime & current, int walking_duration) {
    if(clockwise) {
        return current - walking_duration > best_so_far;
    } else {
        return current + walking_duration < best_so_far;
    }
}

std::vector<Departure_Type>
getParetoFront(bool clockwise, const std::vector<std::pair<type::idx_t, double> > &departs, const std::vector<std::pair<type::idx_t, double> > &destinations,
               const float walking_speed, const std::vector<label_vector_t> &labels, const std::vector<std::vector<const type::JourneyPatternPoint*> > &boardings,
               const std::vector<std::vector<boarding_type> >&boarding_types,
               const type::AccessibiliteParams & accessibilite_params/*const type::Properties &required_properties*/, const type::Data &data){
    std::vector<Departure_Type> result;

    DateTime best_dt, best_dt_jpp;
    if(clockwise) {
        best_dt = DateTimeUtils::min;
        best_dt_jpp = DateTimeUtils::min;
    } else {
        best_dt = DateTimeUtils::inf;
        best_dt_jpp = DateTimeUtils::inf;
    }
    for(unsigned int round=1; round < labels.size(); ++round) {
        // For every round with look for the best journey pattern point that belongs to one of the destination stop points
        // We must not forget to walking duration
        type::idx_t best_jpp = type::invalid_idx;
        for(auto spid_dist : destinations) {
            for(auto journey_pattern_point : data.pt_data.stop_points[spid_dist.first]->journey_pattern_point_list) {
                type::idx_t jppidx = journey_pattern_point->idx;
                auto type = get_type(round, jppidx, boarding_types, data);
                if((type != boarding_type::uninitialized) &&
                   labels[round][jppidx] != DateTimeUtils::inf &&
                   labels[round][jppidx] != DateTimeUtils::min &&
                   improves(best_dt, clockwise, labels[round][jppidx], spid_dist.second/walking_speed) ) {
                    best_jpp = jppidx;
                    best_dt_jpp = labels[round][jppidx];
                    if(type == boarding_type::vj) {
                        // Dans le sens horaire : lors du calcul on gardé que l’heure de départ, mais on veut l’arrivée
                        // Il faut donc retrouver le stop_time qui nous intéresse avec best_stop_time
                        const type::StopTime* st;
                        uint32_t gap;

                        std::tie(st, gap) = best_stop_time(journey_pattern_point, labels[round][jppidx], accessibilite_params/*required_properties*/, !clockwise, data, true);
                        if(clockwise) {
                            DateTimeUtils::update(best_dt_jpp, st->arrival_time + gap, !clockwise);
                        } else {
                            DateTimeUtils::update(best_dt_jpp, st->departure_time + gap, !clockwise);
                        }
                    }
                    if(clockwise)
                        best_dt = best_dt_jpp - (spid_dist.second/walking_speed);
                    else
                        best_dt = best_dt_jpp + (spid_dist.second/walking_speed);
                }
            }
        }
        if(best_jpp != type::invalid_idx) {
            Departure_Type s;
            s.rpidx = best_jpp;
            s.count = round;
            s.walking_time = getWalkingTime(round, best_jpp, departs, destinations, clockwise, labels, boardings, boarding_types, data);
            s.arrival = best_dt_jpp;
            type::idx_t final_rpidx;
            std::tie(final_rpidx, s.upper_bound) = getFinalRpidAndDate(round, best_jpp, clockwise, labels, boardings, boarding_types, data);
            for(auto spid_dep : departs) {
                if(data.pt_data.journey_pattern_points[final_rpidx]->stop_point->idx == spid_dep.first) {
                    if(clockwise) {
                        s.upper_bound = s.upper_bound + (spid_dep.second/walking_speed);
                    }else {
                        s.upper_bound = s.upper_bound - (spid_dep.second/walking_speed);
                    }
                }
            }
            result.push_back(s);
        }
    }

    return result;
}





std::vector<Departure_Type>
getWalkingSolutions(bool clockwise, const std::vector<std::pair<type::idx_t, double> > &departs, const std::vector<std::pair<type::idx_t, double> > &destinations, Departure_Type best,
                    const float walking_speed, const std::vector<label_vector_t> &labels, const std::vector<std::vector<const type::JourneyPatternPoint*> >&boardings,
                    const std::vector<std::vector<boarding_type> > &boarding_types, const type::Data &data){
    std::vector<Departure_Type> result;

    std::/*unordered_*/map<type::idx_t, Departure_Type> tmp;

    for(uint32_t i=0; i<labels.size(); ++i) {
        for(auto spid_dist : destinations) {
            Departure_Type best_departure;
            best_departure.ratio = 2;
            best_departure.rpidx = type::invalid_idx;
            for(auto journey_pattern_point : data.pt_data.stop_points[spid_dist.first]->journey_pattern_point_list) {
                type::idx_t jppidx = journey_pattern_point->idx;
                if(get_type(i, jppidx, boarding_types, data) != boarding_type::uninitialized) {
                    float lost_time;
                    if(clockwise)
                        lost_time = labels[i][jppidx]-(spid_dist.second/walking_speed) - best.arrival;
                    else
                        lost_time = labels[i][jppidx]+(spid_dist.second/walking_speed) - best.arrival;

                    float walking_time = getWalkingTime(i, jppidx, departs, destinations, clockwise, labels, boardings, boarding_types, data);

                    //Si je gagne 5 minutes de marche a pied, je suis pret à perdre jusqu'à 10 minutes.
                    if(walking_time < best.walking_time && (lost_time/(best.walking_time-walking_time)) < best_departure.ratio) {
                        Departure_Type s;
                        s.rpidx = jppidx;
                        s.count = i;
                        s.ratio = lost_time/(best.walking_time-walking_time);
                        s.walking_time = walking_time;
//                        if(clockwise)
                            s.arrival = labels[i][jppidx];
//                        else
//                            s.arrival = labels[i][rpidx];
                        type::idx_t final_rpidx;
                        DateTime last_time;
                        std::tie(final_rpidx, last_time) = getFinalRpidAndDate(i, jppidx, clockwise, labels, boardings, boarding_types, data);
                        if(clockwise) {
                            s.upper_bound = last_time;
                            for(auto spid_dep : departs) {
                                if(data.pt_data.journey_pattern_points[final_rpidx]->stop_point->idx == spid_dep.first) {
                                    s.upper_bound = s.upper_bound + (spid_dep.second/walking_speed);
                                }
                            }
                        } else {
                            s.upper_bound = last_time;
                            for(auto spid_dep : departs) {
                                if(data.pt_data.journey_pattern_points[final_rpidx]->stop_point->idx == spid_dep.first) {
                                    s.upper_bound = s.upper_bound - (spid_dep.second/walking_speed);
                                }
                            }
                        }

                        best_departure = s;
                    }
                }
            }
            if(best_departure.rpidx != type::invalid_idx) {
                type::idx_t journey_pattern = data.pt_data.journey_pattern_points[best_departure.rpidx]->journey_pattern->idx;
                if(tmp.find(journey_pattern) == tmp.end()) {
                    tmp.insert(std::make_pair(journey_pattern, best_departure));
                } else if(tmp[journey_pattern].ratio > best_departure.ratio) {
                    tmp[journey_pattern] = best_departure;
                }
            }

        }
    }


    for(auto p : tmp) {
        result.push_back(p.second);
    }
    std::sort(result.begin(), result.end(), [](Departure_Type s1, Departure_Type s2) {return s1.ratio > s2.ratio;});

    if(result.size() > 2)
        result.resize(2);

    return result;
}


// Reparcours l’itinéraire rapidement pour avoir le JPP et la date de départ (si on cherchait l’arrivée au plus tôt)
std::pair<type::idx_t, DateTime>
getFinalRpidAndDate(int count, type::idx_t rpid, bool clockwise, const std::vector<label_vector_t> &labels,
                    const std::vector<std::vector<const type::JourneyPatternPoint*> >&boardings, const std::vector<std::vector<boarding_type> >&boarding_types, const type::Data &data) {
    type::idx_t current_jpp = rpid;
    int cnt = count;

    DateTime last_time = labels[cnt][current_jpp];
    while(get_type(cnt, current_jpp, boarding_types, data)!= boarding_type::departure) {
        if(get_type(cnt, current_jpp, boarding_types, data) == boarding_type::vj) {
            DateTimeUtils::update(last_time, DateTimeUtils::hour(labels[cnt][current_jpp]), clockwise);
            current_jpp = get_boarding_jpp(cnt, current_jpp, boardings)->idx;
            --cnt;
        } else {
            current_jpp = get_boarding_jpp(cnt, current_jpp, boardings)->idx;
            last_time = labels[cnt][current_jpp];
        }
    }
    return std::make_pair(current_jpp, last_time);
}


float getWalkingTime(int count, type::idx_t jpp_idx, const std::vector<std::pair<type::idx_t, double> > &departs, const std::vector<std::pair<type::idx_t, double> > &destinations,
                     bool clockwise, const std::vector<label_vector_t> &/*labels*/, const std::vector<std::vector<const type::JourneyPatternPoint*> > &boardings,
                     const std::vector<std::vector<boarding_type> >&boarding_types, const type::Data &data) {

    const type::JourneyPatternPoint* current_jpp = data.pt_data.journey_pattern_points[jpp_idx];
    int cnt = count;
    float walking_time = 0;

    //Marche à la fin
    for(auto dest_dist : destinations) {
        if(dest_dist.first == current_jpp->stop_point->idx) {
            walking_time = dest_dist.second;
        }
    }
    //Marche pendant les correspondances
    auto boarding_type_value = get_type(cnt, current_jpp->idx, boarding_types, data);
    while(boarding_type_value != boarding_type::departure /*&& boarding_type_value != boarding_type::uninitialized*/) {
        if(boarding_type_value == boarding_type::vj) {
            current_jpp = get_boarding_jpp(cnt, current_jpp->idx, boardings);
            --cnt;
            boarding_type_value = get_type(cnt, current_jpp->idx, boarding_types, data);
        } else {
            const type::JourneyPatternPoint* boarding = get_boarding_jpp(cnt, current_jpp->idx, boardings);
            if(boarding_type_value == boarding_type::connection) {
                type::idx_t connection_idx = data.dataRaptor.get_stop_point_connection_idx(boarding->stop_point->idx, 
                                                                                           current_jpp->stop_point->idx,
                                                                                           clockwise, data.pt_data);
                if(connection_idx != type::invalid_idx)
                    walking_time += data.pt_data.stop_point_connections[connection_idx]->duration;
            }
            current_jpp = boarding;
            boarding_type_value = get_type(cnt, current_jpp->idx,  boarding_types, data);
        }
    }
    //Marche au départ
    for(auto dep_dist : departs) {
        if(dep_dist.first == current_jpp->stop_point->idx)
            walking_time += dep_dist.second;
    }

    return walking_time;
}

}}

