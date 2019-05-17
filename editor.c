#include <stdio.h>
#include <stdlib.h>

#define READ "r"
#define WRITE "w"

/* line functions */
struct line {
	struct line *prev;
	char *this;
	struct line *next;
};

struct line *first_line;
struct line *current_line;
int file_loaded;
int dirty;

/* file commands */
char *filename;
FILE *filedes;
FILE *open_file(char *fname, char *mode);
int read_in_file();

char *command;

/* prompt loop */
int edit_loop();

/* edit commands */
void edit_list();
void edit_append();
void edit_insert();
void edit_write();
void edit_delete();
void edit_print_current();
void edit_fileinfo();
void edit_rename();
void edit_switch_line();

int count_lines();
int get_current_line();

void clean_up();

int
main(int argc, char **argv)
{
	ssize_t nread;
	dirty = 0;
	file_loaded = 0;
	filedes = 0x0;
	size_t cmdlen = 0;

	if (argc == 2) {
		filedes = open_file(argv[1], READ);
	}

	if (filedes) {
		file_loaded = read_in_file();
	}

	filename = argv[1];
	current_line = first_line;

	do {
		printf("~ "); fflush(stdout);
		nread = getline(&command, &cmdlen, stdin);
	} while (nread <= 0 || edit_loop());

	clean_up();
}

FILE *
open_file(char *fname, char *mode)
{
	FILE *fptr = fopen(fname, mode);
	if (fptr)
		return fptr;
	fprintf(stderr, "Error opening file '%s' with mode '%s'\n", fname, mode);
	return 0x0;
}

int
read_in_file()
{
	size_t len = 0;
	ssize_t nread;
	char *readline = 0x0;
	struct line *newline;

	while ((nread = getline(&readline, &len, filedes)) != -1) {
		newline = (struct line *)malloc(sizeof(struct line));
		newline->this = readline;
		newline->next = 0x0;
		if (!first_line) {
			newline->prev = 0x0;
			first_line = newline;
			current_line = newline;
		} else {
			newline->prev = current_line;
			current_line->next = newline;
			current_line = newline;
		}
		len = 0;
		readline = 0x0;
	}
	fclose(filedes);
	return 1;
}

int
edit_loop()
{
	switch (*command) {
	case 'l':
		edit_list();
		return 1;
	case 'a':
		edit_append();
		return 1;
	case 'i':
		edit_insert();
		return 1;
	case 'w':
		edit_write();
		return 1;
	case 'g':
		edit_fileinfo();
		return 1;
	case '.':
		edit_print_current();
		return 1;
	case 'd':
		edit_delete();
		return 1;
	case 'n':
		edit_rename();
		return 1;
	case 'q':
		if (dirty) {
			printf("Your buffer is dirty!\n");
			dirty = 0;
			return 1;
		}
		return 0;
	default:
		edit_switch_line();
		return 1;
	}
}

void
edit_list()
{
	struct line *current = first_line;

	if (!first_line) {
		return;
	}

	while (current != 0x0) {
		printf("%s", current->this);
		current = current->next;
	}
}

void
edit_append()
{
	size_t len = 0;
	ssize_t nread;
	char *readline = 0x0;
	struct line *newline;
	struct line *following = current_line ? current_line->next : 0x0;

	while ((nread = getline(&readline, &len, stdin)) != -1) {
		if (nread == 2 && *readline == '.') {
			dirty = 1;
			return;
		}
		newline = (struct line *)malloc(sizeof(struct line));
		newline->this = readline;
		newline->next = following;
		if (!first_line) {
			first_line = newline;
			newline->prev = 0x0;
		} else {
			current_line->next = newline;
			newline->prev = current_line;
		}
		current_line = newline;
		len = 0;
		readline = 0x0;
	}
}

void
edit_insert()
{
	struct line *fakefirst;

	if (get_current_line() > 1) { // jump backwards and append
		current_line = current_line->prev;
		edit_append();
		return;
	}

	fakefirst = (struct line *)malloc(sizeof(struct line));
	fakefirst->next = first_line;
	fakefirst->prev = 0x0;
	fakefirst->this = 0x0;

	first_line = fakefirst;
	current_line = fakefirst;

	edit_append();

	first_line = fakefirst->next;
	free(fakefirst);
}

void
edit_delete()
{
	struct line *prev;
	struct line *next;

	if (current_line == 0x0)
		return;

	prev = current_line->prev;
	next = current_line->next;

	prev->next = next;
	if (next != 0x0)
		next->prev = prev;

	/* next is our new old */
	next = current_line;
	current_line = prev;
	free(next);
}

void
edit_write()
{
	struct line *current = first_line;

	filedes = open_file(filename, WRITE);

	if (!first_line) {
		fputs("", filedes);
		return;
	}

	while (current != 0x0) {
		fputs(current->this, filedes);
		current = current->next;
	}

	if (fclose(filedes) != 0) {
		fprintf(stderr, "Can not close file '%s'\n", filename);
	}
	printf("File written to '%s'\n", filename);
	dirty = 0;
}

int
get_current_line()
{
	struct line *current = first_line;
	int count = 0;

	while (current != 0x0 && current != current_line) {
		count++;
		current = current->next;
	}
	return count+1;
}

int
count_lines()
{
	struct line *current = first_line;
	int count = 0;

	while (current != 0x0) {
		count++;
		current = current->next;
	}
	return count;
}

void
edit_fileinfo()
{
	int cur_line = get_current_line();
	int nr_lines = count_lines();

	printf("File: %s  Lines: %d  Current Line: %d\n",
		filename,
		nr_lines,
		cur_line);
}

void
edit_rename()
{
	char *newname = 0x0;
	char *oldname = filename;
	size_t len = 0;
	ssize_t nread;
	printf("Enter new filename: ");
	fflush(stdout);
	while ((nread = getline(&newname, &len, stdin)) == -1 || nread < 1) {
		fprintf(stderr, "You cannot name the file %s\n", newname);
		free(newname);
		newname = 0x0;
		len = 0x0;
	}

	newname[nread-1] = 0x0;
	filename = newname;
}

void
edit_switch_line()
{
	struct line *current;
	int linenr;

	if (command == 0x0 || first_line == 0x0)
		return;

	linenr = atoi(command);
	current = first_line;

	while (linenr > 1) {
		if (current->next == 0x0) {
			fprintf(stderr, "Not enough lines!\n");
			break;
		}
		current = current->next;
		linenr--;
	}
	current_line = current;
}

void
edit_print_current()
{
	if (current_line == 0x0 || current_line->this == 0x0)
		return;

	printf("%s", current_line->this);
}

void
clean_up()
{
	struct line *last = first_line;
	struct line *current = last;

	while (current) {
		if (current->this != 0x0)
			free(current->this);
		last = current;
		current = last->next;
		free(last);
	}
}
