/* ************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[4][25];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];

/* external functs */

void set_title(struct char_data *ch);
int str_cmp(char *arg1, char *arg2);
struct time_info_data age(struct char_data *ch);
void sprinttype(int type, char *names[], char *result);
void sprintbit(long vektor, char *names[], char *result);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);



do_emote(struct char_data *ch, char *argument, int cmd)
{
	int i;
	static char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char("Yes.. But what?\r\n", ch);
	else
	{
		sprintf(buf,"$n %s", argument + i);
		act(buf,FALSE,ch,0,0,TO_ROOM);
		send_to_char("Ok.\r\n", ch);
	}
}



void do_echo(struct char_data *ch, char *argument, int cmd)
{
	int i;
	static char buf[MAX_STRING_LENGTH];
	
	if (IS_NPC(ch))
		return;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char("That must be a mistake...\r\n", ch);
	else
	{
		sprintf(buf,"%s\r\n", argument + i);
		send_to_room_except(buf, ch->in_room, ch);
		send_to_char("Ok.\r\n", ch);
	}
}



void do_trans(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
	struct char_data *victim;
	char buf[100];
	short target;

	if (IS_NPC(ch))
		return;

	one_argument(argument,buf);
	if (!*buf)
		send_to_char("Who do you wich to transfer?\r\n",ch);
	else if (str_cmp("all", buf)) {
		if (!(victim = get_char_vis(ch,buf)))
			send_to_char("No-one by that name around.\r\n",ch);
		else {
			act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
			target = ch->in_room;
			char_from_room(victim);
			char_to_room(victim,target);
			act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
			act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
			do_look(victim,"",15);
			send_to_char("Ok.\r\n",ch);
		}
	} else { /* Trans All */
    for (i = descriptor_list; i; i = i->next)
			if (i->character != ch && !i->connected) {
				victim = i->character;
				act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
				target = ch->in_room;
				char_from_room(victim);
				char_to_room(victim,target);
				act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
				act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
				do_look(victim,"",15);
			}

		send_to_char("Ok.\r\n",ch);
	}
}



void do_at(struct char_data *ch, char *argument, int cmd)
{
	char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
	int loc_nr, location, original_loc;
	struct char_data *target_mob;
	struct obj_data *target_obj;
	extern int top_of_world;
	
	if (IS_NPC(ch))
		return;

	half_chop(argument, loc_str, command);
	if (!*loc_str)
	{
		send_to_char("You must supply a room number or a name.\r\n", ch);
		return;
	}

	
	if (isdigit(*loc_str))
	{
		loc_nr = atoi(loc_str);
		for (location = 0; location <= top_of_world; location++)
			if (world[location].number == loc_nr)
				break;
			else if (location == top_of_world)
			{
				send_to_char("No room exists with that number.\r\n", ch);
				return;
			}
	}
	else if (target_mob = get_char_vis(ch, loc_str))
		location = target_mob->in_room;
	else if (target_obj = get_obj_vis(ch, loc_str))
		if (target_obj->in_room != NOWHERE)
			location = target_obj->in_room;
		else
		{
			send_to_char("The object is not available.\r\n", ch);
			return;
		}
	else
	{
		send_to_char("No such creature or object around.\r\n", ch);
		return;
	}

	/* a location has been found. */

	original_loc = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, location);
	command_interpreter(ch, command);

	/* check if the guy's still there */
	for (target_mob = world[location].people; target_mob; target_mob =
		target_mob->next_in_room)
		if (ch == target_mob)
		{
			char_from_room(ch);
			char_to_room(ch, original_loc);
		}
}



void do_goto(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int loc_nr, location, i;
	struct char_data *target_mob, *pers;
	struct obj_data *target_obj;
	extern int top_of_world;

	if (IS_NPC(ch))
		return;
	
	one_argument(argument, buf);
	if (!*buf)
	{
		send_to_char("You must supply a room number or a name.\r\n", ch);
		return;
	}

	
	if (isdigit(*buf))
	{
		loc_nr = atoi(buf);
		for (location = 0; location <= top_of_world; location++)
			if (world[location].number == loc_nr)
				break;
			else if (location == top_of_world)
			{
				send_to_char("No room exists with that number.\r\n", ch);
				return;
			}
	}
	else if (target_mob = get_char_vis(ch, buf))
		location = target_mob->in_room;
	else if (target_obj = get_obj_vis(ch, buf))
		if (target_obj->in_room != NOWHERE)
			location = target_obj->in_room;
		else
		{
			send_to_char("The object is not available.\r\n", ch);
			return;
		}
	else
	{
		send_to_char("No such creature or object around.\r\n", ch);
		return;
	}

	/* a location has been found. */

	if (IS_SET(world[location].room_flags, PRIVATE))
	{
		for (i = 0, pers = world[location].people; pers; pers =
			pers->next_in_room, i++);
		if (i > 1)
		{
			send_to_char(
				"There's a private conversation going on in that room.\r\n", ch);
			return;
		}
	}

	act("$n disappears in a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, location);
	act("$n appears with an ear-splitting bang.", FALSE, ch, 0,0,TO_ROOM);
	do_look(ch, "",15);
}



void do_stat(struct char_data *ch, char *argument, int cmd)
{
	extern char *spells[];
	struct affected_type *aff;
	char arg1[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	struct char_data *victim=0;
	struct room_data *rm=0;
	struct char_data *k=0;
	struct obj_data  *j=0;
	struct obj_data  *j2=0;
	struct extra_descr_data *desc;
	struct follow_type *fol;
	int i, virtual;
	int i2;
	bool found;

	/* for objects */
	extern char *item_types[];
	extern char *wear_bits[];
	extern char *extra_bits[];
	extern char *drinks[];

	/* for rooms */
	extern char *dirs[];
	extern char *room_bits[];
	extern char *exit_bits[];
	extern char *sector_types[];

	/* for chars */
	extern char *equipment_types[];
	extern char *affected_bits[];
	extern char *apply_types[];
	extern char *pc_class_types[];
	extern char *npc_class_types[];
	extern char *action_bits[];
	extern char *player_bits[];
	extern char *position_types[];
	extern char *connected_types[];

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg1);

	/* no argument */
	if (!*arg1) {
		send_to_char("Stats on who or what?\r\n",ch);
		return;
	} else {
		/* stats on room */
		if (!str_cmp("room", arg1)) {
			rm = &world[ch->in_room];
			sprintf(buf, "Room name: %s, Of zone : %d. V-Number : %d, R-number : %d\r\n",
			        rm->name, rm->zone, rm->number, ch->in_room);
			send_to_char(buf, ch);

			sprinttype(rm->sector_type,sector_types,buf2);
			sprintf(buf, "Sector type : %s", buf2);
			send_to_char(buf, ch);
			
			strcpy(buf,"Special procedure : ");
			strcat(buf,(rm->funct) ? "Exists\r\n" : "No\r\n");
			send_to_char(buf, ch);

			send_to_char("Room flags: ", ch);
			sprintbit((long) rm->room_flags,room_bits,buf);
			strcat(buf,"\r\n");
			send_to_char(buf,ch);

			send_to_char("Description:\r\n", ch);
			send_to_char(rm->description, ch);
			
			strcpy(buf, "Extra description keywords(s): ");
			if(rm->ex_description) {
				strcat(buf, "\r\n");
				for (desc = rm->ex_description; desc; desc = desc->next) {
					strcat(buf, desc->keyword);
					strcat(buf, "\r\n");
				}
				strcat(buf, "\r\n");
				send_to_char(buf, ch);
			} else {
				strcat(buf, "None\r\n");
				send_to_char(buf, ch);
			}

			strcpy(buf, "------- Chars present -------\r\n");
			for (k = rm->people; k; k = k->next_in_room)
			{
				strcat(buf, GET_NAME(k));
				strcat(buf,(!IS_NPC(k) ? "(PC)\r\n" : (!IS_MOB(k) ? "(NPC)\r\n" : "(MOB)\r\n")));
			}
			strcat(buf, "\r\n");
			send_to_char(buf, ch);

			strcpy(buf, "--------- Contents ---------\r\n");
			for (j = rm->contents; j; j = j->next_content)
			{
				strcat(buf, j->name);
				strcat(buf, "\r\n");
			}
			strcat(buf, "\r\n");
			send_to_char(buf, ch);
		
			send_to_char("------- Exits defined -------\r\n", ch);
			for (i = 0; i <= 5; i++) {
				if (rm->dir_option[i]) {
					sprintf(buf,"Direction %s . Keyword : %s\r\n",
					        dirs[i], rm->dir_option[i]->keyword);
					send_to_char(buf, ch);
					strcpy(buf, "Description:\r\n  ");
					if(rm->dir_option[i]->general_description)
				  	strcat(buf, rm->dir_option[i]->general_description);
					else
						strcat(buf,"UNDEFINED\r\n");
					send_to_char(buf, ch);
					sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
					sprintf(buf, "Exit flag: %s \r\nKey no: %d\r\nTo room (R-Number): %d\r\n",
					        buf2, rm->dir_option[i]->key,
					        rm->dir_option[i]->to_room);
					send_to_char(buf, ch);
				}
			}
			return;
		}

		/* stat on object */
		if (j = get_obj_vis(ch, arg1)) {
			virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
			sprintf(buf, "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
			   j->name, j->item_number, virtual);
			sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
			strcat(buf,buf2); strcat(buf,"\r\n");
			send_to_char(buf, ch);
			sprintf(buf, "Short description: %s\r\nLong description:\r\n%s\r\n",
			        ((j->short_description) ? j->short_description : "None"),
			        ((j->description) ? j->description : "None") );
			send_to_char(buf, ch);
			if(j->ex_description){
				strcpy(buf, "Extra description keyword(s):\r\n----------\r\n");
				for (desc = j->ex_description; desc; desc = desc->next) {
					strcat(buf, desc->keyword);
					strcat(buf, "\r\n");
				}
				strcat(buf, "----------\r\n");
				send_to_char(buf, ch);
			} else {
				strcpy(buf,"Extra description keyword(s): None\r\n");
				send_to_char(buf, ch);
			}

			send_to_char("Can be worn on :", ch);
			sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
			strcat(buf,"\r\n");
			send_to_char(buf, ch);

			send_to_char("Set char bits  :", ch);
			sprintbit(j->obj_flags.bitvector,affected_bits,buf);
			strcat(buf,"\r\n");
			send_to_char(buf, ch);

			send_to_char("Extra flags: ", ch);
			sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
			strcat(buf,"\r\n");
			send_to_char(buf,ch);

			sprintf(buf,"Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
			       j->obj_flags.weight,j->obj_flags.cost,
			       j->obj_flags.cost_per_day,  j->obj_flags.timer);
			send_to_char(buf, ch);

			strcpy(buf,"In room: ");
			if (j->in_room == NOWHERE)
				strcat(buf,"Nowhere");
			else {
				sprintf(buf2,"%d",world[j->in_room].number);
				strcat(buf,buf2);
			}
			strcat(buf," ,In object: ");
			strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));
			strcat(buf," ,Carried by:");
			strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
			strcat(buf,"\r\n");
			send_to_char(buf, ch);

			switch (j->obj_flags.type_flag) {
				case ITEM_LIGHT : 
					sprintf(buf, "Colour : [%d]\r\nType : [%d]\r\nHours : [%d]",
						j->obj_flags.value[0],
						j->obj_flags.value[1],
						j->obj_flags.value[2]);
					break;
				case ITEM_SCROLL : 
					sprintf(buf, "Spells : %d, %d, %d, %d",
						j->obj_flags.value[0],
				 		j->obj_flags.value[1],
				 		j->obj_flags.value[2],
				 		j->obj_flags.value[3] );
					break;
				case ITEM_WAND : 
					sprintf(buf, "Spell : %d\r\nMana : %d",
						j->obj_flags.value[0],
						j->obj_flags.value[1]);
					break;
				case ITEM_STAFF : 
					sprintf(buf, "Spell : %d\r\nMana : %d",
						j->obj_flags.value[0],
						j->obj_flags.value[1]);
					break;
				case ITEM_WEAPON :
					sprintf(buf, "Tohit : %d\r\nTodam : %dD%d\r\nType : %d",
						j->obj_flags.value[0],
	    			j->obj_flags.value[1],
	    			j->obj_flags.value[2],
	    			j->obj_flags.value[3]);
					break;
				case ITEM_FIREWEAPON : 
					sprintf(buf, "Tohit : %d\r\nTodam : %dD%d\r\nType : %d",
						j->obj_flags.value[0],
	    			j->obj_flags.value[1],
	    			j->obj_flags.value[2],
	    			j->obj_flags.value[3]);
					break;
				case ITEM_MISSILE : 
					sprintf(buf, "Tohit : %d\r\nTodam : %d\r\nType : %d",
						j->obj_flags.value[0],
						j->obj_flags.value[1],
						j->obj_flags.value[3]);
					break;
				case ITEM_ARMOR :
					sprintf(buf, "AC-apply : [%d]",
						j->obj_flags.value[0]);
					break;
				case ITEM_POTION : 
					sprintf(buf, "Spells : %d, %d, %d, %d",
						j->obj_flags.value[0],
						j->obj_flags.value[1],
						j->obj_flags.value[2],
						j->obj_flags.value[3]); 
					break;
				case ITEM_TRAP :
					sprintf(buf, "Spell : %d\r\n- Hitpoints : %d",
						j->obj_flags.value[0],
						j->obj_flags.value[1]);
					break;
				case ITEM_CONTAINER :
					sprintf(buf, "Max-contains : %d\r\nLocktype : %d\r\nCorpse : %s",
						j->obj_flags.value[0],
						j->obj_flags.value[1],
						j->obj_flags.value[3]?"Yes":"No");
					break;
				case ITEM_DRINKCON :
					sprinttype(j->obj_flags.value[2],drinks,buf2);
					sprintf(buf, "Max-contains : %d\r\nContains : %d\r\nPoisoned : %d\r\nLiquid : %s",
						j->obj_flags.value[0],
						j->obj_flags.value[1],
						j->obj_flags.value[3],
						buf2);
					break;
				case ITEM_NOTE :
					sprintf(buf, "Tounge : %d",
						j->obj_flags.value[0]);
					break;
				case ITEM_KEY :
					sprintf(buf, "Keytype : %d",
						j->obj_flags.value[0]);
					break;
				case ITEM_FOOD :
					sprintf(buf, "Makes full : %d\r\nPoisoned : %d",
						j->obj_flags.value[0],
						j->obj_flags.value[3]);
					break;
				default :
					sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]",
						j->obj_flags.value[0],
						j->obj_flags.value[1],
						j->obj_flags.value[2],
						j->obj_flags.value[3]);
					break;
			}
			send_to_char(buf, ch);

			strcpy(buf,"\r\nEquipment Status: ");
			if (!j->carried_by)
				strcat(buf,"NONE");
			else {
				found = FALSE;
				for (i=0;i < MAX_WEAR;i++) {
					if (j->carried_by->equipment[i] == j) {
						sprinttype(i,equipment_types,buf2);
						strcat(buf,buf2);
						found = TRUE;
					}
				}
				if (!found)
					strcat(buf,"Inventory");
			}
			send_to_char(buf, ch);

			strcpy(buf, "\r\nSpecial procedure : ");
			if (j->item_number >= 0)
				strcat(buf, (obj_index[j->item_number].func ? "exists\r\n" : "No\r\n"));
			else
				strcat(buf, "No\r\n");
			send_to_char(buf, ch);

			strcpy(buf, "Contains :\r\n");
			found = FALSE;
			for(j2=j->contains;j2;j2 = j2->next_content) {
				strcat(buf,fname(j2->name));
				strcat(buf,"\r\n");
				found == TRUE;
			}
			if (!found)
				strcpy(buf,"Contains : Nothing\r\n");
			send_to_char(buf, ch);

			send_to_char("Can affect char :\r\n", ch);
			for (i=0;i<MAX_OBJ_AFFECT;i++) {
				sprinttype(j->affected[i].location,apply_types,buf2);
				sprintf(buf,"    Affects : %s By %d\r\n", buf2,j->affected[i].modifier);
				send_to_char(buf, ch);
			}			
			return;
		}

		/* mobile in world */
		if (k = get_char_vis(ch, arg1)){

			switch(k->player.sex) {
				case SEX_NEUTRAL : 
					strcpy(buf,"NEUTRAL-SEX"); 
					break;
				case SEX_MALE :
					strcpy(buf,"MALE");
					break;
				case SEX_FEMALE :
					strcpy(buf,"FEMALE");
					break;
				default : 
					strcpy(buf,"ILLEGAL-SEX!!");
					break;
			}

			sprintf(buf2, " %s - Name : %s [R-Number%d], In room [%d]\r\n",
			   (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
					GET_NAME(k), k->nr, world[k->in_room].number);
			strcat(buf, buf2);
			send_to_char(buf, ch);
			if (IS_MOB(k)) {
				sprintf(buf, "V-Number [%d]\r\n", mob_index[k->nr].virtual);
				send_to_char(buf, ch);
			}

			strcpy(buf,"Short description: ");
			strcat(buf, (k->player.short_descr ? k->player.short_descr : "None"));
			strcat(buf,"\r\n");
			send_to_char(buf,ch);

			strcpy(buf,"Title: ");
			strcat(buf, (k->player.title ? k->player.title : "None"));
			strcat(buf,"\r\n");
			send_to_char(buf,ch);

			send_to_char("Long description: ", ch);
			if (k->player.long_descr)
				send_to_char(k->player.long_descr, ch);
			else
				send_to_char("None", ch);
			send_to_char("\r\n", ch);

			if (IS_NPC(k)) {
				strcpy(buf,"Monster Class: ");
				sprinttype(k->player.class,npc_class_types,buf2);
			} else {
				strcpy(buf,"Class: ");
				sprinttype(k->player.class,pc_class_types,buf2);
			}
			strcat(buf, buf2);

			sprintf(buf2,"   Level [%d] Alignment[%d]\r\n",k->player.level,
			              k->specials.alignment);
			strcat(buf, buf2);
			send_to_char(buf, ch);

			sprintf(buf,"Birth : [%ld]secs, Logon[%ld]secs, Played[%ld]secs\r\n", 
			        k->player.time.birth,
			        k->player.time.logon,
			        k->player.time.played);

			send_to_char(buf, ch);

			sprintf(buf,"Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\r\n",
			        age(k).year, age(k).month, age(k).day, age(k).hours);
			send_to_char(buf,ch);

			sprintf(buf,"Height [%d]cm  Weight [%d]pounds \r\n", GET_HEIGHT(k), GET_WEIGHT(k));
			send_to_char(buf,ch);

			sprintf(buf,"Hometown[%d], Speaks[%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])\r\n",
				k->player.hometown,
				k->player.talks[0],
				k->player.talks[1],
				k->player.talks[2],
				k->specials.spells_to_learn,
				int_app[GET_INT(k)].learn,
				wis_app[GET_WIS(k)].bonus);
			send_to_char(buf, ch);

			sprintf(buf,"Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\r\n",
				GET_STR(k), GET_ADD(k),
				GET_INT(k),
				GET_WIS(k),
				GET_DEX(k),
				GET_CON(k) );
			send_to_char(buf,ch);

			sprintf(buf,"Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\r\n",
				GET_MANA(k),mana_limit(k),mana_gain(k),
				GET_HIT(k),hit_limit(k),hit_gain(k),
				GET_MOVE(k),move_limit(k),move_gain(k) );
			send_to_char(buf,ch);

			sprintf(buf,"AC:[%d/10], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\r\n",
				GET_AC(k),
				GET_GOLD(k),
				GET_EXP(k),
				k->points.hitroll,
				k->points.damroll );
			send_to_char(buf,ch);

			sprinttype(GET_POS(k),position_types,buf2);
			sprintf(buf,"Position: %s, Fighting: %s",buf2,
			        ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody") );
			if (k->desc) {
				sprinttype(k->desc->connected,connected_types,buf2);
				strcat(buf,", Connected: ");
				strcat(buf,buf2);
			}
			strcat(buf,"\r\n");
			send_to_char(buf, ch);

			strcpy(buf,"Default position: ");
			sprinttype((k->specials.default_pos),position_types,buf2);
			strcat(buf, buf2);
			if (IS_NPC(k))
			{
				strcat(buf,",NPC flags: ");
				sprintbit(k->specials.act,action_bits,buf2);
			}
			else
			{
				strcat(buf,",PC flags: ");
				sprintbit(k->specials.act,player_bits,buf2);
			}

			strcat(buf, buf2);

			sprintf(buf2,",Timer [%d] \r\n", k->specials.timer);
			strcat(buf, buf2);
			send_to_char(buf, ch);

			if (IS_MOB(k)) {
				strcpy(buf, "\r\nMobile Special procedure : ");
				strcat(buf, (mob_index[k->nr].func ? "Exists\r\n" : "None\r\n"));
				send_to_char(buf, ch);
			}

			if (IS_NPC(k)) {
				sprintf(buf, "NPC Bare Hand Damage %dd%d.\r\n",
					k->specials.damnodice, k->specials.damsizedice);
				send_to_char(buf, ch);
			}

			sprintf(buf,"Carried weight: %d   Carried items: %d\r\n",
				IS_CARRYING_W(k),
				IS_CARRYING_N(k) );
			send_to_char(buf,ch);

			for(i=0,j=k->carrying;j;j=j->next_content,i++);
			sprintf(buf,"Items in inventory: %d, ",i);

			for(i=0,i2=0;i<MAX_WEAR;i++)
				if (k->equipment[i]) i2++;
			sprintf(buf2,"Items in equipment: %d\r\n", i2);
			strcat(buf,buf2);
			send_to_char(buf, ch);

			sprintf(buf,"Apply saving throws: [%d] [%d] [%d] [%d] [%d]\r\n",
			        k->specials.apply_saving_throw[0],
			        k->specials.apply_saving_throw[1],
			        k->specials.apply_saving_throw[2],
			        k->specials.apply_saving_throw[3],
			        k->specials.apply_saving_throw[4]);
			send_to_char(buf, ch);

			sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\r\n",
			        k->specials.conditions[THIRST],
			        k->specials.conditions[FULL],
			        k->specials.conditions[DRUNK]);
			send_to_char(buf, ch);

			sprintf(buf, "Master is '%s'\r\n",
				((k->master) ? GET_NAME(k->master) : "NOBODY"));
			send_to_char(buf, ch);
			send_to_char("Followers are:\r\n", ch);
			for(fol=k->followers; fol; fol = fol->next)
				act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);

			/* Showing the bitvector */
			sprintbit(k->specials.affected_by,affected_bits,buf);
			send_to_char("Affected by: ", ch);
			strcat(buf,"\r\n");
			send_to_char(buf, ch);

			/* Routine to show what spells a char is affected by */
			if (k->affected) {
				send_to_char("\r\nAffecting Spells:\r\n--------------\r\n", ch);
				for(aff = k->affected; aff; aff = aff->next) {
					sprintf(buf, "Spell : '%s'\r\n",spells[aff->type-1]);
					send_to_char(buf, ch);
					sprintf(buf,"     Modifies %s by %d points\r\n",
						apply_types[aff->location], aff->modifier);
					send_to_char(buf, ch);
					sprintf(buf,"     Expires in %3d hours, Bits set ",
						aff->duration);
					send_to_char(buf, ch);
					sprintbit(aff->bitvector,affected_bits,buf);
					strcat(buf,"\r\n");
					send_to_char(buf, ch);
				}
			}
			return;
		} else {
			send_to_char("No mobile or object by that name in the world\r\n", ch);
		}
	}
}



void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("If you want to shut something down - say so!\r\n", ch);
}



void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
	extern int shutdown, reboot;
	char buf[100], arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);

	if (!*arg)
	{
		sprintf(buf, "Shutdown by %s.", GET_NAME(ch) );
		send_to_all(buf);
		log(buf);
		shutdown = 1;
	}
	else if (!str_cmp(arg, "reboot"))
	{
		sprintf(buf, "Reboot by %s.", GET_NAME(ch));
		send_to_all(buf);
		log(buf);
		shutdown = reboot = 1;
	}
	else
		send_to_char("Go shut down someone your own size.\r\n", ch);
}


void do_snoop(struct char_data *ch, char *argument, int cmd)
{
	static char arg[MAX_STRING_LENGTH];
	struct char_data *victim;

	if (!ch->desc)
		return;

	one_argument(argument, arg);

	if(!*arg)
	{
		send_to_char("Snoop who ?\r\n",ch);
		return;
	}

	if(!(victim=get_char_vis(ch, arg)))
	{
		send_to_char("No such person around.\r\n",ch);
		return;
	}

	if(!victim->desc)
	{
		send_to_char("There's no link.. nothing to snoop.\r\n",ch);
		return;
	}

	if(victim == ch)
	{
		send_to_char("Ok, you just snoop yourself.\r\n",ch);
		if(ch->desc->snoop.snooping)
			{
				ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
				ch->desc->snoop.snooping = 0;
			}
		return;
	}

	if(victim->desc->snoop.snoop_by)	
	{
		send_to_char("Busy already. \r\n",ch);
		return;
	}

	if(GET_LEVEL(victim)>=GET_LEVEL(ch))
	{
		send_to_char("You failed.\r\n",ch);
		return;
	}

	send_to_char("Ok. \r\n",ch);

	if(ch->desc->snoop.snooping)
		ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

	ch->desc->snoop.snooping = victim;
	victim->desc->snoop.snoop_by = ch;
	return;
}



void do_switch(struct char_data *ch, char *argument, int cmd)
{
	static char arg[MAX_STRING_LENGTH];
	char buf[70];
	struct char_data *victim;

	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);
	
	if (!*arg)
	{
		send_to_char("Switch with who?\r\n", ch);
	}
	else
	{
		if (!(victim = get_char(arg)))
			 send_to_char("They aren't here.\r\n", ch);
		else
		{
			if (ch == victim)
			{
				send_to_char("He he he... We are jolly funny today, eh?\r\n", ch);
				return;
			}

			if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping)
			{
				send_to_char("Mixing snoop & switch is bad for your health.\r\n", ch);
				return;
			}

			if(victim->desc || (!IS_NPC(victim))) 
      {
				send_to_char(
           "You can't do that, the body is already in use!\r\n",ch);
			}
			else
			{
				send_to_char("Ok.\r\n", ch);
				
				ch->desc->character = victim;
				ch->desc->original = ch;

				victim->desc = ch->desc;
				ch->desc = 0;
			}
		}
	}
}



void do_return(struct char_data *ch, char *argument, int cmd)
{
	static char arg[MAX_STRING_LENGTH];
	char buf[70];

	if(!ch->desc)
		return;

	if(!ch->desc->original)
   { 
		send_to_char("Arglebargle, glop-glyf!?!\r\n", ch);
		return;
	}
	else
	{
		send_to_char("You return to your originaly body.\r\n",ch);

		ch->desc->character = ch->desc->original;
		ch->desc->original = 0;

		ch->desc->character->desc = ch->desc; 
		ch->desc = 0;
	}
}


void do_force(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
	struct char_data *vict;
	char name[100], to_force[100],buf[100]; 

	if (IS_NPC(ch))
		return;

	half_chop(argument, name, to_force);

	if (!*name || !*to_force)
		 send_to_char("Who do you wish to force to do what?\r\n", ch);
	else if (str_cmp("all", name)) {
		if (!(vict = get_char_vis(ch, name)))
			send_to_char("No-one by that name here..\r\n", ch);
		else
		{
			if (GET_LEVEL(ch) < GET_LEVEL(vict))
				send_to_char("Oh no you don't!!\r\n", ch);
			else {
				sprintf(buf, "$n has forced you to '%s'.", to_force);
				act(buf, FALSE, ch, 0, vict, TO_VICT);
				send_to_char("Ok.\r\n", ch);
				command_interpreter(vict, to_force);
			}
		}
	} else { /* force all */
    for (i = descriptor_list; i; i = i->next)
			if (i->character != ch && !i->connected) {
				vict = i->character;
				if (GET_LEVEL(ch) < GET_LEVEL(vict))
					send_to_char("Oh no you don't!!\r\n", ch);
				else {
					sprintf(buf, "$n has forced you to '%s'.", to_force);
					act(buf, FALSE, ch, 0, vict, TO_VICT);
					command_interpreter(vict, to_force);
				}
			}
			send_to_char("Ok.\r\n", ch);
	}
}



void do_load(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *mob;
	struct obj_data *obj;
	char type[100], num[100], buf[100];
	int number, r_num;

	extern int top_of_mobt;
	extern int top_of_objt;

	if (IS_NPC(ch))
		return;

	argument_interpreter(argument, type, num);

	if (!*type || !*num || !isdigit(*num))
	{
		send_to_char("Syntax:\r\nload <'char' | 'obj'> <number>.\r\n", ch);
		return;
	}

	if ((number = atoi(num)) < 0)
	{
		send_to_char("A NEGATIVE number??\r\n", ch);
		return;
	}
	if (is_abbrev(type, "char"))
	{
		if ((r_num = real_mobile(number)) < 0)
		{
			send_to_char("There is no monster with that number.\r\n", ch);
			return;
 		}
		mob = read_mobile(r_num, REAL);
		char_to_room(mob, ch->in_room);

		act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
			0, 0, TO_ROOM);
		act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
		send_to_char("Done.\r\n", ch);
	}
	else if (is_abbrev(type, "obj"))
	{
		if ((r_num = real_object(number)) < 0)
		{
			send_to_char("There is no object with that number.\r\n", ch);
			return;
		}
		obj = read_object(r_num, REAL);
		obj_to_room(obj, ch->in_room);
		act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
		act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
		send_to_char("Ok.\r\n", ch);
	}
	else
		send_to_char("That'll have to be either 'char' or 'obj'.\r\n", ch);
}





/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict, *next_v;
	struct obj_data *obj, *next_o;

	char name[100], buf[100];

	if (IS_NPC(ch))
		return;

	one_argument(argument, name);

	if (*name)  /* argument supplied. destroy single object or char */
	{
		if (vict = get_char_room_vis(ch, name))
		{
			if (!IS_NPC(vict) && (GET_LEVEL(ch)<24)) {
				send_to_char("Fuuuuuuuuu!\r\n", ch);
				return;
			}

			act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

			if (IS_NPC(vict)) {
				extract_char(vict);
			} else {
				if (vict->desc) 
				{
					close_socket(vict->desc);
					vict->desc = 0;
					extract_char(vict);
				}
				else 
				{
					extract_char(vict);
				}
			}
		}
		else if (obj = get_obj_in_list_vis(ch, name,
			world[ch->in_room].contents))
		{
			act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
			extract_obj(obj);
		}
		else
		{
			send_to_char("I don't know anyone or anything by that name.\r\n", ch);
			return;
		}

		send_to_char("Ok.\r\n", ch);
	}
	else   /* no argument. clean out the room */
	{
		if (IS_NPC(ch))
		{
			send_to_char("Don't... You would only kill yourself..\r\n", ch);
			return;
		}

		act("$n gestures... You are surrounded by scorching flames!", 
			FALSE, ch, 0, 0, TO_ROOM);
		send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

		for (vict = world[ch->in_room].people; vict; vict = next_v)
		{
			next_v = vict->next_in_room;
			if (IS_NPC(vict))
				extract_char(vict);
		}

		for (obj = world[ch->in_room].contents; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj(obj);
		}
	}
}



/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
	int i, j, k, temp;
	ubyte table[5];
	ubyte rools[4];

	for(i=0; i<5; table[i++]=0)  ;

	for(i=0; i<5; i++) {

		for(j=0; j<4; j++)
			rools[j] = number(1,6);
		
		temp = rools[0]+rools[1]+rools[2]+rools[3] -
		           MIN(rools[0], MIN(rools[1], MIN(rools[2],rools[3])));

		for(k=0; k<5; k++)
			if (table[k] < temp)
				SWITCH(temp, table[k]);
	}

	ch->abilities.str_add = 0;

	switch (GET_CLASS(ch)) {
		case CLASS_MAGIC_USER: {
			ch->abilities.intel = table[0];
			ch->abilities.wis = table[1];
			ch->abilities.dex = table[2];
			ch->abilities.str = table[3];
			ch->abilities.con = table[4];
		}	break;
		case CLASS_CLERIC: {
			ch->abilities.wis = table[0];
			ch->abilities.intel = table[1];
			ch->abilities.str = table[2];
			ch->abilities.dex = table[3];
			ch->abilities.con = table[4];
		} break;
		case CLASS_THIEF: {
			ch->abilities.dex = table[0];
			ch->abilities.str = table[1];
			ch->abilities.con = table[2];
			ch->abilities.intel = table[3];
			ch->abilities.wis = table[4];
		} break;
		case CLASS_WARRIOR: {
			ch->abilities.str = table[0];
			ch->abilities.dex = table[1];
			ch->abilities.con = table[2];
			ch->abilities.wis = table[3];
			ch->abilities.intel = table[4];
			if (ch->abilities.str == 18)
				ch->abilities.str_add = number(0,100);
		} break;
	}
	ch->tmpabilities = ch->abilities;
}



void do_start(struct char_data *ch)
{
	int i, j;
	byte table[5];
	byte rools[4];

	extern struct dex_skill_type dex_app_skill[];
	void advance_level(struct char_data *ch);

	send_to_char("Welcome. This is now you character in DikuMud,\r\nYou can now earn XP, and lots more...\r\n", ch);

	GET_LEVEL(ch) = 1;
	GET_EXP(ch) = 1;

	set_title(ch);

	roll_abilities(ch);

	ch->points.max_hit  = 10;  /* These are BASE numbers   */

	switch (GET_CLASS(ch)) {

		case CLASS_MAGIC_USER : {
		} break;

		case CLASS_CLERIC : {
		} break;

		case CLASS_THIEF : {
			ch->skills[SKILL_SNEAK].learned = 10;
			ch->skills[SKILL_HIDE].learned =  5;
			ch->skills[SKILL_STEAL].learned = 15;
			ch->skills[SKILL_BACKSTAB].learned = 10;
			ch->skills[SKILL_PICK_LOCK].learned = 10;
		} break;

		case CLASS_WARRIOR: {
		} break;
	}

	advance_level(ch);

	GET_HIT(ch) = hit_limit(ch);
	GET_MANA(ch) = mana_limit(ch);
	GET_MOVE(ch) = move_limit(ch);

	GET_COND(ch,THIRST) = 24;
	GET_COND(ch,FULL) = 24;
	GET_COND(ch,DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

}


void do_advance(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[100], level[100], buf[240];
	int adv, newlevel;

	void gain_exp(struct char_data *ch, int gain);

	if (IS_NPC(ch))
		return;

	argument_interpreter(argument, name, level);

	if (*name)
	{
		if (!(victim = get_char_room_vis(ch, name)))
		{
			send_to_char("That player is not here.\r\n", ch);
			return;
		}
	} else {
		send_to_char("Advance who?\r\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		send_to_char("NO! Not on NPC's.\r\n", ch);
		return;
	}

	if (GET_LEVEL(victim) == 0)
		adv = 1;
	else if (!*level) {
		send_to_char("You must supply a level number.\r\n", ch);
		return;
	} else {
		if (!isdigit(*level))
		{
			send_to_char("Second argument must be a positive integer.\r\n",ch);
			return;
		}
		if ((newlevel = atoi(level)) <= GET_LEVEL(victim))
		{
			send_to_char("Can't dimish a players status (yet).\r\n", ch);
			return;
		}
		adv = newlevel - GET_LEVEL(victim);
	}

	if (((adv + GET_LEVEL(victim)) > 1) && (GET_LEVEL(ch) < 24))
	{
		send_to_char("Thou art not godly enough.\r\n", ch);
		return;
	}

	if ((adv + GET_LEVEL(victim)) > 24)
	{
		send_to_char("24 is the highest possible level.\r\n", ch);
		return;
	}

	if (((adv + GET_LEVEL(victim)) < 21)&&((adv + GET_LEVEL(victim)) != 1))
	{
		send_to_char("21 is the lowest possible level.\r\n", ch);
		return;
	}

	send_to_char("You feel generous.\r\n", ch);
  act("$n makes some strange gestures.\r\nA strange feeling comes uppon you,"
			"\r\nLike a giant hand, light comes down from\r\nabove, grabbing your "
			"body, that begins\r\nto pulse with coloured lights from inside.\r\nYo"
			"ur head seems to be filled with deamons\r\nfrom another plane as your"
			" body dissolves\r\nto the elements of time and space itself.\r\nSudde"
			"nly a silent explosion of light snaps\r\nyou back to reality. You fee"
			"l slightly\r\ndifferent.",FALSE,ch,0,victim,TO_VICT);


	if (GET_LEVEL(victim) == 0) {
		do_start(victim);
	} else {
		if (GET_LEVEL(victim) < 24) {
			gain_exp_regardless(victim, (titles[GET_CLASS(victim)-1][
				GET_LEVEL(victim)+adv].exp)-GET_EXP(victim));
			send_to_char("WARNING! Was not level 0.\r\n", ch);
		} else {
			send_to_char("Some idiot just tried to advance your level.\r\n",
				victim);
			send_to_char("IMPOSSIBLE! IDIOTIC!\r\n", ch);
		}
	}
}


void do_reroll(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char buf[100];
	short target;

	if (IS_NPC(ch))
		return;

	one_argument(argument,buf);
	if (!*buf)
		send_to_char("Who do you wich to reroll?\r\n",ch);
	else
		if(!(victim = get_char(buf)))
			send_to_char("No-one by that name in the world.\r\n",ch);
		else {
			send_to_char("Rerolled...\r\n", ch);
			roll_abilities(victim);
		}
}


void do_restore(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char buf[100];
	int i;

	void update_pos( struct char_data *victim );

	if (IS_NPC(ch))
		return;

	one_argument(argument,buf);
	if (!*buf)
		send_to_char("Who do you wich to restore?\r\n",ch);
	else
		if(!(victim = get_char(buf)))
			send_to_char("No-one by that name in the world.\r\n",ch);
		else {
			GET_MANA(victim) = GET_MAX_MANA(victim);
			GET_HIT(victim) = GET_MAX_HIT(victim);
			GET_MOVE(victim) = GET_MAX_MOVE(victim);

			if (GET_LEVEL(victim) >= 21) {
				for (i = 0; i < MAX_SKILLS; i++) {
					victim->skills[i].learned = 100;
					victim->skills[i].recognise = TRUE;
				}

				if (GET_LEVEL(victim) >= 23) {
					victim->abilities.str_add = 100;
					victim->abilities.intel = 25;
					victim->abilities.wis = 25;
					victim->abilities.dex = 25;
					victim->abilities.str = 25;
					victim->abilities.con = 25;
				}
				victim->tmpabilities = victim->abilities;

			}
			update_pos( victim );
			send_to_char("Done.\r\n", ch);
			act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
		}
}




do_noshout(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	struct obj_data *dummy;
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, buf);

	if (!*buf)
		if (IS_SET(ch->specials.act, PLR_NOSHOUT))
		{
			send_to_char("You can now hear shouts again.\r\n", ch);
			REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
		}
		else
		{
			send_to_char("From now on, you won't hear shouts.\r\n", ch);
			SET_BIT(ch->specials.act, PLR_NOSHOUT);
		}
	else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\r\n", ch);
	else if (IS_NPC(vict))
		send_to_char("Can't do that to a beast.\r\n", ch);
	else if (GET_LEVEL(vict) > GET_LEVEL(ch))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
	else if (IS_SET(vict->specials.act, PLR_NOSHOUT))
	{
		send_to_char("You can shout again.\r\n", vict);
		send_to_char("NOSHOUT removed.\r\n", ch);
		REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
	}
	else
	{
		send_to_char("The gods take away your ability to shout!\r\n", vict);
		send_to_char("NOSHOUT set.\r\n", ch);
		SET_BIT(vict->specials.act, PLR_NOSHOUT);
	}
}
