/* 
Copyright 2010 Rorschach <r0rschach@lavabit.com>,
2011 Andrew Kravchuk <awkravchuk@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include <libnotify/notify.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define VERSION_NUMBER 1.4

/* Prints the help. */
int print_help(char *name)
{
	printf("Usage: %s [options]\n\n",name);
	printf("Options:\n");
	printf("          --command|-c [value]        Set the command which gives out the list of updates.\n");
        printf("                                      The default is /usr/bin/pacman -Qu\n");
	printf("          --icon|-p [value]           Shows the icon, whose path has been given as value, in the notification.\n");
	printf("                                      By default no icon is shown.\n");
	printf("          --maxentries|-m [value]     Set the maximum number of packages which shall be displayed in the notification.\n");
        printf("                                      The default value is 30.\n");
	printf("          --timeout|-t [value]        Set the timeout after which the notification disappers in seconds.\n");
	printf("                                      The default value is 3600 seconds, which is 1 hour.\n");
        printf("          --uid|-i [value]            Set the uid of the process.\n");
        printf("                                      The default is to keep the uid of the user who started archup.\n");
        printf("                                      !!You should change this if root is executing archup!!\n");
	printf("          --urgency|-u [value]        Set the libnotify urgency-level. Possible values are: low, normal and critical.\n");
        printf("                                      The default value is normal. With changing this value you can change the color of the notification.\n");
	printf("          --help                      Prints this help.\n");
	printf("          --version                   Shows the version.\n");
	printf("\nMore informations can be found in the manpage.\n");
	exit(0);
}

/* Prints the version. */
int print_version()
{
        printf("archup %1.1f\n",VERSION_NUMBER);
	printf("Copyright 2010 Rorschach <r0rschach@lavabit.com>,\n2011 Andrew Kravchuk <awkravchuk@gmail.com>\n");
        printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
        printf("This is free software: you are free to change and redistribute it.\n");
        printf("There is NO WARRANTY, to the extent permitted by law.\n");
        exit(0);
}

int main(int argc, char **argv)
{
	typedef int bool;
	NotifyUrgency urgency;


	/* Default timeout-value is: 60 min (int timeout = 3600*1000;)
	   After this time the desktop notification disappears */
	int timeout = 3600*1000;
	/* Restricts the number of packages which should be included in the desktop notification.*/
	int max_number_out = 30;
	/* Sets the urgency-level to normal. */
	urgency = NOTIFY_URGENCY_NORMAL;
	/* The default command to get a list of packages to update. */
	char *command = "/usr/bin/pacman -Qu";
	/* The default icon to show: none */
	gchar *icon = NULL;

	/* We parse the commandline options. */
	int i;
	for (i=1;i<argc;i++)
	{
		if ( strcmp(argv[i],"--help") == 0 )
		{
			print_help(argv[0]);
		} 
		else if ( strcmp(argv[i],"--version") == 0 )
		{
			print_version();
		}
		else if  ( strcmp(argv[i],"--timeout") == 0 ||  strcmp(argv[i],"-t") == 0 )
		{
			/* If argv[i] is not the last command line option (because
 			   if it is, there's no place left for the value) and the next
                           value is a digit, we take this as new value */
			if ( (argc-1 != i) && isdigit(*argv[i+1]) )
			{
				timeout = atoi(argv[i+1])*1000;	
			}
		}
                else if  ( strcmp(argv[i],"--maxentries") == 0 ||  strcmp(argv[i],"-m") == 0 )
                {
                        if ( (argc-1 != i) && isdigit(*argv[i+1]) )
                        {
                                max_number_out = atoi(argv[i+1]);
                        }
                }
                else if  ( strcmp(argv[i],"--urgency") == 0 ||  strcmp(argv[i],"-u") == 0 )
                {
                        if ( (argc-1 != i) )
                        {
				if ( strcmp(argv[i+1],"low") == 0 )
				{
					urgency=NOTIFY_URGENCY_LOW;
				}
				else if ( strcmp(argv[i+1],"normal") == 0 )
                        	{
                                	urgency=NOTIFY_URGENCY_NORMAL;
                                }        
				else if ( strcmp(argv[i+1],"critical") == 0 )
                                {
                                        urgency=NOTIFY_URGENCY_CRITICAL;
                                }
			}
                }
                else if  ( strcmp(argv[i],"--command") == 0 ||  strcmp(argv[i],"-c") == 0 )
                {
                        if ( (argc-1 != i) )
                        {
				command = argv[i+1];
                        }
                }
                else if  ( strcmp(argv[i],"--uid") == 0 ||  strcmp(argv[i],"-i") == 0 )
                {
                        if ( (argc-1 != i) && isdigit(*argv[i+1]) )
                        {
                        	if ( setuid(atoi(argv[i+1])) != 0 )
				{
				printf("Couldn't change to the given uid!\n");
				exit(1);
				}
                        }
                }
                else if  ( strcmp(argv[i],"--icon") == 0 ||  strcmp(argv[i],"-p") == 0 )
                {
                        if ( (argc-1 != i) )
                        {
                                icon = argv[i+1];
                        }
                }

	}

	/* Those are needed by libnotify. */
	char *name = "arch_update_check";
	char *category = "update";
        NotifyNotification *my_notify;
        GError *error = NULL;

        /* Those are needed for the output. */
	char *output_string=malloc(23); 
	sprintf(output_string,"There are updates for:\n");
	bool got_updates = FALSE;

	/* Those are needed for getting the list of updates. */
	FILE *pac_out;
	int llen = 0;
	char line[BUFSIZ];

 
	/* We get stdout of pacman -Qu into the pac_out stream.
	   Remember we can't use fseek(stream,0,SEEK_END) with 
	   popen-streams, thus we are reading BUFSIZ sized lines
	   and allocate dynamically more memory for our output. */  
        pac_out = popen(command,"r");

	i = 0;
	while (fgets(line,BUFSIZ,pac_out)) 
	{
		/* We leave the loop if we have more updates than we want to show in the notification. */
		if (i >= max_number_out)
		{
			break;
		}
		i++;

		/* If we are in this loop, we got updates waiting. */
		got_updates = TRUE;
		/* We get the length of the current line. */
		llen = strlen(line);
		/* We allocate that much more memory+2 bytes for the "- "+1 byte as delimiter. */
		output_string = (char *)realloc(output_string,strlen(output_string)+1+llen+2);
		/* We add the line to the output string. */
		strncat(output_string,"- ",2);
		strncat(output_string,line,llen);
	}

	/* We close the popen stream if we don't need it anymore. */
	pclose(pac_out);

	/* If we got updates we are showing them in a notification */
	if (got_updates == TRUE)
	{
		/* Initiates the libnotify. */	
		notify_init(name);
		/* Removes the last newlinefeed of the output_string, if the last sign is a newlinefeed. */
		if ( output_string[strlen(output_string)-1] == '\n' )
		{
			output_string[strlen(output_string)-1] = '\0';
		}
		/* Constructs the notification. */
		my_notify = notify_notification_new("New updates for Archlinux available!",output_string,icon);
		/* Sets the timeout until the notification disappears. */
		notify_notification_set_timeout(my_notify,timeout);
		/* We set the category.*/
		notify_notification_set_category(my_notify,category);
		/* We set the urgency, which can be changed with a commandline option */
		notify_notification_set_urgency (my_notify,urgency);
		/* We finally show the notification, */	
		notify_notification_show(my_notify,&error);
		/* and deinitialize the libnotify afterwards. */
		notify_uninit();
		/* Should be safe now */
		free(output_string);
	}

	return(0);
}
