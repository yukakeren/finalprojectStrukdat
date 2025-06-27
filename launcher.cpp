#include <gtk/gtk.h>
#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

GtkWidget *entry_dna; 

void on_button_clicked(GtkWidget *widget, gpointer data) {
    const char *option = (const char *)data;

    const char *query = gtk_entry_get_text(GTK_ENTRY(entry_dna));
    if (strlen(query) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_OK, "Masukkan DNA terlebih dahulu.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    string filename = "dna_data_" + string(option) + ".csv";

    string command = "./lookup " + filename + " " + query;
    cout << "Running: " << command << endl;
    system(command.c_str());
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *label_data;
    GtkWidget *label_dna;
    GtkWidget *button100, *button500, *button1000;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pilih Ukuran Data & DNA");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 250);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    label_dna = gtk_label_new("Masukkan DNA:");
    gtk_box_pack_start(GTK_BOX(vbox), label_dna, FALSE, FALSE, 0);

    entry_dna = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry_dna, FALSE, FALSE, 0);

    label_data = gtk_label_new("Pilih ukuran data:");
    gtk_box_pack_start(GTK_BOX(vbox), label_data, FALSE, FALSE, 10);

    button100 = gtk_button_new_with_label("100 data");
    g_signal_connect(button100, "clicked", G_CALLBACK(on_button_clicked), (gpointer)"100");
    gtk_box_pack_start(GTK_BOX(vbox), button100, TRUE, TRUE, 0);

    button500 = gtk_button_new_with_label("500 data");
    g_signal_connect(button500, "clicked", G_CALLBACK(on_button_clicked), (gpointer)"500");
    gtk_box_pack_start(GTK_BOX(vbox), button500, TRUE, TRUE, 0);

    button1000 = gtk_button_new_with_label("1000 data");
    g_signal_connect(button1000, "clicked", G_CALLBACK(on_button_clicked), (gpointer)"1000");
    gtk_box_pack_start(GTK_BOX(vbox), button1000, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}