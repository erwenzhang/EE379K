#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include "CraigUtils.h"
#include "Params.h"
#include "Event.h"
#include "ObjInfo.h"
#include "Random.h"
#include "Weems.h"
#include "Window.h"

using namespace std;
using String = std::string;

template <typename T>
void bound(T& x, const T& min, const T& max) {
	assert(min < max);
	if (x > max) { x = max; }
	if (x < min) { x = min; }
}

Initializer<Weems> __Weems_initializer;

String Weems::species_name(void) const {
	return "Weems";
}

bool Weems::victory(const ObjInfo& info) {
	switch (encounter_strategy) {
		case EVEN_MONEY:       return false;
		case FASTER_GUY_WINS:  return true; // (get_speed() > info.their_speed);
		case SLOWER_GUY_WINS:  return true; // (get_speed() < info.their_speed);
		case BIG_GUY_WINS:     return (health() > info.health);
		case UNDERDOG_IS_HERE: return (health() < info.health);
		default:               return true;
	}
}

Action Weems::encounter(const ObjInfo& info) {
	update_pos();

	if (info.species == species_name()) {
		/* don't be cannibalistic */
		set_course((info.their_course + get_course()) / 2);
		return LIFEFORM_IGNORE;
	} else {
		hunt_event->cancel();
		SmartPointer<Weems> self{this};
		hunt_event = new Event(0.0, [self] (void) { self->hunt(); });
		return LIFEFORM_EAT;
	}
}

void Weems::initialize(void) {
	LifeForm::add_creator(Weems::create, "Weems");
}

Weems::Weems() {
		hunt_event = Nil<Event>();
		SmartPointer<Weems> self{this};
		(void) new Event(0.0, [self] (void) { self->live();});
}


Weems::~Weems() {}

void Weems::spawn(void) {
	Weems* child = new Weems;
	reproduce(child);
}


Color Weems::my_color(void) const {
  	return MAGENTA;
}  

LifeForm* Weems::create(void)
{
	Weems* res = Nil<Weems>();
	res = new Weems;
	res->display();
	return res;
}

void Weems::live(void) {
	p = Point(0, 0);
	last_update = Event::now();
	density = 1.0;

	set_course(drand48() * 2.0 * M_PI);
	set_speed(2 + 5.0 * drand48());

	SmartPointer<Weems> self{this};
	hunt_event = new Event(5.0, [self] (void) { self->hunt();});
}

void Weems::update_pos(void) {
	double now = Event::now();
	double delta = now - last_update;
	if (delta < min_delta_time) return;

	double xdelta = delta * get_speed() * cos(get_course());
	double ydelta = delta * get_speed() * sin(get_course());
	p = Point(p.xpos + xdelta, p.ypos + ydelta);

	last_update = now;
}

void Weems::hunt(void) {
	const String fav_food = "Algae";

    hunt_event = Nil<Event>();
    if (health() == 0) { return; }

    update_pos();

    double region = sqrt(10 / density);
    bound(region, grid_max * 0.01, grid_max * 0.1);
    ObjList prey = perceive(region);
    density = (prey.size() + 0.01) / (region * region);

    double best_d = HUGE;
    double best_me = HUGE;
    double best_me_bearing = 0;
    double old_speed = get_speed();
    for (ObjInfo i : prey) {
    	if (i.species == species_name()) {
    		if (best_me > i.distance) {
    			best_me = i.distance;
    			best_me_bearing = i.bearing;
    		}
    	} else if (i.species == fav_food) {
			if (best_d > i.distance) {
				set_speed(old_speed * 1.01);
				set_course(i.bearing);
				best_d = i.distance;
				course_changed = 0;
			}
	    } else if (victory(i)) {
        	if (best_d * 0.1 > i.distance) {
        		switch (encounter_strategy) {
        			case FASTER_GUY_WINS:
        				set_speed(i.their_speed + M_E);
        				break;
        			case SLOWER_GUY_WINS:
        				set_speed(i.their_speed - M_E);
        				break;
        			default:
        				set_speed((i.their_speed + 0.01) * 1.01);
        				break;
        		}
        		set_course(i.bearing);
        		best_d = i.distance;
        		course_changed = 0;
        	}
        }
    }
    if (best_d == HUGE) {
        if(course_changed == 0) {
        	course_changed = 1;
        	if (best_me == HUGE) {
	            set_course(get_course() + drand48() * 0.25 * M_PI);
	            set_speed((1 - drand48() * 0.05) * old_speed);        		
        	} else {
        		set_course(best_me_bearing);
        	}
        }
    }
    if (p.distance(Point(0, 0)) > (grid_max / 3)) {
    	if(course_changed == 0) {
            course_changed = 1;
	    	set_course(p.bearing(Point(0, 0)));
	    	set_speed(old_speed * 0.90);
	    }
    }

    SmartPointer<Weems> self{this};
    hunt_event = new Event(10.0, [self] (void) { self->hunt();});

    if (health() >= 3.0) spawn();
}

