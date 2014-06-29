#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <string.h>

static void activate_action (GtkAction *action);

static GtkActionEntry entries[] = {
  { "Show",      "document-new", "_Show", "<control>S",
    "Show status window", G_CALLBACK (activate_action) },
  { "Quit",     "application-exit", "_Quit", "<control>Q",
    "Exit the application", G_CALLBACK (gtk_main_quit) },
};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <popup name='IndicatorPopup'>"
"    <menuitem action='Show' />"
"    <menuitem action='Quit' />"
"  </popup>"
"</ui>";

char *cmddir  = "";

char *lowercase(const char *s)
{
 int  i;
 char *str = strdup(s);

 for(i = 0; str[i]; i++) {
   str[i] = tolower(str[i]);
 }

 return str;
}

static void
activate_action (GtkAction *action)
{
 char *cmdfile, *cmd;
 struct timespec tp;
 size_t siz;
 const gchar *name = gtk_action_get_name (action);
 FILE *fp;

 tp.tv_sec = 0;
 tp.tv_nsec = 0;

 clock_gettime(CLOCK_MONOTONIC, &tp);

 siz     = strlen(cmddir) + 128;
 cmdfile = malloc(siz);
 cmd     = lowercase(name);

 snprintf(cmdfile, siz, "%s/cmd.%09lld-%09ld.%s",
          cmddir, tp.tv_sec, tp.tv_nsec, cmd);

 if(fp = fopen(cmdfile, "w")) fclose(fp);

 free(cmd);
 free(cmdfile);
}

int main (int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *indicator_menu;
  GtkActionGroup *action_group;
  GtkUIManager *uim;
  AppIndicator *indicator;
  GError *error = NULL;

  if(argc < 4) exit(1);

  cmddir = argv[3];

  gtk_init (&argc, &argv);

  /* main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Trivial indicator");
  gtk_window_set_icon_name (GTK_WINDOW (window), "indicator-messages-new");
  g_signal_connect (G_OBJECT (window),
                    "destroy",
                    G_CALLBACK (gtk_main_quit),
                    NULL);


  /* Menus */
  action_group = gtk_action_group_new ("AppActions");
  gtk_action_group_add_actions (action_group,
                                entries, n_entries,
                                window);

  uim = gtk_ui_manager_new ();
  g_object_set_data_full (G_OBJECT (window),
                          "ui-manager", uim,
                          g_object_unref);
  gtk_ui_manager_insert_action_group (uim, action_group, 0);
  gtk_window_add_accel_group (GTK_WINDOW (window),
                              gtk_ui_manager_get_accel_group (uim));

  if (!gtk_ui_manager_add_ui_from_string (uim, ui_info, -1, &error))
    {
      g_message ("Failed to build menus: %s\n", error->message);
      g_error_free (error);
      error = NULL;
    }

  gtk_window_set_default_size (GTK_WINDOW (window),
                               200, 200);

  /* Show the window */
  /* gtk_widget_show_all (window); */

  /* Indicator */
  indicator = app_indicator_new (argv[1], argv[2],
                                 APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  indicator_menu = gtk_ui_manager_get_widget (uim, "/ui/IndicatorPopup");

  app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon (indicator, argv[2]);
  app_indicator_set_menu (indicator, GTK_MENU (indicator_menu));

  gtk_main ();

  return 0;
}
