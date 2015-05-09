#ifndef _amw3647_h
#define _amw3647_h

#include "LifeForm.h"
#include "Init.h"

class amw3647 : public LifeForm {
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
  amw3647(void);
  ~amw3647(void);
  Color my_color(void) const;
  static LifeForm* create(void);
  std::string species_name(void) const;
  Action encounter(const ObjInfo&);
  friend class Initializer<amw3647>;
};

#endif /* !(_amw3647_h) */