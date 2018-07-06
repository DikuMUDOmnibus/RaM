/*
 * RAM $Id: alias.c 42 2008-11-07 13:00:29Z quixadhal $
 */

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "strings.h"
#include "interp.h"

/* does aliasing and other fun stuff */
void substitute_alias( DESCRIPTOR_DATA *d, char *argument )
{
    CHAR_DATA              *ch = NULL;
    char                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                    prefix[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                    name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    const char             *point = NULL;
    int                     alias = 0;

    ch = d->original ? d->original : d->character;

    /*
     * check for prefix 
     */
    if ( ch->prefix[0] != '\0' && str_prefix( "prefix", argument ) )
    {
        if ( strlen( ch->prefix ) + strlen( argument ) > MAX_INPUT_LENGTH )
            ch_printf( ch, "Line to long, prefix not processed.\r\n" );
        else
        {
            sprintf( prefix, "%s %s", ch->prefix, argument );
            argument = prefix;
        }
    }

    if ( IS_NPC( ch ) || ch->pcdata->alias[0] == NULL
         || !str_prefix( "alias", argument ) || !str_prefix( "una", argument )
         || !str_prefix( "prefix", argument ) )
    {
        interpret( d->character, argument );
        return;
    }

    strcpy( buf, argument );

    for ( alias = 0; alias < MAX_ALIAS; alias++ )      /* go through the aliases */
    {
        if ( ch->pcdata->alias[alias] == NULL )
            break;

        if ( !str_prefix( ch->pcdata->alias[alias], argument ) )
        {
            point = one_argument( argument, name );
            if ( !strcmp( ch->pcdata->alias[alias], name ) )
            {
                buf[0] = '\0';
                strcat( buf, ch->pcdata->alias_sub[alias] );
                strcat( buf, " " );
                strcat( buf, point );

                if ( strlen( buf ) > MAX_INPUT_LENGTH - 1 )
                {
                    ch_printf( ch, "Alias substitution too long. Truncated.\r\n" );
                    buf[MAX_INPUT_LENGTH - 1] = '\0';
                }
                break;
            }
        }
    }
    interpret( d->character, buf );
}

void do_alia( CHAR_DATA *ch, const char *argument )
{
    ch_printf( ch, "I'm sorry, alias must be entered in full.\r\n" );
    return;
}

void do_alias( CHAR_DATA *ch, const char *argument )
{
    CHAR_DATA              *rch = NULL;
    char                    arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                     pos = 0;
    char                    local_argument[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    const char             *lap = local_argument;

    strcpy( local_argument, argument );
    smash_tilde( local_argument );

    if ( ch->desc == NULL )
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if ( IS_NPC( rch ) )
        return;

    lap = one_argument( lap, arg );

    if ( arg[0] == '\0' )
    {

        if ( rch->pcdata->alias[0] == NULL )
        {
            ch_printf( ch, "You have no aliases defined.\r\n" );
            return;
        }
        ch_printf( ch, "Your current aliases are:\r\n" );

        for ( pos = 0; pos < MAX_ALIAS; pos++ )
        {
            if ( rch->pcdata->alias[pos] == NULL || rch->pcdata->alias_sub[pos] == NULL )
                break;

            ch_printf( ch, "    %s:  %s\r\n", rch->pcdata->alias[pos],
                       rch->pcdata->alias_sub[pos] );
        }
        return;
    }

    if ( !str_prefix( "una", arg ) || !str_cmp( "alias", arg ) )
    {
        ch_printf( ch, "Sorry, that word is reserved.\r\n" );
        return;
    }

    if ( lap[0] == '\0' )
    {
        for ( pos = 0; pos < MAX_ALIAS; pos++ )
        {
            if ( rch->pcdata->alias[pos] == NULL || rch->pcdata->alias_sub[pos] == NULL )
                break;

            if ( !str_cmp( arg, rch->pcdata->alias[pos] ) )
            {
                ch_printf( ch, "%s aliases to '%s'.\r\n", rch->pcdata->alias[pos],
                           rch->pcdata->alias_sub[pos] );
                return;
            }
        }

        ch_printf( ch, "That alias is not defined.\r\n" );
        return;
    }

    if ( !str_prefix( lap, "delete" ) || !str_prefix( lap, "prefix" ) )
    {
        ch_printf( ch, "That shall not be done!\r\n" );
        return;
    }

    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
        if ( rch->pcdata->alias[pos] == NULL )
            break;

        if ( !str_cmp( arg, rch->pcdata->alias[pos] ) ) /* redefine an alias */
        {
            free_string( rch->pcdata->alias_sub[pos] );
            rch->pcdata->alias_sub[pos] = str_dup( lap );
            ch_printf( ch, "%s is now realiased to '%s'.\r\n", arg, lap );
            return;
        }
    }

    if ( pos >= MAX_ALIAS )
    {
        ch_printf( ch, "Sorry, you have reached the alias limit.\r\n" );
        return;
    }

    /*
     * make a new alias 
     */
    rch->pcdata->alias[pos] = str_dup( arg );
    rch->pcdata->alias_sub[pos] = str_dup( lap );
    ch_printf( ch, "%s is now aliased to '%s'.\r\n", arg, lap );
}

void do_unalias( CHAR_DATA *ch, const char *argument )
{
    CHAR_DATA              *rch = NULL;
    char                    arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                     pos = 0;
    bool                    found = false;

    if ( ch->desc == NULL )
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if ( IS_NPC( rch ) )
        return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch_printf( ch, "Unalias what?\r\n" );
        return;
    }

    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
        if ( rch->pcdata->alias[pos] == NULL )
            break;

        if ( found )
        {
            rch->pcdata->alias[pos - 1] = rch->pcdata->alias[pos];
            rch->pcdata->alias_sub[pos - 1] = rch->pcdata->alias_sub[pos];
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            continue;
        }

        if ( !strcmp( arg, rch->pcdata->alias[pos] ) )
        {
            ch_printf( ch, "Alias removed.\r\n" );
            free_string( rch->pcdata->alias[pos] );
            free_string( rch->pcdata->alias_sub[pos] );
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            found = true;
        }
    }

    if ( !found )
        ch_printf( ch, "No alias of that name to remove.\r\n" );
}
