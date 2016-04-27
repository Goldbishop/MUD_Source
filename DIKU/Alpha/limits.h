/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */

/* Public Procedures */
extern int mana_limit (struct char_data *ch);
extern int hit_limit (struct char_data *ch);
extern int move_limit (struct char_data *ch);
extern void set_title (struct char_data *ch);
extern void gain_condition (struct char_data *ch, int condition, int value);
extern void gain_exp_regardless (struct char_data *ch, int gain);
extern void gain_exp (struct char_data *ch, int gain);

struct title_type {
  char *title_m;
  char *title_f;
  int exp;
};
