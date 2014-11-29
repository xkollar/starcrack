/*
 *   Copyright (C) 2014 by Matej Kollar
 *   Copyright (C) 2007 by David Zoltan Kedves kedazo@gmail.com
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "starcrack.h"

char* ABC = (char*) &default_ABC;
int ABCLEN;

char password[PWD_LEN+1] = {'\0', '\0'}; //this contains the actual password
char password_good[PWD_LEN+1] = {'\0', '\0'};  //this changed only once, when we found the good passord
unsigned int curr_len = 1; //current password length
long counter = 0;	//this couning probed passwords
xmlMutexPtr pwdMutex;	//mutex for password char array
char filename[255];	//the archive file name
char statname[259];	//status xml file name filename + ".xml"
xmlDocPtr status;
int finished = 0;
xmlMutexPtr finishedMutex;
char finalcmd[300] = {'\0', '\0'}; //this depending on arhive file type, it's a command to test file with password

inline int checkpass(const char *pass, const char *filename) {
	pid_t cpid, w;
	int status;
	char *empty_env[] = { NULL };
	char passarg[PWD_LEN+2+1] = {'\0', '\0'};
	int devnull;
	cpid = fork();
	if (cpid == 0) {
		snprintf(passarg, PWD_LEN+2+1, "-p%s", pass);
		devnull = open("/dev/null", O_WRONLY);
		dup2(devnull, 1);
		dup2(devnull, 2);
		fclose(stdin);
		execve("/usr/bin/unrar", (char * []) { "unrar", "t", passarg, (char*) filename, NULL }, empty_env);
		perror("execve");
		exit(EXIT_FAILURE);
	}
	do {
		w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
		if (w == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	return WEXITSTATUS(status) == EXIT_SUCCESS;
}

char * getfirstpassword() {
	static char ret[2];
	ret[0] = ABC[0];
	ret[1] = '\0';
	return ret;
}

inline void savestatus() {
	xmlNodePtr root = NULL;
	xmlNodePtr node = NULL;
	xmlChar* tmp = NULL;
	if ((strlen(statname) > 0) && (status != NULL)) {
		xmlMutexLock(finishedMutex);
		root = xmlDocGetRootElement(status);
		if (root) {
			for (node = root->children; node; node = node->next) {
				if (xmlStrcmp(node->name, (const xmlChar*) "current") == 0) {
					xmlMutexLock(pwdMutex);
					tmp = xmlEncodeEntitiesReentrant(status, (const xmlChar*) &password);
					xmlMutexUnlock(pwdMutex);
					if (node->children) {
						if (password[0] == '\0') {
							xmlNodeSetContent(node->children, (const xmlChar*) getfirstpassword());
						} else {
							xmlNodeSetContent(node->children, tmp);
						}
					}
					xmlFree(tmp);
				} else if ((finished == 1) && (xmlStrcmp(node->name, (const xmlChar*) "good_password") == 0)) {
					tmp =  xmlEncodeEntitiesReentrant(status, (const xmlChar*) &password_good);
					if (node->children) {
						xmlNodeSetContent(node->children, tmp);
					}
					xmlFree(tmp);
				}
			}
		}
		xmlSaveFormatFileEnc(statname, status, "UTF-8", 1);
		xmlMutexUnlock(finishedMutex);
	}
}

inline int abcnumb(char a) {
	int i;
	for (i = 0; i<ABCLEN; i++) {
		if (ABC[i] == a) {
			return i;
		}
	}
	return 0;
}

int loadstatus() {
	xmlNodePtr root = NULL;
	xmlNodePtr node = NULL;
	xmlParserCtxtPtr parserctxt;
	int ret = 0;
	xmlChar* tmp;
	if (access(statname, R_OK | W_OK) != -1) {
		status = xmlParseFile(statname);
	}
	if (status != NULL) {
		root = xmlDocGetRootElement(status);
	} else {
		status = xmlNewDoc(NULL);
	}
	if (root != NULL) {
		parserctxt = xmlNewParserCtxt();
		for (node = root->children; node; node = node->next) {
			if (xmlStrcmp(node->name, (const xmlChar*) "abc") == 0) {
				if (node->children && (strlen((const char*) node->children->content) > 0)) {
					ABC = (char*) xmlStringDecodeEntities(parserctxt,
						node->children->content, XML_SUBSTITUTE_BOTH, 0, 0, 0);
				} else {
					ret = 1;
				}
			} else if (xmlStrcmp(node->name, (const xmlChar*) "current") == 0) {
				if (node->children && (strlen((const char*) node->children->content) > 0)) {
					tmp = xmlStringDecodeEntities(parserctxt,
						node->children->content, XML_SUBSTITUTE_BOTH, 0, 0, 0);
					strcpy(password, (char*) tmp);
					curr_len = strlen(password);
					printf("INFO: Resuming cracking from password: '%s'\n", password);
					xmlFree(tmp);
				} else {
					ret = 1;
				}
			} else if (xmlStrcmp(node->name, (const xmlChar*) "good_password") == 0) {
				if (node->children && (strlen((const char*) node->children->content) > 0)) {
					tmp = xmlStringDecodeEntities(parserctxt,
						node->children->content, XML_SUBSTITUTE_BOTH, 0, 0, 0);
					strcpy(password, (const char*) tmp);
					curr_len = strlen(password);
					xmlMutexLock(finishedMutex);
					finished = 1;
					xmlMutexUnlock(finishedMutex);
					strcpy((char*) &password_good, (char*) &password);
					printf("GOOD: This archive was succesfully cracked\n");
					printf("      The good password is: '%s'\n", password);
					xmlFree(tmp);
					ret = 0;
					exit(EXIT_SUCCESS);
				} else if (node->children == NULL) {
					// Workaround for optimizing parser
					xmlAddChild(node, xmlNewText((const xmlChar*) ""));
				}
			}
		}
		xmlFreeParserCtxt(parserctxt);
	} else {
		root = xmlNewNode(NULL, (const xmlChar*) "starcrack");
		xmlDocSetRootElement(status, root);
		node = xmlNewTextChild(root, NULL, (const xmlChar*) "abc", (const xmlChar*) ABC);
		node = xmlNewTextChild(root, NULL, (const xmlChar*) "current", (const xmlChar*) getfirstpassword());
		node = xmlNewTextChild(root, NULL, (const xmlChar*) "good_password", (const xmlChar*) "");
		savestatus();
	}
	return ret;
}

void nextpass2(char* p, unsigned int n) {
	int i;
	if (p[n] == ABC[ABCLEN-1]) {
		p[n] = ABC[0];
		if (n>0) {
			nextpass2(p, n-1);
		} else {
			for (i=curr_len; i>=0; i--) {
				p[i+1]=p[i];
			}
			p[0]=ABC[0];
			p[++curr_len]='\0';
		}
	} else {
		p[n] = ABC[abcnumb(p[n])+1];
	}
	return;
}

inline char * nextpass() {	//IMPORTANT: the returned string must be freed
	char *ok = malloc(sizeof(char)*(PWD_LEN+1));
	xmlMutexLock(pwdMutex);
	strcpy(ok, password);
	nextpass2((char*) &password, curr_len - 1);
	xmlMutexUnlock(pwdMutex);
	return ok;
}

static void * status_thread() {
	float pwds;
	const short status_sleep = 7;
	while(1) {
		sleep(status_sleep);
		xmlMutexLock(finishedMutex);
		pwds = (float) counter / (float) status_sleep;
		counter = 0;
		if (finished != 0) {
			xmlMutexUnlock(finishedMutex);
			printf("Password found, exiting...\n");
			break;
		}
		xmlMutexUnlock(finishedMutex);
		xmlMutexLock(pwdMutex);
		printf("Probing: '%s' [%.2f pwds/sec]\n", password, pwds);
		xmlMutexUnlock(pwdMutex);
		savestatus();	//FIXME: this is wrong, when probing current password(s) is(are) not finished yet, and the program is exiting
	}
	return NULL;
}

static void * crack_thread() {
	char * current;

	while (1) {
		current = nextpass();

		if (checkpass(current, filename)) {
			strcpy(password_good, current);
			xmlMutexLock(finishedMutex);
			finished = 1;
			printf("GOOD: password cracked: '%s'\n", current);
			xmlMutexUnlock(finishedMutex);
			savestatus();
		}

		xmlMutexLock(finishedMutex);
		counter++;
		free(current);
		if (finished != 0) {
			xmlMutexUnlock(finishedMutex);
			break;
		}
		xmlMutexUnlock(finishedMutex);
	}
	return NULL;
}

void crack_start(unsigned int threads) {
	pthread_t status_th;
	pthread_t *th = malloc(sizeof(pthread_t)*threads);
	printf("INFO: Starting %u threads\n", threads);
	unsigned int i;
	for (i = 0; i < threads; i++) {
		(void) pthread_create(&th[i], NULL, crack_thread, NULL);
	}
	(void) pthread_create(&status_th, NULL, status_thread, NULL);
	for (i = 0; i < threads; i++) {
		(void) pthread_join(th[i], NULL);
	}
	// Wake sleeping status thread
	raise(SIGALRM);
	(void) pthread_join(status_th, NULL);
	return;
}

void init(int argc, char **argv) {
	int i, j;
	int help = 0;
	int threads = 2;
	int archive_type = -1;
	FILE* totest;
	char test[300];
	if (argc == 1) {
		printf("USAGE: starcrack encrypted_archive.ext [--threads NUM] [--type rar|zip|7z]\n");
		printf("       For more information please run \"starcrack --help\"\n");
		help = 1;
	} else {
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--help") == 0) {
				printf("Usage:   starcrack encrypted_archive.ext [--threads NUM] [--type rar|zip|7z]\n\n");
				printf("Options: --help: show this screen.\n");
				printf("         --type: you can specify the archive program, this needed when\n");
				printf("                 the program couldn't detect the proper file type\n");
				printf("         --threads: you can specify how many threads\n");
				printf("                    will be run, maximum 12 (default: 2)\n\n");
				printf("Info:    This program supports only RAR, ZIP and 7Z encrypted archives.\n");
				printf("         StarCrack! usually detects the archive type.\n\n");
				help = 1;
				break;
			} else if (strcmp(argv[i], "--threads") == 0) {
				if ((i + 1) < argc) {
					sscanf(argv[++i], "%d", &threads);
					if (threads < 1) {
						threads = 1;
					}
					if (threads > 12) {
						printf("INFO: number of threads adjusted to 12\n");
						threads = 12;
					}
				} else {
					printf("ERROR: missing parameter for option: --threads!\n");
					help = 1;
				}
			} else if (strcmp(argv[i], "--type") == 0) {
				if ((i + 1) < argc) {
					sscanf(argv[++i], "%s", test);
					for (j = 0; TYPE[j] != NULL; j++) {
						if (strcmp(TYPE[j], test) == 0) {
							strcpy(finalcmd, CMD[j]);
							archive_type = j;
							break;
						}
					}
					if (archive_type < 0) {
						printf("WARNING: invalid parameter --type %s!\n", argv[i]);
						finalcmd[0] = '\0';
					}
				} else {
					printf("ERROR: missing parameter for option: --type!\n");
					help = 1;
				}
			} else {
				strcpy((char*)&filename, argv[i]);
			}
		}
	}
	if (help == 1) {
		return;
	}
	sprintf((char*)&statname, "%s.xml", (char*)&filename);
	if (access(filename, R_OK) == -1) {
		printf("ERROR: The specified file (%s) does not exist or \n", filename);
		printf("       you don't have sufficient permissions!\n");
		return;
	}
	if (finalcmd[0] == '\0') {
		//when we specify the file type, the programm will skip the test
		sprintf((char*)&test, CMD_DETECT, filename);
		totest = popen(test, "r");
		fscanf(totest, "%s", (char*)&test);
		pclose(totest);
		for (i = 0; MIME[i] != NULL; i++) {
			if (strcmp(MIME[i], test) == 0) {
				strcpy(finalcmd, CMD[i]);
				archive_type = i;
				break;
			}
		}
		printf("INFO: detected file type: %s\n", TYPE[archive_type]);
	} else {
		printf("INFO: the specified archive type: %s\n", TYPE[archive_type]);
	}
	if (finalcmd[0] == '\0') {
		printf("ERROR: Couldn't detect archive type\n");
		return;
	} /*else
		printf("DEBUG: the unpack command is: '%s'\n", finalcmd);*/
	printf("INFO: cracking %s, status file: %s\n", filename, statname);
	if (loadstatus() == 1) {
		printf("ERROR: The status file (%s) is corrupted!\n", statname);
		return;
	}
	ABCLEN = strlen(ABC);
	if (password[0] == '\0') {
		password[0] = ABC[0];
	}
	crack_start(threads);
}

int main(int argc, char *argv[]) {
	printf("StarCrack! 0.3 by Matej Kollar, based on\n");
	printf("RarCrack! 0.2 by David Zoltan Kedves (kedazo@gmail.com)\n\n");
	xmlInitThreads();
	pwdMutex = xmlNewMutex();
	finishedMutex = xmlNewMutex();
	init(argc, argv);
	if (ABC != (char*) &default_ABC) {
		xmlFree(ABC);
	}
	if (status) {
		xmlFreeDoc(status);
	}
	xmlFreeMutex(pwdMutex);
	xmlFreeMutex(finishedMutex);
	return EXIT_SUCCESS;
}
