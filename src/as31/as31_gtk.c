/* ----------------------------------------------------------------------
 * FILE: main.c
 * PACKAGE: as31 - 8031/8051 Assembler.
 *
 * DESCRIPTION:
 *	The file contains main(). It handles the arguments and makes
 *	sure that pass 1 is done before pass 2 etc...
 *
 * REVISION HISTORY:
 *	Jan. 19, 1990 - Created. (Ken Stauffer)
 *
 * AUTHOR:
 *	All code in this file written by Ken Stauffer (University of Calgary).
 *	January, 1990. "Written by: Ken Stauffer"
 *
 *	April, 2000, Paul Stoffregen: see Makefile for details
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "as31.h"


void do_assemble(GtkWidget *widget, gpointer *data);
gint delete_handler(GtkWidget *widget, gpointer *data);


/* make all the GTK widgets global, instead of passing lots */
/* of pointers into the callbacks, through the parser, etc */

GtkWidget *file_label, *file_entry, *otype_combo;
GtkWidget *list_check, *go_button;
GtkWidget *otype_label, *output_text;
GtkWidget *main_window, *main_vbox;
GtkWidget *top_hbox, *middle_hbox, *bottom_hbox;


int main(int argc, char **argv)
{
	GList *otypelist;
	const char *desc;
	int n;

	gtk_init(&argc, &argv);

	/* create all the gtk widgets... it's a pain to type in all */
	/* this stuff, maybe someday I'll have to learn about glade */

	file_label = gtk_label_new("ASM File: ");
	gtk_label_set_justify(GTK_LABEL(file_label), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(file_label);

	otype_label = gtk_label_new("Output Format");
	gtk_label_set_justify(GTK_LABEL(otype_label), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(otype_label);

	file_entry = gtk_entry_new();
	/* fill in a default filename, from a previous session ??? */
	gtk_widget_show(file_entry);

	list_check = gtk_check_button_new_with_label("Create List File");
	/* check or unchecked based on previous usage */
	gtk_widget_show(list_check);

	otype_combo = gtk_combo_new();
	otypelist = NULL;
	n = 0;
	while ((desc = emit_desc_lookup(n++)) != NULL) {
		otypelist = g_list_append(otypelist, strdup(desc));
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(otype_combo), otypelist);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(otype_combo)->entry),
		FALSE);
	/* set to same output format as a previous run ?? */
	gtk_widget_show(otype_combo);

	go_button = gtk_button_new_with_label("Assemble");
	gtk_widget_show(go_button);

	output_text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(output_text), FALSE);
	gtk_widget_show(output_text);

	top_hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(top_hbox), file_label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(top_hbox), file_entry, TRUE, TRUE, 2);
        gtk_widget_show(top_hbox);

	middle_hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(middle_hbox), otype_label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(middle_hbox), otype_combo, FALSE, FALSE, 2);
        gtk_widget_show(middle_hbox);

	bottom_hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(bottom_hbox), go_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(bottom_hbox), list_check, FALSE, FALSE, 2);
        gtk_widget_show(bottom_hbox);

	main_vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), top_hbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), middle_hbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), bottom_hbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), output_text, TRUE, TRUE, 2);
        gtk_widget_show(main_vbox);

        main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
        gtk_widget_show(main_window);

	/* attach signal handlers so we can actually do something */

	gtk_signal_connect(GTK_OBJECT(main_window), "delete_event",
		GTK_SIGNAL_FUNC(delete_handler), NULL);

	gtk_signal_connect(GTK_OBJECT(go_button), "pressed",
		GTK_SIGNAL_FUNC(do_assemble), NULL);

	/* that's it, everything past here is a callback from GTK */

	mesg("WARNING: this is an alpha-quality release.\n");
	mesg("It WILL crash the second time you assemble !!\n");
	mesg("Please report any problems that occur on the\n");
	mesg("first assembly to: paul@pjrc.com\n");

	gtk_main();
	return 0;
}


/* actually do the assembly.  The entire assembly is done from */
/* start to finish in this callback attached to the "assemble now" */
/* button.  It's not so nice to spend a lot of CPU time in a */
/* callback, because all the gtk widgets stop responding until */
/* we're done, but it keeps things simple, and at least the */
/* window manager takes care of raise/lower and reposition, */
/* ...unlike ms windoze where those are handled by the */
/* application's event loop.  It would be "nice" to launch a */
/* thread that would do the assembly, but for a simple free */
/* software project (where the assembly completes rapidly on */
/* any modern cpu), I don't want to bother with all the unpleasant */
/* threading issues */

void
do_assemble(GtkWidget *widget, gpointer *data)
{
	const char *fmt=NULL, *filename;
	int r, do_lst;


	/* collect info from the GTK widgets */

	do_lst = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list_check));

	filename = gtk_entry_get_text(GTK_ENTRY(file_entry));
	if (filename == NULL || *filename == '\0') {
		mesg("You need to specify the ASM file\n");
		return;
	}

	fmt = emit_desc_to_name_lookup(gtk_entry_get_text(
		GTK_ENTRY(GTK_COMBO(otype_combo)->entry)));
	if (fmt == NULL) {
		mesg("Please select a valid output type\n");
		return;
	}

	/* now we can begin working like normal */

	r = run_as31(filename, do_lst, 0, fmt, NULL);

	if (r == 0) {
		mesg("Assembly complete, no errors :)\n");
	}
}


/* quit the program if they close it */

gint
delete_handler(GtkWidget *widget, gpointer *data)
{
	gtk_main_quit();
	return FALSE;
}


/* the assembler calls here to display any messages */

void mesg(const char *str)
{
	if (str == NULL) str = "(null)";
	/* printf("%s", str); */
	gtk_text_insert(GTK_TEXT(output_text), NULL, NULL,
		NULL, str, strlen(str));
}

