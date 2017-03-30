#ifndef __MONSTER_WRAPPER_H
#define __MONSTER_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct Monster Monster;
Monster * newMonster();
int Monster_get_x(Monster * m);
int Monster_get_y(Monster * m);
int Monster_get_decimal_type(Monster * m);
int Monster_get_last_known_player_x(Monster * m);
int Monster_get_last_known_player_y(Monster * m);
int Monster_get_speed(Monster * m);

void Monster_set_x(Monster * m, int v);
void Monster_set_y(Monster *m, int v);
void Monster_set_decimal_type(Monster * m, int v);
void Monster_set_last_known_player_x(Monster * m, int v);
void Monster_set_last_known_player_y(Monster * m, int v);
void Monster_set_speed(Monster * m, int v);
void Monster_reset_player_location(Monster *m);

void deleteMonster(Monster * m);

#ifdef __cplusplus
}
#endif
#endif
