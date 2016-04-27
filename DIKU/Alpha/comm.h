/* ************************************************************************
*  file: comm.h , Communication module.                   Part of DIKUMUD *
*  Usage: Prototypes for the common functions in comm.c                   *
************************************************************************* */

extern void send_to_all (char *messg);
extern void send_to_char (char *messg, struct char_data *ch);
extern void send_to_except (char *messg, struct char_data *ch);
extern void send_to_room (char *messg, int room);
extern void send_to_room_except (char *messg, int room, struct char_data *ch);
extern void send_to_room_except_two
  (char *messg, int room, struct char_data *ch1, struct char_data *ch2);
extern void send_to_outdoor (char *messg);
extern void perform_to_all (char *messg, struct char_data *ch);
extern void perform_complex (struct char_data *ch1, struct char_data *ch2,
  struct obj_data *obj1, struct obj_data *obj2,
  char *mess, byte mess_type, bool hide);
extern void close_socket (struct descriptor_data *d);

extern void act (char *str, int hide_invisible, struct char_data *ch,
  struct obj_data *obj, void *vict_obj, int type);

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

extern int write_to_descriptor (int desc, char *txt);
extern void write_to_q (char *txt, struct txt_q *queue);
#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)

