#ifndef _Weems_h
#define _Weems_h

#include "LifeForm.h"
#include "Init.h"

class Weems : public LifeForm {
protected:
  int course_changed;
  double density;
  double last_update;
  Point p;
  static void initialize(void);
  void spawn(void);
  void hunt(void);
  void live(void);
  bool victory(const ObjInfo& info);
  void update_pos(void);
  Event* hunt_event;
public:
  Weems(void);
  ~Weems(void);
  Color my_color(void) const;
  static LifeForm* create(void);
  std::string species_name(void) const;
  Action encounter(const ObjInfo&);
  friend class Initializer<Weems>;
};

#endif /* !(_Weems_h) */