/* main test simulator */
#include <iostream>
#include "CraigUtils.h"
#include "Window.h"
#include "tokens.h"
#include "ObjInfo.h"
#include "QuadTree.h" 
#include "Params.h"
#include "LifeForm.h"
#include "Event.h"

using namespace std;

template <typename T>
void bound(T& x, const T& min, const T& max) {
	assert(min < max);
	if (x > max) { x = max; }
	if (x < min) { x = min; }
}

ObjInfo LifeForm::info_about_them(SmartPointer<LifeForm> neighbor) {
	ObjInfo info;

	info.species = neighbor->species_name();
	info.health = neighbor->health();
	info.distance = pos.distance(neighbor->position());
	info.bearing = pos.bearing(neighbor->position());
	info.their_speed = neighbor->speed;
	info.their_course = neighbor->course;
	return info;
}


void LifeForm::compute_next_move(void) {
	border_cross_event->cancel();
	if (!is_alive || speed < min_delta_time) {
		return;
	}

	double delta = (space.distance_to_edge(pos, course) / speed) + min_delta_time;

	SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
	border_cross_event = new Event(delta, [self](void) { self->border_cross(); });
}

/*
 * Calculate the current position for an object.  If less than
 * Time::tolerance time units have passed since the last call
 * to update_position, then do nothing (we can't have moved
 * very far so there's no point in updating our position)
 * 
 * computes the new position, charge for energy consumed. To
 * compute the new position, you'll need to know how long it
 * has been since the last time you'd called update_position on
 * this object (how much delta time has passed). You'll need a
 * data member called update_time. Each time update_position is
 * called, use Event::now() to see how much time has passed
 * since update_time. Calculate the new position by multiplying
 * your speed by the delta time. Don't forget to set
 * update_time equal to Event::now() when you're done.
 */
void LifeForm::update_position(void) {
	if (!is_alive) return;

	double now = Event::now();
	double delta = now - update_time;
	if (delta < min_delta_time) return;

	double xdelta = delta * speed * cos(course);
	double ydelta = delta * speed * sin(course);
	Point pos_new = Point(pos.xpos + xdelta, pos.ypos + ydelta);
	Point pos_old = pos;

	energy -= movement_cost(speed, delta);
	if (space.is_out_of_bounds(pos_new) || (energy < min_energy)) {
		die();
	} else {
		if (pos_old != pos_new) {
			pos = pos_new;
			space.update_position(pos_old, pos);
		}
	}

	update_time = now;
}

/*
 * The event handler function for the border cross event.
 * 
 * This is your movement event handler function. It calls
 * update_position and then schedules the next movement event.
 * You'll probably want to keep a pointer to the movement event
 * as part of your LifeForm base class (that way, you can
 * cancel this event, see below).
 */
void LifeForm::border_cross(void) {
	if (!is_alive) return;

	update_position();
	compute_next_move();
	check_encounter();
}

/*
 * The callback function for region resizes (invoked by the
 * quadtree).
 * 
 * This function will be a callback from the QuadTree. When
 * another object is created nearby, your object needs to
 * determine the next possible time that you could collide. The
 * QuadTree knows when objects are inserted inside it, so it
 * can invoke this callback function on your objects. What you
 * will want to do is have region_resize cancel any pending
 * border crossing events, update your position, and schedule
 * the next movement event (since the region has changed size,
 * you will encounter the boundary at a different time than
 * than before).
 */
void LifeForm::region_resize(void) {
	if (!is_alive) return;

	border_cross_event->cancel();

	update_position();
	compute_next_move();
}

/*
 * These functions should cancel any pending border_cross
 * event, update the current position of the object and then
 * schedule a new border_cross event based on the new course
 * and speed. Note that an object that is stationary will never
 * cross a border.
 */
void LifeForm::set_course(double p) {
	if (!is_alive) return;

	border_cross_event->cancel();
	update_position();

	course = p;
	compute_next_move();
}

void LifeForm::set_speed(double p) {
	if (!is_alive) return;

	border_cross_event->cancel();
	update_position();

	bound(p, 0.0, max_speed);
	speed = p;
	if (speed == 0.0) return;

	compute_next_move();
}

/*
 * subtract age_penalty from energy
 */
void LifeForm::age(void) {
	if (!is_alive) return;

	energy -= age_penalty;
	if (energy < min_energy) {
		die();
	} else {
		SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
		new Event(age_frequency, [self](void) { self->age(); });
	}
}

void LifeForm::gain_energy(double e) {
	if(!is_alive) return;

	energy += e * eat_efficiency;
}

void LifeForm::eat(SmartPointer<LifeForm> that) {
	if(!is_alive) return;

	energy = energy - eat_cost_function(energy, energy);
	if (energy < min_energy) {
		die();
	} else {
		SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
		new Event(digestion_time, [self,that](void) { self->gain_energy(that->energy); });

		that->die();
	}
}

/*
 * When a LifeForm reproduces (see above), the energy from the parent LifeForm
 * is divided in half. This amount of energy is given to both the parent and the
 * child. Then, the fractional reproduce_cost is subtracted from both the parent
 * and the child. For example, a LifeForm with 100 energy that reproduces will
 * produce a child that has 50 * (1.0 - reproduce_cost) energy. The parent will
 * have the same energy as the child.
 *
 * As a special rule, do not allow an object to reproduce faster than
 * min_reproduce_time. This rule is included to prevent the ugly strategy of
 * LifeForms reproducing themselves to death to avoid being eaten ( inside
 * encounter call reproduce 1000 times). If a LifeForm tries to call reproduce
 * twice within min_reproduce_time time units, the child should be deleted and
 * no energy penalty should be applied to the parent.
 */
void LifeForm::reproduce(SmartPointer<LifeForm> child) {
	if (!is_alive) return;

	double now = Event::now();
	if (now - reproduce_time < min_reproduce_time) {
		child->die(); /* very morbid */
		return;
	}

	double angle, radius;
	while (true) {
		angle = drand48() * 2 * M_PI;
		radius = drand48() * reproduce_dist;

		child->pos.xpos = pos.xpos + radius * cos(angle);
		child->pos.ypos = pos.ypos + radius * sin(angle);

		if (space.is_out_of_bounds(child->pos)) {
			continue;
		}

		if (space.is_occupied(child->pos)) {
			continue;
		}

		SmartPointer<LifeForm> close = space.closest(child->pos);
		if (close->pos.distance(child->pos) < encounter_distance) {
			continue;
		}

		break;
	}

	child->is_alive = true;
	space.insert(child, child->pos, [child](void) { child->region_resize(); });

	energy = (energy / 2) - reproduce_cost;
	child->energy = energy;

	reproduce_time = Event::now();
	child->reproduce_time = reproduce_time;

	if (energy < min_energy) {
		die();
		child->die();
	} else {
		new Event(age_frequency, [child](void) { child->age(); });
	}
}

/*
 * The perceive function should return a list of ObjInfos.
 * Each ObjInfo includes the name, bearing, speed, health
 * and distance to the object. One ObjInfo should be placed
 * into the list for every object within the prescribed
 * radius.
 */
ObjList LifeForm::perceive(double radius) {
	if (!is_alive) return ObjList{};

	/* bounds checking */
	if (radius < min_perceive_range) {
		radius = min_perceive_range;
	}
	if (radius > max_perceive_range) {
		radius = max_perceive_range;
	}

	ObjList near{};
	auto nearObjects = space.nearby(pos, radius);
	for (auto obj : nearObjects) {
		near.push_back(info_about_them(obj));
	}

	energy -= perceive_cost(radius);
	if (energy < min_energy) {
		die();
	}

	return near;
}

void LifeForm::check_encounter(void) {
	if (!is_alive) return;

	SmartPointer<LifeForm> close = space.closest(pos);
	if (close->is_alive && (pos.distance(close->pos) < encounter_distance)) {
		resolve_encounter(close);
	}
}

void LifeForm::resolve_encounter(SmartPointer<LifeForm> that) {
	if (!is_alive || !that->is_alive) return;

	that->energy = that->energy - encounter_penalty;
	if (that->energy < min_energy) {
		that->die();
	}

	energy = energy - encounter_penalty;
	if (energy < min_energy) {
		die();
	}

	if (is_alive && that->is_alive) {
		bool this_wins = false;
		bool that_wins = false;
		if ((this->encounter(this->info_about_them(that)) == LIFEFORM_EAT) &&
			(drand48() < eat_success_chance(energy, that->energy))) {
			this_wins = true;
		}

		if ((that->encounter(that->info_about_them(this)) == LIFEFORM_EAT) &&
			(drand48() < eat_success_chance(that->energy, energy))) {
			that_wins = true;
		}
		if (this_wins && that_wins) {
			switch (encounter_strategy) {
				case EVEN_MONEY:
					if (drand48() <= 0.5) {
						that_wins = false;
					} else {
						this_wins = false;
					}
					break;
				case FASTER_GUY_WINS:
					if (speed > that->speed) {
						that_wins = false;
					} else {
						this_wins = false;
					}
					break;
				case SLOWER_GUY_WINS:
					if (speed < that->speed) {
						that_wins = false;
					} else {
						this_wins = false;
					}
					break;
				case BIG_GUY_WINS:
					if (energy > that->energy) {
						that_wins = false;
					} else {
						this_wins = false;
					}
					break;
				case UNDERDOG_IS_HERE:
					if (energy < that->energy) {
						that_wins = false;
					} else {
						this_wins = false;
					}
					break;
			}
		}

		if (this_wins) {
			this->eat(that);
		} else if (that_wins) {
			that->eat(this);
		}
	}
}

