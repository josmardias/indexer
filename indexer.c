#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include "linkedlist/list.h"

//=-=-=-=-=--=-=-=-=-= Paths
typedef struct {
	int length;
	char **files;
} Paths;

Paths * Paths_new() {
	Paths *p = (Paths *)malloc(sizeof(Paths));
	return p;
}

void Paths_each(Paths *p, void (*func)(char *, int)) {
	int i;

	for(i=0; i<p->length; i++) {
		func(p->files[i], i+1);
	}
}

void _Paths_print(char *path, int n) {
	printf("[%d] %s\n", n, path);
}

//debug
void Paths_print(Paths *p) {
	puts("----------");
	printf("length: %d\n", p->length);
	Paths_each(p, _Paths_print);
}

void Paths_read(Paths *p) {
	char path[200][200];
	int i;

	for(i=0; ; i++) {
		scanf("%s", path[i]);
		if(feof(stdin))
			break;
	}

	p->files = (char **)malloc(sizeof(char*)*i);
	p->length = i;

	for(i=0; i<p->length; i++) {
		int len;

		len = strlen(path[i]);
		p->files[i] = (char *)malloc(sizeof(char)*len);
		strncpy(p->files[i], path[i], len);
	}
}

//=-=-=-=-=--=-=-=-=-= Index

typedef struct {
	int fileNumber;
	int lineNumber;
} Location;

typedef struct {
	char *word;
	List *locations;
} Index;

Location * Location_new(int f, int l) {
	Location *lo = (Location *)malloc(sizeof(Location));
	lo->fileNumber = f;
	lo->lineNumber = l;
	return lo;
}

void Location_print(Location *lo){
	printf("%d %d ", lo->fileNumber, lo->lineNumber);
}

Index * Index_new(char * word) {
	Index *ind = (Index *)malloc(sizeof(Index));
	ind->word = (char *)malloc(sizeof(char)*strlen(word));
	strcpy(ind->word, word);
	ind->locations = List_new(NULL, (void (*)(void *))Location_print, NULL);

	return ind;
}

int Index_cmp(Index *i1, Index *i2) {
	int ret = strcmp(i1->word, i2->word);
	return ret;
}

bool Index_query(Index *ind, char *word) {
	return !strcmp(ind->word, word);
}

void Index_print(Index *ind) {
	printf("%s %d ", ind->word, ind->locations->length);
	List_print(ind->locations);
	printf("\n");
}

List * Index_new_list() {
	List *l = List_new((int (*)(void *, void *))Index_cmp,
		(void (*)(void *))Index_print,
		(bool (*)(void *, void*))Index_query);

	return l;
}

//=-=-=-=-=--=-=-=-=-= Hashmap

List *Hashmap[100];

void Hashmap_init() {
	memset(Hashmap, 0, sizeof(Hashmap));
}

int Hashmap_hash(char *w) {
	int hash = 0;

//int i;
/*for (i=0; i<3; i++) {
	if (w[i] == 0) break;
	hash += w[i];
}*/
	hash = w[0];

	hash = hash % 100;

	return hash;
}

void Hashmap_add(char *w, Location *lo) {
	List *l;
	int hash;

	hash = Hashmap_hash(w);
	l = Hashmap[hash];
	if (l == NULL)
		Hashmap[hash] = l = Index_new_list();

	Index *ind = List_findOne(l, w);

	if (ind == NULL) {
		ind = Index_new(w);
		List_add(l, ind);
	}

	List_add(ind->locations, lo);
}

void Hashmap_print() {
	int i, max;

	max = sizeof(Hashmap) / sizeof(List *);
	for(i=0; i<max; i++) {
		if (Hashmap[i] == NULL)
			continue;
		List_print(Hashmap[i]);
	}
}

void Hashmap_index_line(char *line, int fileNumber, int lineNumber) {
	char *word;

	word = strtok(line, " ");
	while (word != NULL)
	{
		Hashmap_add(word, Location_new(fileNumber, lineNumber));
		word = strtok(NULL, " ");
	}
}

void Hashmap_index_file(char *filename, int fileNumber) {
	FILE *f;
	char line[1000];
	int i;

	f = fopen(filename, "r");
	if(!f) {
		printf("Fail to open file %s\n", filename);
		exit(1);
	}

	for(i=0; ; i++) {
		fgets(line, 1000, f);
		if(feof(f))
			break;
		Hashmap_index_line(line, fileNumber, i+1);
	}
}

//=-=-=-=-=--=-=-=-=-= Main

void main_index() {
	Paths *paths;

	Hashmap_init();

	paths = Paths_new();
	Paths_read(paths);

	Paths_each(paths, Hashmap_index_file);
	//puts("done indexing");

	Hashmap_print();
}

void main_search(FILE *f, char *query) {
	char trash[1000];
	char word[50];

	printf("[");
		
	while(!feof(f)) {
		fscanf(f, "%s ", word);

		if(strstr(word, query) == NULL)
			fscanf(f, "%s", trash);

		int length, i, location, line;

		fscanf(f, "%d", &length);
		for(i=0;i<length;i++) {
			fscanf(f, "%d %d", &location, &line);
			printf("[%d,%d]", location, line);
		}
	}

	printf("]");
}

int main(int argc, char *argv[]) {
	if(argc == 1) {
		main_index();
		return 0;
	}

	FILE *f = fopen(argv[1], "r");
	char query[100];
	scanf("%s", query);

	main_search(f, query);
	fclose(f);


	return(0);
}
