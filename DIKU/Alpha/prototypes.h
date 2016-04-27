/*! \file prototypes.h
  The intent of this header file is to provide explicit declarations
  for public functions.

 \author Jon A. Lambert
 \date 01/15/2005
 \version 0.1
 \remarks
  This source code copyright (C) 2005 by Jon A. Lambert
  All rights reserved.

  Released under the same terms of DikuMud
*/

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

/* act.movement.c */
extern int do_simple_move (struct char_data *ch, int cmd, int following);

/* act.social.c */
extern void do_action (struct char_data *ch, char *argument, int cmd);

/* act.wizard.c */
extern void do_start (struct char_data *ch);

/* fight.c */
extern void damage (struct char_data *ch, struct char_data *victim,
                    int dam, int attacktype);
extern void hit (struct char_data *ch, struct char_data *victim, int type);
extern void set_fighting (struct char_data *ch, struct char_data *vict);
extern void stop_fighting (struct char_data *ch);
extern void death_cry (struct char_data *ch);
extern void update_pos (struct char_data *victim);

/* modify.c */
extern void night_watchman (void);
extern void page_string (struct descriptor_data *d, char *str, int keep_internal);

/* signals.c */
void signal_setup (void);
void block_signals(void);
void restore_signals(void);

/* spec_assign.c */
extern void assign_mobiles (void);
extern void assign_objects (void);
extern void assign_rooms (void);

/* spell_parser.c */
extern bool circle_follow (struct char_data *ch, struct char_data *victim);
extern bool saves_spell (struct char_data *ch, sh_int save_type);
extern void add_follower (struct char_data *ch, struct char_data *leader);

/* utility.c */
extern int str_cmp (char *arg1, char *arg2);
extern int strn_cmp (char *arg1, char *arg2, int n);
extern int number (int from, int to);
extern void log (char *str);
extern int dice (int number, int size);
extern void sprinttype (int type, char *names[], char *result);
extern void sprintbit (long vektor, char *names[], char *result);
#if !defined MIN
extern int MIN (int a, int b);
#endif
#if !defined MAX
extern int MAX (int a, int b);
#endif

/* weather.c */
extern void weather_and_time (int mode);


#endif
