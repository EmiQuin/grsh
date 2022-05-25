#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char **path;
int path_num;
char error_message[30] = "An error has occurred\n";

char **tokenize_c(char *str){

	char *delim = " \t\n";
	char *ptr;
	char *token = strtok_r(str, delim, &ptr);
	char **myargv = malloc(sizeof(char *));
	int i = 0;
	while (token != NULL){
		myargv[i] = token;
		i++;
		myargv = realloc(myargv, sizeof(char *) * (i + 1));
		token = strtok_r(NULL, delim, &ptr);
	}
	myargv[i] = NULL;
	return myargv;
}

char ***tokenize_l(char *str){

	char *delim = "&";
	char *ptr;
	char *token = strtok_r(str, delim, &ptr);
	char ***myargv = malloc(sizeof(char **));
	int i = 0;
	while (token != NULL){
		myargv[i] = tokenize_c(token);
		i++;
		myargv = realloc(myargv, sizeof(char **) * (i + 1));
		token = strtok_r(NULL, delim, &ptr);
	}
	myargv[i] = NULL;
	return myargv;
}

int runcom(char **myargv){

	if (strcmp(myargv[0], "exit") == 0){
		return 1;
	}
	if (strcmp(myargv[0], "cd") == 0){
		if (myargv[1] != NULL && myargv[2] == NULL){
			chdir(myargv[1]);
		}
		else {
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		return 0;
	}
	if (strcmp(myargv[0], "path") == 0){
		char *new_path = myargv[1];
		free(path);
		path = malloc(sizeof(char *));
		int i = 0;
		while (new_path != NULL){
			path = realloc(path, sizeof(char *)*(i+1));
			path[i] = new_path;
			i++;
			new_path = myargv[i];
		}
		path_num = i;
		return 0;
	}
	if (myargv[0] != NULL){
		int i, c = 0;
		char *path_copy;
		for(i = 0; i < path_num; i++){
			path_copy = malloc(strlen(myargv[0])+strlen(path[i])+1);
			char *func_copy = malloc(strlen(myargv[0]));
			strcpy(path_copy, path[i]);
			strcpy(func_copy, myargv[0]);
			strcat(path_copy, "/");
			strcat(path_copy, func_copy);
			if (access(path_copy, F_OK) == 0){
				c = 1;
				break;
			}
		}
		if (c){
			int pid = fork();
			if (pid < 0){
				write(STDERR_FILENO, error_message, strlen(error_message));
				return 0;
			}
			if (pid == 0){
				execv(path_copy, myargv);
			}
			else {
				wait(NULL);
			}
		}
		else {
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
	}
	return 0;
}
int runline(FILE *input){

	char *buffer = NULL;
	size_t buffer_size = 0;
	getline(&buffer, &buffer_size, input);
	char ***command = tokenize_l(buffer);
	int i = 0, r = 0;
	while (command[i] != NULL){
		r += runcom(command[i++]);
	}
	if (feof(input)){
		return 1;
	}
	return r;
}

int main(int argc, char *argv[]){

	path = malloc(sizeof(char *));
	path[0] = "/bin";
	path_num = 1;
	int signal = 0;
	if (argc == 1){
		while(signal == 0){
			printf("grsh> ");
			signal = runline(stdin);
		}
	}
	else if (argc == 2){
		char *file_name = argv[1];
		FILE *file = fopen(file_name, "r");
		if (file == NULL){
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
		while(signal == 0){
			signal = runline(file);
		}
		fclose(file);
	}
	else {
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	exit(0);
}
