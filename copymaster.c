#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


#include "options.h"


void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);
int BezPrepinacov();

int main(int argc, char* argv[]) {

    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    PrintCopymasterOptions(&cpm_options);

	if (cpm_options.fast && cpm_options.slow) {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...
/*	if(cpm_options.create && cpm_options.delete_opt) {
        chmod(cpm_options.infile, 0666);
        chmod(cpm_options.outfile, 0666);
    }
*/

	if (cpm_options.umask) {
		umask(S_IWGRP|S_IROTH|S_IWOTH);
	}


    if (cpm_options.fast) {
		char buf[1000000];
		int in, out;

		if ( (in = open("infile", O_RDONLY)) > 0) {
			if ((out = open("outfile", O_RDWR)) > 0) {
				int tmp = lseek(in, 0, SEEK_END);
				lseek(in, 0, SEEK_SET);
				read(in,&buf,tmp);
				write(out,&buf,tmp);
			} else {
				perror("INA CHYBA");
			}
		} else {
			perror("INA CHYBA");
		}
		close(in);
		close(out);
	}

	if (cpm_options.slow) {
		char buf[1000000] = " ";
		int in, out;

		if ( (in = open("infile", O_RDONLY)) > 0) {
			if ( (out = open("outfile", O_RDWR|O_TRUNC)) > 0) {
				int tmp = lseek(out, 0, SEEK_END);
				lseek(out, 0, SEEK_SET);
				int c = 0;
				while (c != tmp) {
					write(out, "", 1);
					c++;
				}

				while ((read(in, buf, 1)) > 0) {
					write(out, buf, 1);
				}
			} else {
				perror("INA CHYBA");
			}
		} else {
			perror("INA CHYBA");
		}
		close(in);
		close(out);
	}

	if (cpm_options.create) {
		char buf;
		int in, out;
		int tmp = atoi(argv[2]);
		if ( (tmp > 777) ) {
			FatalError('c',"ZLE PRAVA", 23);
		}

		if ( (in = open("infile", O_RDWR, 0763)) > 0) {
			if ( (out = open("outfile", O_RDWR|O_CREAT|O_EXCL, 0763)) > 0) {
				while ( (read(in,&buf,1)) > 0) {
					write(out, &buf, 1);
				}
			}
			else if ( (out = open("outfile", O_CREAT|O_EXCL, 0763)) < 0) {
				FatalError('c', "SUBOR EXISTUJE", 23);
			} else {
				FatalError('c', "INA CHYBA", 23);
			}
		}
		close(in);
		close(out);
	}

	if (cpm_options.overwrite) {
		char buf;
        int in, out;

        if ( (in = open("infile", O_RDWR)) > 0) {
            if ( (out = open("outfile", O_RDWR)) > 0) {
                while ( (read(in,&buf,1)) > 0) {
                    write(out, &buf, 1);
                }
            } else if ( (out = open("outfile", O_RDWR)) < 0) {
                FatalError('o',"SUBOR NEEXISTUJE", 24);
            } else {
               	FatalError('o',"INA CHYBA", 24);
            }
        }
        close(in);
        close(out);
	}

	if (cpm_options.append) {

		char buf[1000000];
		int in, out;

		if ( (in = open("infile", O_RDONLY)) > 0) {
			if ( (out = open("outfile", O_RDWR)) < 0) {
				FatalError('a', "SUBOR NEEXISTUJE", 22);
			}
			else if ( (out = open("outfile", O_RDWR|O_APPEND)) > 0) {
				int tmp_in = lseek(in, 0, SEEK_END);
				lseek(in, 0, SEEK_SET);
				read(in, &buf, tmp_in);
				write(out, &buf, tmp_in);
			} else {
				FatalError('a',"INA CHYBA", 22);
			}
		}

		close(in);
		close(out);
	}

	if (cpm_options.lseek) {

		char buf[1000000];
		int in, out;

		if ( (in = open("infile", O_RDWR)) > 0) {
            if ( (lseek(in, cpm_options.lseek_options.pos1, SEEK_SET)) < 0) {
				FatalError('l', "CHYBA POZICIE infile", 33);
			}
        } else {
			FatalError('1', "INA CHYBA", 33);
		}


		if ( (out = open("outfile", O_RDWR)) > 0) {
			if ( cpm_options.lseek_options.x == SEEK_SET || cpm_options.lseek_options.x == SEEK_END ||
				cpm_options.lseek_options.x == SEEK_CUR ) {
				if(cpm_options.lseek_options.pos2 < 0) {
					FatalError('l', "CHYBA POZICIE outfile", 33);
				}
			} else if (cpm_options.lseek_options.x != 'b' || cpm_options.lseek_options.x != 'e' ||
						cpm_options.lseek_options.x != 'c') {
				FatalError('l', "INA CHYBA", 33);
			}
			lseek(in, cpm_options.lseek_options.pos1, SEEK_SET);
			read(in, &buf, cpm_options.lseek_options.num);
			lseek(out, cpm_options.lseek_options.pos2, cpm_options.lseek_options.x);
			write(out, &buf, cpm_options.lseek_options.num);
		} else {
			FatalError('1', "INA CHYBA", 33);
		}

		close(in);
		close(out);
	}
/*
	if(cpm_options.directory) {

		FILE *fp;
		DIR *dir;
		struct dirent *ent;
		struct stat info;
		char buf[10];

		dir = opendir(cpm_options.infile);
		chdir(cpm_options.infile);

		fp = fopen(cpm_options.outfile, "w+");
		ent = readdir(dir);
		stat(ent, &info);
		read(info.st_mode, &buf, 9);

		for(int i = 0; i < 10; i++) {
			fprintf(fp, "%c", buf[i]);
		}

	}

*/


	if(cpm_options.inode) {
		int in, out;
		char buf[1000000];
		struct stat info_in;

		stat(cpm_options.infile, &info_in);

		if (info_in.st_ino == cpm_options.inode_number) {
			if ( (in = open(cpm_options.infile, O_RDWR, 0777)) > 0) {
				out = open(cpm_options.outfile, O_RDWR|O_CREAT|O_TRUNC, 0777);
				int tmp = lseek(in, 0, SEEK_END);
				lseek(in, 0, SEEK_SET);
				read(in, &buf, tmp);
				write(out, &buf, tmp);
			} else if ( (in = open(cpm_options.infile, O_RDWR, 0777)) < 0) {
				FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);
			}
		} else if (info_in.st_ino != cpm_options.inode_number) {
			FatalError('i', "ZLY INODE", 27);
		} else {
			FatalError('i', "INA CHYBA", 27);
		}
	}

	if(cpm_options.delete_opt) {

		int in, out;
		char buf[1000000];

		if ( (in = open(cpm_options.infile, O_RDWR, 0777)) > 0) {
			if ( (out = open(cpm_options.outfile, O_RDWR|O_CREAT|O_TRUNC, 0777)) > 0) {
				int tmp = lseek(in, 0, SEEK_END);
                lseek(in, 0, SEEK_SET);
                read(in,&buf,tmp);
                write(out,&buf,tmp);
				if (unlink(cpm_options.infile)) {
					close(out);
				} else if (!unlink(cpm_options.infile)) {
					close(in);
					close(out);
					FatalError('d', "SUBOR NEBOL ZMAZANY", 26);
				}
			} else if ( (out = open(cpm_options.outfile, O_RDWR|O_CREAT|O_TRUNC, 0777)) < 0) {
				FatalError('d', "INA CHYBA", 26);
			}
		} else {
			FatalError('d', "INA CHYBA", 26);
		}

	}


	if (cpm_options.chmod) {
		int in, out;
		char buf[1000000];

		if ( (in = open(cpm_options.infile, O_RDONLY)) > 0) {
			if ( (out = open(cpm_options.outfile, O_RDWR|O_CREAT|O_TRUNC, 0777)) > 0) {
				int tmp = lseek(in, 0, SEEK_END);
				lseek(in, 0, SEEK_SET);
				read(in, &buf, tmp);
				write(out, &buf, tmp);
				if (cpm_options.chmod_mode <= 0777) {
					chmod(cpm_options.outfile, cpm_options.chmod_mode);
				} else {
					FatalError('m', "ZLE PRAVA", 34);
				}
			} else {
				FatalError('m', "INA CHYBA", 34);
			}
		} else {
			FatalError('m', "INA CHYBA", 34);
		}
	}

	if(!cpm_options.fast && !cpm_options.slow && !cpm_options.create && !cpm_options.overwrite &&
	!cpm_options.append && !cpm_options.lseek && !cpm_options.directory && !cpm_options.delete_opt && !cpm_options.chmod &&
	!cpm_options.inode && !cpm_options.umask && !cpm_options.link && !cpm_options.truncate && !cpm_options.sparse) {
		char buf[1000000];

		int in, out;
		struct stat info_in;
		struct stat info_out;
		stat(cpm_options.outfile, &info_out);
		stat(cpm_options.infile, &info_in);

		if ( (in = open(cpm_options.infile, O_RDONLY, info_in.st_mode)) > 0) {
			if ( (out = open(cpm_options.outfile, O_RDWR|O_CREAT|O_TRUNC, 0755)) > 0) {
                while ( (read(in, &buf, 1)) > 0) {
					write(out, &buf, 1);
                }
			} else if ( (out = open(cpm_options.outfile, O_RDWR|O_CREAT|O_TRUNC, info_out.st_mode)) < 0) {
				FatalError('B',"INA CHYBA", 21);
			}
		} else if ( (in = open("infile", O_RDONLY)) < 0) {
			FatalError('B',"SUBOR NEEXISTUJE",21);
		} else {
			FatalError('B',"INA CHYBA", 21);
		}
	}
	return 0;
}

void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d\n", c, errno); 
    fprintf(stderr, "%c:%s\n", c, strerror(errno));
    fprintf(stderr, "%c:%s\n", c, msg);
    exit(exit_status);
}

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;

    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);

    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);

    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);

    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);

    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }


    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}


