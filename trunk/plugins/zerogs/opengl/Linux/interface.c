/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_Config (void)
{
  GtkWidget *Config;
  GtkWidget *vbox4;
  GtkWidget *checkInterlace;
  GtkWidget *checkBilinear;
  GtkWidget *frame4;
  GtkWidget *alignment1;
  GtkWidget *hbox6;
  GtkWidget *radioAANone;
  GSList *radioAANone_group = NULL;
  GtkWidget *radioAA2X;
  GtkWidget *radioAA4X;
  GtkWidget *radioAA8X;
  GtkWidget *radioAA16X;
  GtkWidget *label9;
  GtkWidget *checkWireframe;
  GtkWidget *checkAVI;
  GtkWidget *checkTGA;
  GtkWidget *checkfullscreen;
  GtkWidget *frame5;
  GtkWidget *alignment2;
  GtkWidget *hbox7;
  GtkWidget *radioSize640;
  GSList *radioSize640_group = NULL;
  GtkWidget *radioSize800;
  GtkWidget *radioSize1024;
  GtkWidget *radioSize1280;
  GtkWidget *label10;
  GtkWidget *frame6;
  GtkWidget *alignment3;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview1;
  GtkWidget *label12;
  GtkWidget *label11;
  GtkWidget *hbuttonbox1;
  GtkWidget *button1;
  GtkWidget *button2;

  Config = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (Config), 5);
  gtk_window_set_title (GTK_WINDOW (Config), _("ZeroOGS Configuration"));

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (Config), vbox4);

  checkInterlace = gtk_check_button_new_with_mnemonic (_("Interlace Enable (toggle with F5)\n   there are 2 modes + interlace off"));
  gtk_widget_show (checkInterlace);
  gtk_box_pack_start (GTK_BOX (vbox4), checkInterlace, FALSE, FALSE, 0);

  checkBilinear = gtk_check_button_new_with_mnemonic (_("Bilinear Filtering (Shift+F5)\n   Best quality is on, turn off for speed"));
  gtk_widget_show (checkBilinear);
  gtk_box_pack_start (GTK_BOX (vbox4), checkBilinear, FALSE, FALSE, 0);

  frame4 = gtk_frame_new (NULL);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (vbox4), frame4, TRUE, TRUE, 0);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame4), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 12, 0);

  hbox6 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox6);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox6);

  radioAANone = gtk_radio_button_new_with_mnemonic (NULL, _("None"));
  gtk_widget_show (radioAANone);
  gtk_box_pack_start (GTK_BOX (hbox6), radioAANone, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioAANone), radioAANone_group);
  radioAANone_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioAANone));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioAANone), TRUE);

  radioAA2X = gtk_radio_button_new_with_mnemonic (NULL, _("2X"));
  gtk_widget_show (radioAA2X);
  gtk_box_pack_start (GTK_BOX (hbox6), radioAA2X, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioAA2X), radioAANone_group);
  radioAANone_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioAA2X));

  radioAA4X = gtk_radio_button_new_with_mnemonic (NULL, _("4X"));
  gtk_widget_show (radioAA4X);
  gtk_box_pack_start (GTK_BOX (hbox6), radioAA4X, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioAA4X), radioAANone_group);
  radioAANone_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioAA4X));

  radioAA8X = gtk_radio_button_new_with_mnemonic (NULL, _("8X"));
  gtk_widget_show (radioAA8X);
  gtk_box_pack_start (GTK_BOX (hbox6), radioAA8X, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioAA8X), radioAANone_group);
  radioAANone_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioAA8X));

  radioAA16X = gtk_radio_button_new_with_mnemonic (NULL, _("16X"));
  gtk_widget_show (radioAA16X);
  gtk_box_pack_start (GTK_BOX (hbox6), radioAA16X, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioAA16X), radioAANone_group);
  radioAANone_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioAA16X));

  label9 = gtk_label_new (_("<b>Anti-aliasing for higher quality (F6)</b>"));
  gtk_widget_show (label9);
  gtk_frame_set_label_widget (GTK_FRAME (frame4), label9);
  gtk_label_set_use_markup (GTK_LABEL (label9), TRUE);

  checkWireframe = gtk_check_button_new_with_mnemonic (_("Wireframe rendering (Shift+F6)"));
  gtk_widget_show (checkWireframe);
  gtk_box_pack_start (GTK_BOX (vbox4), checkWireframe, FALSE, FALSE, 0);

  checkAVI = gtk_check_button_new_with_mnemonic (_("Capture Avi (zerogs.avi)(F7)"));
  gtk_widget_show (checkAVI);
  gtk_box_pack_start (GTK_BOX (vbox4), checkAVI, FALSE, FALSE, 0);

  checkTGA = gtk_check_button_new_with_mnemonic (_("Save Snapshots as TGAs (default is JPG)"));
  gtk_widget_show (checkTGA);
  gtk_box_pack_start (GTK_BOX (vbox4), checkTGA, FALSE, FALSE, 0);

  checkfullscreen = gtk_check_button_new_with_mnemonic (_("Fullscreen (Alt+Enter)\n   to get out press Alt+Enter again (or ESC)"));
  gtk_widget_show (checkfullscreen);
  gtk_box_pack_start (GTK_BOX (vbox4), checkfullscreen, FALSE, FALSE, 0);

  frame5 = gtk_frame_new (NULL);
  gtk_widget_show (frame5);
  gtk_box_pack_start (GTK_BOX (vbox4), frame5, TRUE, TRUE, 0);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame5), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 12, 0);

  hbox7 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox7);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox7);

  radioSize640 = gtk_radio_button_new_with_mnemonic (NULL, _("640x480"));
  gtk_widget_show (radioSize640);
  gtk_box_pack_start (GTK_BOX (hbox7), radioSize640, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioSize640), radioSize640_group);
  radioSize640_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioSize640));

  radioSize800 = gtk_radio_button_new_with_mnemonic (NULL, _("800x600"));
  gtk_widget_show (radioSize800);
  gtk_box_pack_start (GTK_BOX (hbox7), radioSize800, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioSize800), radioSize640_group);
  radioSize640_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioSize800));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioSize800), TRUE);

  radioSize1024 = gtk_radio_button_new_with_mnemonic (NULL, _("1024x768"));
  gtk_widget_show (radioSize1024);
  gtk_box_pack_start (GTK_BOX (hbox7), radioSize1024, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioSize1024), radioSize640_group);
  radioSize640_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioSize1024));

  radioSize1280 = gtk_radio_button_new_with_mnemonic (NULL, _("1280x960"));
  gtk_widget_show (radioSize1280);
  gtk_box_pack_start (GTK_BOX (hbox7), radioSize1280, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioSize1280), radioSize640_group);
  radioSize640_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioSize1280));

  label10 = gtk_label_new (_("<b>Default Window Size (no speed impact)</b>"));
  gtk_widget_show (label10);
  gtk_frame_set_label_widget (GTK_FRAME (frame5), label10);
  gtk_label_set_use_markup (GTK_LABEL (label10), TRUE);

  frame6 = gtk_frame_new (NULL);
  gtk_widget_show (frame6);
  gtk_box_pack_start (GTK_BOX (vbox4), frame6, TRUE, TRUE, 0);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (frame6), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 12, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (alignment3), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

  treeview1 = gtk_tree_view_new ();
  gtk_widget_show (treeview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);

  label12 = gtk_label_new (_("<b>Advanced Options</b>"));
  gtk_widget_show (label12);
  gtk_frame_set_label_widget (GTK_FRAME (frame6), label12);
  gtk_label_set_use_markup (GTK_LABEL (label12), TRUE);

  label11 = gtk_label_new (_("Show Frames Per Second (Shift+F7)\n   (value is the average over 4-16 PS2 frames)"));
  gtk_widget_show (label11);
  gtk_box_pack_start (GTK_BOX (vbox4), label11, FALSE, FALSE, 0);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox4), hbuttonbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_SPREAD);
  gtk_box_set_spacing (GTK_BOX (hbuttonbox1), 30);

  button1 = gtk_button_new_with_mnemonic (_("Ok"));
  gtk_widget_show (button1);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button1);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_with_mnemonic (_("Cancel"));
  gtk_widget_show (button2);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button2);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) button1, "clicked",
                    G_CALLBACK (OnConf_Ok),
                    NULL);
  g_signal_connect ((gpointer) button2, "clicked",
                    G_CALLBACK (OnConf_Cancel),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (Config, Config, "Config");
  GLADE_HOOKUP_OBJECT (Config, vbox4, "vbox4");
  GLADE_HOOKUP_OBJECT (Config, checkInterlace, "checkInterlace");
  GLADE_HOOKUP_OBJECT (Config, checkBilinear, "checkBilinear");
  GLADE_HOOKUP_OBJECT (Config, frame4, "frame4");
  GLADE_HOOKUP_OBJECT (Config, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (Config, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (Config, radioAANone, "radioAANone");
  GLADE_HOOKUP_OBJECT (Config, radioAA2X, "radioAA2X");
  GLADE_HOOKUP_OBJECT (Config, radioAA4X, "radioAA4X");
  GLADE_HOOKUP_OBJECT (Config, radioAA8X, "radioAA8X");
  GLADE_HOOKUP_OBJECT (Config, radioAA16X, "radioAA16X");
  GLADE_HOOKUP_OBJECT (Config, label9, "label9");
  GLADE_HOOKUP_OBJECT (Config, checkWireframe, "checkWireframe");
  GLADE_HOOKUP_OBJECT (Config, checkAVI, "checkAVI");
  GLADE_HOOKUP_OBJECT (Config, checkTGA, "checkTGA");
  GLADE_HOOKUP_OBJECT (Config, checkfullscreen, "checkfullscreen");
  GLADE_HOOKUP_OBJECT (Config, frame5, "frame5");
  GLADE_HOOKUP_OBJECT (Config, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (Config, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (Config, radioSize640, "radioSize640");
  GLADE_HOOKUP_OBJECT (Config, radioSize800, "radioSize800");
  GLADE_HOOKUP_OBJECT (Config, radioSize1024, "radioSize1024");
  GLADE_HOOKUP_OBJECT (Config, radioSize1280, "radioSize1280");
  GLADE_HOOKUP_OBJECT (Config, label10, "label10");
  GLADE_HOOKUP_OBJECT (Config, frame6, "frame6");
  GLADE_HOOKUP_OBJECT (Config, alignment3, "alignment3");
  GLADE_HOOKUP_OBJECT (Config, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (Config, treeview1, "treeview1");
  GLADE_HOOKUP_OBJECT (Config, label12, "label12");
  GLADE_HOOKUP_OBJECT (Config, label11, "label11");
  GLADE_HOOKUP_OBJECT (Config, hbuttonbox1, "hbuttonbox1");
  GLADE_HOOKUP_OBJECT (Config, button1, "button1");
  GLADE_HOOKUP_OBJECT (Config, button2, "button2");

  return Config;
}

GtkWidget*
create_About (void)
{
  GtkWidget *About;
  GtkWidget *vbox2;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *hbuttonbox2;
  GtkWidget *button3;

  About = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (About), 5);
  gtk_window_set_title (GTK_WINDOW (About), _("ZeroGS KOSMOS About"));

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (About), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  label2 = gtk_label_new (_("OpenGL version"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (vbox2), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_CENTER);

  label3 = gtk_label_new (_("Author: zerofrog(@gmail.com)"));
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (vbox2), label3, FALSE, FALSE, 0);

  label4 = gtk_label_new (_("Many thanks to the Pcsx2 testing team"));
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (vbox2), label4, FALSE, FALSE, 0);

  hbuttonbox2 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox2);
  gtk_box_pack_start (GTK_BOX (vbox2), hbuttonbox2, TRUE, TRUE, 0);
  gtk_box_set_spacing (GTK_BOX (hbuttonbox2), 30);

  button3 = gtk_button_new_with_mnemonic (_("Ok"));
  gtk_widget_show (button3);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), button3);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) button3, "clicked",
                    G_CALLBACK (OnAbout_Ok),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (About, About, "About");
  GLADE_HOOKUP_OBJECT (About, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (About, label2, "label2");
  GLADE_HOOKUP_OBJECT (About, label3, "label3");
  GLADE_HOOKUP_OBJECT (About, label4, "label4");
  GLADE_HOOKUP_OBJECT (About, hbuttonbox2, "hbuttonbox2");
  GLADE_HOOKUP_OBJECT (About, button3, "button3");

  return About;
}

