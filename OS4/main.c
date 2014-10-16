#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

struct metadata {
	char revision_number[20];
	int corresponding_data_size;
	char date[9];
	char tag;
	char comment[100];
};

char **get_command ( int * );
char **pathname_parser( char *pathname );
void check_in( char *, char *, int, char*, char* );
void check_out( char*, char *, int, char* );
int difference( char *, char* );
int patch( char* );

void tag( char *full_pathname,  char* filename ) {
	FILE *pfile, *temporary;
	int c, n, counter = 0, counter1;;
	struct metadata temp;

	if ( ( pfile = fopen( full_pathname, "rb+" ) ) == NULL ) {
		perror( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	if ( ( temporary = fopen( "temporary.tmp", "wb+" ) ) == NULL ) {
		perror( "Error in temporary file opening" );
		exit( EXIT_FAILURE );
	}
	while( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
		if ( fseek( pfile, temp.corresponding_data_size, SEEK_CUR ) != 0 ) {
			perror( "Error in file manipulation" );
			exit( EXIT_FAILURE );
		}
		counter++;
	}
	fclose( pfile );
	if ( ( pfile = fopen( full_pathname, "rb+" ) ) == NULL ) {
		perror( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	while( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
		if ( counter > 1 ) {
			temp.tag = 0;
			fwrite( &temp, sizeof( struct metadata ), 1, temporary );
		}
		else {
			temp.tag = 1;
			fwrite( &temp, sizeof( struct metadata ), 1, temporary );
		} 
		counter--;
		counter1 = 0;
		while ( counter1 < temp.corresponding_data_size  ) {
			c = fgetc( pfile );
			counter1++;
			fputc( c, temporary );
		}
	}
	fclose( pfile );
	fclose( temporary );
	if ( ( pfile = fopen( full_pathname, "w+" ) ) == NULL ) {
		perror ( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	if ( ( temporary = fopen( "temporary.tmp", "r" ) ) ==  NULL ) {
		perror (  "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	while ( ( ( c = fgetc( temporary ) ) != EOF ) ) {
			counter1++;
			fputc( c, pfile );
	}
	fclose( pfile );
	fclose( temporary );
	if ( remove( "temporary.tmp" ) != 0 ) {
		perror( "Error in temporary file removal" );
		exit( EXIT_FAILURE );
	}
}

void changes( char* full_pathname, char* filename, int mode, char* parameter ) {
	char  *token, parameter1[30], parameter2[30], buffer[256], ranged = 0, parameter1_found=0, parameter2_found=0;
	FILE *pfile, *temporary1, *temporary2, *changes_file, *temp_copy;
	int  n, counter1 = 0, counter = 0, c, i;
	struct metadata temp;

	if ( ( pfile = fopen( full_pathname, "r" ) ) == NULL ) {
		perror( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	if ( mode != 2 ) {
		for ( i = 0 ; i < strlen( parameter ) ; i++ ) 
			if ( parameter[i] == '-' ) 
				ranged = 1;
		if ( ranged ) {
			strcpy( buffer, parameter );
			token = strtok( buffer, "-" );
			strcpy( parameter1, token);
			token = strtok( NULL, "-" );
			strcpy( parameter2, token);
			if ( strcmp( parameter1, parameter2 ) == 0) {
				printf( "Parameters are equal.\n" );
				return;
			}
		}
	}
	if ( ( temporary1 = fopen( "temporary1.tmp", "w+" ) ) == NULL ) {
		perror ( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	if ( ( temporary2 = fopen( "temporary2.tmp", "w+" ) ) == NULL ) {
		perror ( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	// Changes by revision number
	if ( mode == 0 ) {
		// Not ranged changes by revision number
		if ( ranged == 0 ) {
			if ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					fputc( c, temporary1 );
					counter1++;
				}
			}
			fclose( temporary1 );
			if ( fseek( pfile, 0, SEEK_SET ) != 0 ) {
				perror( "Error in file manipulation" );
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				exit( EXIT_FAILURE );
			}
			while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				counter1 = 0;
				if (  strcmp( temp.revision_number, parameter ) <= 0 ){
					if ( strcmp( temp.revision_number, parameter ) == 0 )
						parameter1_found = 1;
					
					if ( ( temp_copy = fopen( "delta", "w+") ) == NULL) {
						perror( "Error generating temporary stream!" );
						exit( EXIT_FAILURE );
					}
					if ( counter == 0 ) {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temporary2 );
						}
						fclose( temporary2 );
					}
					else {
						while ( counter1 < temp.corresponding_data_size) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temp_copy );
						}
						fclose( temp_copy );
						patch( "temporary2.tmp" );
					}
					counter++;
				}
				else {
					if ( !parameter1_found  ) {
						if ( remove( "temporary1.tmp" ) != 0 ) {
							perror( "Error in temporary file removal" );
							exit( EXIT_FAILURE );
						}
						if ( remove( "temporary2.tmp" ) != 0 ) {
							perror( "Error in temporary file removal" );
							exit( EXIT_FAILURE );
						}
						if ( counter > 1) {
							if ( remove( "delta" ) != 0 ) {
								perror( "Error in temporary file removal" );
								exit( EXIT_FAILURE );
							}
						}
						printf( "The revision limit you entered was not found.\n" );
						return;
					}
					break;
				}
			}
		}
		// Ranged changes by revision number
		else {
			while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				counter1 = 0;
				if (  strcmp( temp.revision_number, parameter1 ) <= 0 ) {
					if ( strcmp( temp.revision_number, parameter1 ) == 0 )
						parameter1_found = 1;			
					if ( ( temp_copy = fopen( "delta", "w+") ) == NULL) {
						perror( "Error generating temporary stream!" );
						exit( EXIT_FAILURE );
					}
					if ( counter == 0 ) {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temporary1 );
						}
						fclose( temporary1 );
					}
					else {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temp_copy );
						}
						fclose( temp_copy ); 
						patch( "temporary1.tmp" );
					}
					counter++;
				}
				else 
					break;
				
			}
			if ( !parameter1_found ) {
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
				printf( "The lower revision limit was not found.\n" );
				return;
			}
			if ( fseek( pfile, 0, SEEK_SET ) != 0 ) {
				perror( "Error in file manipulation.\n" );
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
			}	     
			counter = 0;
			while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				counter1 = 0;
				if (  strcmp( temp.revision_number, parameter2 ) <= 0 ) {
					if ( strcmp( temp.revision_number, parameter2 ) == 0 )
						parameter2_found = 1;
					if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
						perror( "Error generating temporary stream!" );
						exit( EXIT_FAILURE );
					}
					if ( counter == 0 ) {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temporary2 );
						}
						fclose( temporary2 );
					}
					else {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temp_copy );
						}
						fclose( temp_copy );
						patch( "temporary2.tmp" );
					}
					counter++;
				}
				else 
				
					break;
				
			}
			if ( !parameter2_found ) {
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
				printf( "The upper revision limit was not found.\n" );
				return;
			}
			
		}
	}
	// Changes by date
	else if ( mode == 1 ) {
		// Not ranged changes by date
		if ( ranged == 0 ) {
			if ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					fputc( c, temporary1 );
					counter1++;
				}
			}
			fclose( temporary1 );
			if ( fseek( pfile, 0, SEEK_SET ) != 0 ) {
				perror( "Error in file manipulation" );
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				exit( EXIT_FAILURE );
			}
			while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				
				if (  strcmp( temp.date, parameter ) <= 0 ){
					if ( strcmp( temp.date, parameter ) == 0 )
						parameter1_found = 1;
					counter1 = 0;
					if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
						perror( "Error generating temporary stream!" );
						exit( EXIT_FAILURE );
					}
					if ( counter == 0 ) {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temporary2 );
						}
						fclose( temporary2 );
					}
					else {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temp_copy );
						}
						fclose( temp_copy );
						patch( "temporary2.tmp" );
					}
					counter++;
				}
				else 
					break;
				
			}
			if ( ( !parameter1_found ) && ( counter == 0 ) ) {
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
				printf( "The upper date limit is invalid. Older than the first revision date.\n" );
				return;
			}
			if ( !parameter1_found && counter != 0  ) 
				printf( "The upper date limit was not found. The revision of the closest was picked.\n" );
		}
		// Ranged changes by date
		else {
			while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				if (  strcmp( temp.date, parameter1 ) <= 0 ) {
					if ( strcmp( temp.date, parameter1 ) == 0 )
						parameter1_found = 1;
					counter1 = 0;
					if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
						perror( "Error generating temporary stream!" );
						exit( EXIT_FAILURE );
					}
					if ( counter == 0 ) {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temporary1 );
						}
						fclose( temporary1 );
					}
					else {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temp_copy );
						}
						fclose( temp_copy );
						patch( "temporary1.tmp" );
					}
					counter++;
				}
				else 
					break;
			}
			if ( ( !parameter1_found ) && ( counter == 0 ) ) {
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
				printf( "The lower date limit is invalid. Older than the first revision date.\n" );
				return;
			}
			else if ( ( !parameter1_found ) && ( counter != 0 ) ) 
				printf( "The lower date limit was not found. The closest date revision was picked.\n" );
			if ( fseek( pfile, 0, SEEK_SET ) != 0 ) {
				perror( "Error in file manipulation.\n" );
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
			}	     
			counter = 0;
			while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				if (  strcmp( temp.date, parameter2 ) <= 0 ){
					if ( strcmp( temp.date, parameter2 ) == 0 )
						parameter2_found = 1;
					counter1 = 0;
					if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
						perror( "Error generating temporary stream!" );
						exit( EXIT_FAILURE );
					}
					if ( counter == 0 ) {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temporary2 );
						}
						fclose( temporary2 );
					}
					else {
						while ( counter1 < temp.corresponding_data_size ) {
							c = fgetc( pfile );
							counter1++;
							fputc( c, temp_copy );
						}
						fclose( temp_copy );
						patch( "temporary2.tmp" );
					}
					counter++;
				}
				else 
					break;
				
			}
			if ( !parameter2_found && counter == 0 ) {
				if ( remove( "temporary1.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( remove( "temporary2.tmp" ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				if ( counter > 1) {
					if ( remove( "delta" ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
				}
				printf( "The upper date limit is invalid. Older than the first revision date.\n" );
				return;
			}
			else if ( ( !parameter2_found ) && ( counter != 0 ) ) 
				printf( "The upper date limit was not found. The closest date revision was picked.\n" );
		}
	}
	else if ( mode == 2 ) {
		if ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					fputc( c, temporary1 );
					counter1++;
				}
		}
		fclose( temporary1 );
		if ( fseek( pfile, 0, SEEK_SET ) != 0 ) {
			perror( "Error in file manipulation" );
			if ( remove( "temporary1.tmp" ) != 0 ) {
				perror( "Error in temporary file removal" );
				exit( EXIT_FAILURE );
			}
			if ( remove( "temporary2.tmp" ) != 0 ) {
				perror( "Error in temporary file removal" );
				exit( EXIT_FAILURE );
			}
			exit( EXIT_FAILURE );
		}
		while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
				counter1 = 0;
				if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
					perror( "Error generating temporary stream!" );
					exit( EXIT_FAILURE );
				}
				if ( counter == 0 ) {
					while ( counter1 < temp.corresponding_data_size ) {
						c = fgetc( pfile );
						counter1++;
						fputc( c, temporary2 );
					}
					fclose( temporary2 );
				}
				else {
					while ( counter1 < temp.corresponding_data_size) {
						c = fgetc( pfile );
						counter1++;
						fputc( c, temp_copy );
					}
					fclose( temp_copy );
					patch( "temporary2.tmp" );
				}
				counter++;
		}
	
	}
	fclose ( pfile );	
	difference( "temporary1.tmp", "temporary2.tmp" );
	if ( ( changes_file = fopen( "delta", "r" ) ) == NULL ) {
		perror( "Error in file opening" );
		if ( remove( "temporary1.tmp" ) != 0 ) {
				perror( "Error in temporary file removal" );
				exit( EXIT_FAILURE );
		}
		if ( remove( "temporary2.tmp" ) != 0 ) {
			perror( "Error in temporary file removal" );
			exit( EXIT_FAILURE );
		}
		exit( EXIT_FAILURE );
	}
	if ( ( mode == 0 ) && ( ranged == 0 ) )
		printf( "Changes between the first revision and revision: %s\n", parameter );
	else if ( ( mode == 0 ) && ( ranged == 1 ) )
		printf( "Changes between revision: %s and revision: %s\n", parameter1, parameter2 );
	else if ( ( mode == 1 ) && ( ranged == 0 ) )
		printf( "Changes between the first revision and revision and the last revision of date: %s\n", parameter );
	else if ( ( mode == 1 ) && ( ranged == 1 ) ) 
		printf( "Changes between the last revision of date: %s and the last revision of date: %s\n", parameter1, parameter2 );
	else if ( mode == 2 )
		printf( "Changes between the first revision and the last revision\n" );
	printf( "###################################################################################\n" );
	while ( ( c = fgetc( changes_file ) ) != EOF ) {
		putchar(c);
		
	}
	fclose( changes_file );
	printf( "###################################################################################\n" );
	if ( remove( "temporary1.tmp" ) != 0 ) {
		perror( "Error in temporary file removal" );
		exit( EXIT_FAILURE );
	}
	if ( remove( "temporary2.tmp" ) != 0 ) {
		perror( "Error in temporary file removal" );
		exit( EXIT_FAILURE );
	}
	if ( remove( "delta" ) != 0 ) {
		perror( "Error in temporary file removal" );
		exit( EXIT_FAILURE );
	}
		
}

void fill_metadata_field( struct metadata *temp, char* revision_number, int size, char* comment ){
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[9];
	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime( buffer, 9, "%Y%m%d", timeinfo );
	strcpy( temp->date, buffer );
	strcpy( temp->revision_number, revision_number );
	temp->corresponding_data_size = size;
	temp->tag = 0;
	strcpy( temp->comment, comment );
}

char* get_filename( char* pathname ) {
	int i, start, limit;
        char *filename, problematic = 0;
	
	if ( ( filename = malloc( 256*sizeof(char) ) ) == NULL ) {
		perror( "Error in memory allocation" );
		exit( EXIT_FAILURE );
	}
	limit = strlen( pathname ) - 1;
	for ( i = 0 ; i < limit ; i++ ) 
		if ( ( pathname[i] == '/' )  && ( i <  (limit - 1) ) ) 
			start = i;
	for ( i = 0 ; i < limit-start ; i++ ) {
		if ( pathname[start + i + 1] != '/' ) 
			filename[i] = pathname[ start + i + 1 ];
		else 
			problematic = 1;
	}
	if ( !problematic )
		filename[i] = '\0';
	else 
		filename[i-1] = '\0';
	return filename;
}

char* set_comment() {
	int i,c;
	char *comment;
	if ( ( comment = malloc( 100*sizeof(char) ) ) == NULL ) {
		perror ( "Error in memory allocation.\n" );
		exit ( EXIT_FAILURE );
	}
	printf( "Please type a comment up to 99 chars.\n" );
	fflush(stdout);
	i = 0;
	while ( ( ( c = getchar() ) != '\n' ) && ( i < 99 ) ) {
		comment[i] = c;
		i++;
	}
	if ( i == 99 )
		printf ( "Comment space ended. Your comment has been saved.\n" );
	comment[i] = '\0';
	return comment;
}

void directory_hierarchy( DIR *dir, char *full_path_name, char * action, int mode, char* parameter, char* comment ) {
	struct stat info, info1;
	struct dirent *entry;
	DIR *subdir;
	char file_exists,  entry_full_path_name[256];
	
	while ( ( entry = readdir(dir) ) != NULL ) {
		printf("  %s \n", entry->d_name );
		
		if ( ( strcmp( entry->d_name, "." ) == 0 ) || ( strcmp( entry->d_name, ".." ) == 0 ) )
			continue;
		strcpy( entry_full_path_name, full_path_name );
		if ( entry_full_path_name[ strlen( entry_full_path_name ) - 1 ] != '/' )
			strcat( entry_full_path_name, "/" );
		strcat( entry_full_path_name, entry->d_name );
		lstat( entry_full_path_name, &info );
		
		if ( S_ISDIR( info.st_mode ) ) {
			if ( ( subdir = opendir( entry_full_path_name ) ) == NULL ) {                             //Check if Repository Directory exists        
				perror( "Error in directory opening.\n" );
				exit( EXIT_FAILURE );
			}
			file_exists = lstat( entry->d_name, &info1 );
			if ( file_exists != 0 || !S_ISDIR( info1.st_mode ) ) { 
				if ( mkdir( entry->d_name, info.st_mode ) != 0 ) {
					perror( "Error in mkdir" );
					exit( EXIT_FAILURE );
				}
			}
			else { 
				if ( chmod( entry->d_name, info.st_mode ) != 0 ) {
					perror( "Error in chmod" );
					exit( EXIT_FAILURE );
				}
			}
			if ( chdir( entry->d_name ) != 0 ) {
				perror( "Error in current working directory change. chdir()" );
				exit( EXIT_FAILURE );
			}
			directory_hierarchy( subdir, entry_full_path_name, action, mode, parameter, comment );
			
		}
		else {
			if ( strcmp( action, "checkin" ) == 0 )
				check_in( entry_full_path_name, entry->d_name, mode, parameter, comment );
			else if ( strcmp( action, "checkout" ) == 0 )
				check_out( entry_full_path_name, entry->d_name, mode, parameter );
			else if ( strcmp( action, "tag" ) == 0 )
				tag( entry_full_path_name, entry->d_name );
			else if ( strcmp( action, "changes" ) == 0 )
				changes( entry_full_path_name, entry->d_name, mode, parameter );
			
		}
	}
	if ( chdir( ".." ) != 0 ) {
		perror( "Error in current working directory change, chdir()" );
		exit ( EXIT_FAILURE );
	}
	closedir( dir );
}		

int difference( char *filename1, char *filename2 ) {

	int fd[2];
	pid_t cpid;
	int ffd, n;
	char buf[10];
	
	if ( pipe(fd) == -1 ) {
		perror( "Error in diff, pipe");
		return -1;
	}
	
	if ( ( cpid = fork() ) == -1 ) {
		perror( "Error in diff, fork" );
		return -1;
	}
	
	if( cpid == 0 ) {
		close(fd[0]);
		close(0);
		dup2( fd[1], 1 ); /*Child's stdout is set to write end of pipe*/
		execlp( "diff", "diff", filename1, filename2, NULL );
		perror( "Error in exec" );
		return -1;
	}
	else {
		close(fd[1]);
		if ( ( ffd = open( "delta", O_WRONLY | O_CREAT, 0666 ) ) == -1 ) {
			perror( "Error in diff, input file opening" );
			return -1;
		}
		
		/*Parent reads from read end of pipe*/
		while( ( n = read(fd[0], buf, sizeof(buf) ) ) > 0 ) {
			if( write(ffd, buf, n) < 0 ) { 
				perror( "Error in diff, write to output file");
				return -1;
			}
		}
		close(fd[0]);
	

	}  
	return 0;
			
}

int patch( char *filename ) {
	
	int fd[2], status;
	pid_t cpid, pid;
	int ffd, n;
	char buf[32];
	
	if( pipe(fd) == -1) {
		perror( "Error in patch,pipe" );
		return -1;
	}
	
	if( (cpid = fork()) == -1 ) {
		perror( "Error in patch, fork" );
		return -1;
	}
	
	if( cpid == 0 ) {
		close(fd[1]);
		close(1); /*Updated line*/
		dup2(fd[0], 0); /*Child's stdin is set to read end of pipe*/
		execlp( "patch", "patch", filename, NULL );
		perror( "Error in exec" );
		return -1;
	}
	else {
		close(fd[0]);
		if( ( ffd = open( "delta", O_RDONLY ) ) == -1 ) {
			perror( "Error in patch, file opening" );
			return -1;
		}
		
		/*Parent writes to write end of pipe*/
		while( ( n = read( ffd, buf, sizeof(buf) ) ) > 0 ) {
			if( write(fd[1], buf, n) < 0 ) { 
				perror( "Error in patch, write to output file" );
				return -1;
			}		
		}
		close(fd[1]);
		if ( ( pid = wait( &status ) ) == -1 ) { 
			perror( "Error in wait" );
			return -1;
		}
		return 0;
	}	
}



void pre_check_out( char* pathname, char* action, int mode, char* parameter ) {
	DIR *dir;
	char full_pathname[256], file_exists = -1, buffer[256], *filename;
	struct stat info, info1;
	
	chdir( "./RepDir" );
	getcwd( buffer, 256 );
	

	strcpy( full_pathname, buffer );
	if ( pathname[strlen( pathname ) - 1] != '/' )
		strcat( full_pathname, "/" );
	strcat( full_pathname, pathname );
	chdir( "../WorkDir" );
	if ( ( file_exists = lstat( full_pathname, &info ) ) == 0 ) {
		printf( "and is a directory.\n" );
		if ( ( dir = opendir( full_pathname ) )  == NULL ) {
			perror( "opendir() error" );
			exit( EXIT_FAILURE );
		}
		filename = get_filename( full_pathname );
		if ( ( file_exists = lstat( filename, &info1 ) ) != 0 ) {
			if ( mkdir( filename, info.st_mode ) != 0 ) {
				perror( "Error in mkdir" );
				exit( EXIT_FAILURE );
			}
		}
		else {
			if ( chmod( filename, info.st_mode ) != 0 ) {
				perror( "Error in chmod" );
				exit( EXIT_FAILURE );
			}
		}
		chdir( filename );
			directory_hierarchy( dir, full_pathname, action, mode, parameter, (char*)NULL );
	}		
	else {
		strcat( full_pathname, ".u" );
		if ( ( file_exists = lstat( full_pathname, &info1 ) ) == 0 ) {
			filename = get_filename( full_pathname );
			if ( strcmp( action, "checkout" ) == 0 )			
				check_out( full_pathname, filename, mode, parameter );
			else if ( strcmp( action, "tag" ) == 0 )
				tag( full_pathname, filename );
			else if ( strcmp( action, "changes" ) == 0 )
				changes( full_pathname, filename, mode, parameter );
		}
		else 
			printf( "File does not exist.\n" );
	}
	chdir( ".." );

}

void pre_check_in( char* pathname, int mode, char* parameter ) {
	DIR *dir;
	char *comment,full_pathname[256], file_exists = -1, buffer[256], *filename;
	struct stat info, info1;
	
	if ( pathname[0] == '/' ) 
		strcpy( full_pathname, pathname );
	else {
		getcwd( buffer, 256 );
		strcpy( full_pathname, buffer );
		strcat( full_pathname, "/" );
		strcat( full_pathname, pathname );
	}
	comment = set_comment();
	chdir( "./RepDir" );
	if ( ( file_exists = stat( full_pathname, &info ) ) == 0 ) {
		printf( "File exists " );

			if ( S_ISDIR( info.st_mode ) ) {
				printf( "and is a directory.\n" );
				if ( ( dir = opendir( full_pathname ) )  == NULL ) {
					perror( "opendir() error" );
					exit( EXIT_FAILURE );
				}
			
				filename = get_filename( full_pathname );
				printf ( "File name: %s\n", filename );
				if ( ( file_exists = stat( filename, &info1 ) ) != 0 ) {
					if ( mkdir( filename, info.st_mode ) != 0 ) {
						perror( "Error in mkdir" );
						exit( EXIT_FAILURE );
					}
				}
				else {
					if ( chmod( filename, info.st_mode ) != 0 ) {
						perror( "Error in chmod" );
						exit( EXIT_FAILURE );
					}
				}
				chdir( filename );
				directory_hierarchy( dir, full_pathname, "checkin", mode, parameter, comment );
				free(filename);
			}	
			else {
				printf( "and is a file.\n" );
				filename = get_filename( full_pathname );
				check_in( full_pathname, filename, mode, parameter, comment );
				free( filename );
			}
	}
	else
		printf( "File does not exist.\n" );
	chdir( ".." );
	getcwd( buffer, 256 );

}

void check_in( char *full_path_name, char *filename, int mode, char *parameter, char* comment ) {
	FILE *pfile, *inputfile, *temp_copy, *temp_patched, *delta;
	int c, file_exists = -1, n, counter = 0, counter1, j;
	char rep_filename[256], buffer[20];
	struct stat info;
	struct metadata temp, temp_write;

	strcpy( rep_filename, filename );
	strcat( rep_filename, ".u");
						
	// File exists in Repository
	if ( ( file_exists = lstat ( rep_filename, &info ) ) == 0 ) {
		printf( ".u file exists in repository.\n" );
		if ( ( pfile = fopen( rep_filename, "rb+" ) ) == NULL ) {
			perror( "Error in file opening" );
			exit( EXIT_FAILURE );
		}
		if ( ( temp_patched = fopen( "temp_patched.tmp", "w+" ) ) == NULL ) {
			perror( "Error generating temporary stream" );
			exit( EXIT_FAILURE );
		}
		while( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
			counter1 = 0;
			if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
				perror( "Error generating temporary stream!" );
				exit( EXIT_FAILURE );
			}
			if ( counter == 0 ) {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					counter1++;
					fputc( c, temp_patched );
				}
				fclose( temp_patched );
			}
			else {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					counter1++;
					fputc( c, temp_copy );
				}
				fclose( temp_copy );
				patch( "temp_patched.tmp" );
			}
			counter++;
		}
		difference( "temp_patched.tmp", full_path_name );
		if ( ( file_exists = stat( "delta", &info ) ) == 0 ) {
			if ( info.st_size == 0 ) {
				printf( "No differences between chosen and repository entry.\n" );
				return;
			}
		}
		else {
			printf( "Error difference file not created.\n" );
		}

		if ( mode == 0 ) {
			j = atoi( temp.revision_number );
			j++;
			sprintf( buffer, "%d", j );
			fill_metadata_field( &temp_write, buffer, info.st_size, comment );
		}
		else if ( mode == 1 )
			fill_metadata_field( &temp_write, parameter, info.st_size, comment );
		fwrite( &temp_write, sizeof( struct metadata ), 1, pfile );
		if ( ( delta = fopen( "delta", "rb" ) ) == NULL ) {
			perror( "Error in temporary file opening" );
			exit( EXIT_FAILURE );
		}
		fseek( delta, 0, SEEK_SET );
		while ( ( c = fgetc( delta ) ) != EOF ) 
			fputc( c, pfile );
		fclose( delta );
		fclose( pfile );
		if ( remove( "temp_patched.tmp" ) ) {
			perror( "Error in temporary file temp_pathced.tmp removal" );
			exit( EXIT_FAILURE );
		}
		if ( remove( "delta" ) ) {
			perror( "Error in temporary file delta removal" );
			exit( EXIT_FAILURE );
		}
	}
	//File does not exist in Repository
	else {
		printf( "File does not exist in Repository.\n" );
		stat( full_path_name, &info );
	
		if ( ( pfile = fopen( rep_filename, "wb+" ) ) == NULL ) {
			perror( "Error in file opening.\n" );
			exit( EXIT_FAILURE );
		}
		printf( "full path name: %s\n", full_path_name );
		
		if ( mode == 0 )
			fill_metadata_field( &temp_write, "10000", info.st_size, comment );
		else if ( mode == 1 )
			fill_metadata_field( &temp_write, parameter, info.st_size, comment );
		fwrite( &temp_write, sizeof( struct metadata ), 1, pfile );
		if ( ( inputfile = fopen( full_path_name, "rb" ) ) == NULL ) {
			perror( "Error in file opening.\n" );
			exit( EXIT_FAILURE );
		}
		while ( ( c = fgetc( inputfile ) ) != EOF ) 
			fputc( c, pfile );
		
		fclose( pfile );
		fclose( inputfile );
	}
	lstat ( full_path_name, &info );
	if ( chmod ( rep_filename, info.st_mode ) != 0 ) {
		perror( "Error in chmod" );
		exit( EXIT_FAILURE );
	}
	
}
		
void check_out( char* full_pathname, char* rep_filename, int mode, char* parameter ) {
	FILE *pfile, *outputfile, *temp_copy;
	int c, i, n, counter = 0, counter1;;
	char work_filename[256], parameter_found = 0;
	struct metadata temp;
	struct stat info;
	
	for  ( i = 0 ; i < ( strlen( rep_filename ) - 2 ) ; i++ )
		work_filename[i] = rep_filename[i];
	work_filename[i] = '\0';

	if ( ( pfile = fopen( full_pathname, "rb" ) ) == NULL ) {
		perror( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	if ( ( outputfile = fopen( work_filename, "w+" ) ) == NULL ) {
		perror ( "Error in file opening" );
		exit( EXIT_FAILURE );
	}
	//Check out latest version
	if ( mode == 0 ) {
		while( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) {
			counter1 = 0;
			if ( ( temp_copy = fopen( "delta", "wb+") ) == NULL ) {
				perror( "Error generating temporary stream!" );
				if ( remove( work_filename ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				exit( EXIT_FAILURE );
			}
			if ( counter == 0 ) {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					counter1++;
					fputc( c, outputfile );
				}
				fclose( outputfile );
			}
			else {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					counter1++;
					fputc( c, temp_copy );
				}
				fclose( temp_copy );
				patch( work_filename );
			}
			counter++;
		}
	}
	//Check out by revision
	else if ( mode == 1 ) {
		while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
			counter1 = 0;
			if ( strcmp( temp.revision_number, parameter ) <= 0 ) {
				if ( strcmp( temp.revision_number, parameter ) == 0 )
					parameter_found = 1;
				if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
					perror( "Error generating temporary stream!" );
					if ( remove( work_filename ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
					exit( EXIT_FAILURE );
				}
				if ( counter == 0 ) {
					while ( counter1 < temp.corresponding_data_size ) {
						c = fgetc( pfile );
						counter1++;
						fputc( c, outputfile );
					}
					fclose( outputfile );
				}
				else {
					while ( counter1 < temp.corresponding_data_size ) {
						c = fgetc( pfile );
						counter1++;
						fputc( c, temp_copy );
					}
					fclose( temp_copy );
					patch( work_filename );
				}
				counter++;
			}
			else {
				if ( !parameter_found ) {
					if ( remove( work_filename ) != 0 ) {
						perror( "Error in file removal" );
						exit( EXIT_FAILURE );
					}
					if ( counter > 1 ) {
						if ( remove( "delta" ) != 0 ) {
							perror( "Error in file removal" );
							exit( EXIT_FAILURE );
						}
					}
					printf( "Parameter not found.\n" );
					return;
				}
						
				
				break;
			}
		}
	}
	//Check out by date
	else if ( mode == 2 ) {
		while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
			if ( strcmp( temp.date, parameter ) <= 0 ){
				if ( strcmp( temp.date, parameter ) == 0 )
					parameter_found = 1;
			
				counter1 = 0;
				if ( ( temp_copy = fopen( "delta", "w+") ) == NULL ) {
					perror( "Error generating temporary stream!" );
					if ( remove( work_filename ) != 0 ) {
						perror( "Error in temporary file removal" );
						exit( EXIT_FAILURE );
					}
					exit( EXIT_FAILURE );
				}
				if ( counter == 0 ) {
					while ( counter1 < temp.corresponding_data_size ) {
						c = fgetc( pfile );
						counter1++;
						fputc( c, outputfile );
					}
					fclose( outputfile );
				}
				else {
					while ( counter1 < temp.corresponding_data_size ) {
						c = fgetc( pfile );
						counter1++;
						fputc( c, temp_copy );
					}
					fclose( temp_copy );
					patch( work_filename );
				}
				counter++;
			}
			else 	
				break;
			
		}
	}
	//Check out by stable version
	else if ( mode == 3 ) {
		while ( ( n = fread( &temp, 1, sizeof( struct metadata ), pfile ) ) > 0 ) { 
			if (  temp.tag == 1 )
				parameter_found = 1;
			counter1 = 0;
			if ( ( temp_copy = fopen( "delta", "wb+") ) == NULL ) {
				perror( "Error generating temporary stream!" );
				if ( remove( work_filename ) != 0 ) {
					perror( "Error in temporary file removal" );
					exit( EXIT_FAILURE );
				}
				exit( EXIT_FAILURE );
			}
			if ( counter == 0 ) {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					counter1++;
					fputc( c, outputfile );
				}
				fclose( outputfile );
			}
			else {
				while ( counter1 < temp.corresponding_data_size ) {
					c = fgetc( pfile );
					counter1++;
					fputc( c, temp_copy );
				}
				fclose( temp_copy );
				patch( work_filename );
			}
			counter++;
			if ( parameter_found )
				break;
		}
		if ( !parameter_found ) 
			printf( "Stable version not found, the last file version was checked out.\n" );
	}
	
	fclose( pfile );
	lstat ( full_pathname, &info );
	if ( chmod ( work_filename, info.st_mode ) != 0 ) {
		perror( "Error in chmod" );
		exit( EXIT_FAILURE );
	}
	remove( "delta" );
}

int main() {
	struct stat info;
	char file_exists, **s, *pch, parameter_found;
	int args;
	
	file_exists = stat( "./RepDir", &info );
	if ( file_exists != 0 || !S_ISDIR( info.st_mode ) ) {
		printf( "Repository Directory does not exist. Creating Repository Directory..\n" );
		mkdir( "./RepDir", 0777 );
	}
	file_exists = stat( "./WorkDir", &info );
	if ( file_exists != 0 || !S_ISDIR( info.st_mode ) ) {
		printf( "Working Directory does not exist. Creating Working Directory..\n" );
		mkdir( "./WorkDir", 0777 );
	}
	while(1) {
		printf("prompt> ");
		fflush(stdout);
		s = get_command( &args );
		pch = NULL;
		parameter_found = 0;
		if ( args == 0 ) {
			printf( "Command not found.\n" );
			continue;
		}
		else {	
			
			if ( strcmp( s[0], "exit" ) == 0 )
				break;
			else if ( ( strcmp( s[0], "nanorcs" ) == 0 ) && ( strcmp( s[1], "checkin" ) == 0 ) ) {
				printf( "You chose checkin.\n" );
				if ( args < 3 ) {
					printf( "Command not found.\n" );
					break;
				}
				if ( s[2][0] == '-'  && s[2][1] == 'r' ) {
					pch = strtok ( s[2], "r" );
					pch = strtok ( NULL, "r" );
					printf( "Substring: %s\n", pch );
					pre_check_in ( s[3], 1, pch );
					parameter_found = 1;
				}
				if ( !parameter_found )
					pre_check_in( s[2], 0, (char*) NULL );
			}
			else if ( ( strcmp( s[0], "nanorcs" ) == 0 ) && ( strcmp( s[1], "checkout" ) == 0 ) ) {
				printf( "You chose checkout.\n" );
				if ( args < 3 ) {
					printf( "Command not found.\n" );
					break;
				}
				if ( s[2][0] == '-' ) {
					if ( s[2][1] == 'r' ) {
						printf( "Checkout by revision number choice.\n" );
						pch = strtok ( s[2], "r" );
						pch = strtok ( NULL, "r" );
						pre_check_out( s[3], "checkout", 1, pch );
						parameter_found = 1;
					}
					else if ( s[2][1] == 'd' ) {
					
						pch = strtok( s[2], "d" );
						pch = strtok( NULL, "d" );
						pre_check_out( s[3], "checkout", 2, pch );
						parameter_found = 1;
						
					}
					else {
						printf( "One of your arguments is wrong. Try again.\n" );
						break;
					}
				}
				else if ( strcmp( s[2], "stable" ) == 0 ) {
				
					pre_check_out( s[3], "checkout", 3, (char*) NULL );
					parameter_found = 1;
				}
				if ( !parameter_found )
					pre_check_out( s[2], "checkout", 0, (char*) NULL );
			}
		
		
			else if ( ( strcmp( s[0], "nanorcs" ) == 0 ) && ( strcmp( s[1], "tag" ) == 0 ) ) {
				if ( args < 3 ) {
					printf( "Command not found.\n" );
					break;
				}
				pre_check_out( s[2], "tag", 0, (char*) NULL );
			}
			else if ( ( strcmp( s[0], "nanorcs" ) == 0 ) && ( strcmp( s[1], "changes" ) == 0 ) ) {
				
				if ( args < 3 ) {
					printf( "Command not found.\n" );
					break;
				}
				if ( s[2][0] == '-' ) {
					if ( s[2][1] == 'r' ) {
						pch = strtok( s[2], "r" );
						pch = strtok( NULL, "r" );
						parameter_found = 1;
						pre_check_out( s[3], "changes", 0, pch );
						
												
					}
					if ( s[2][1] == 'd' ) {
						printf( "Changes by date choice.\n" );
						pch = strtok( s[2], "d" );
						pch = strtok( NULL, "d" );
						parameter_found = 1;
						pre_check_out( s[3], "changes", 1, pch );
						
						
					}
				}
				if ( !parameter_found ) 
					pre_check_out( s[2], "changes", 2, (char*) NULL );
			}
			else if ( strcmp( s[0], "exit" ) == 0 )
				  break;
			else {
				printf( "Command not found.\n" );
			}
		//	for ( i = 0 ; i < args ; i++ )
		//		free(s[i]);
		//	free(s);
			
		}
	}
	return 0;
}

char ** get_command ( int *args ) {
	int chars, size, c;
	char **s;

	s = NULL;
	*args = 0;
	chars = 0;
	while ( ( c = getchar() ) != '\n' && c != '\r' ) {                     //Oso den vriskei EOF, telos grammhs, h carriage return
		if ( c == ' ' || c == '\t' ) {                                 //an vrei space, tab h pavla
			if ( chars ) {
				s[*args-1][chars] = '\0';                      //teleiwnei to string me \0
				chars = 0;                                     //kai mhdenizei to chars
			}
		}                                                 
		else {                                                         
			if ( chars == 0 ) {                                                    //->an to chars einai 0, au3anei ta orismata 
				if ( ( s = realloc ( s, (++*args)*sizeof( char * ) ) ) == NULL ) {
					perror("Error in memory allocation.\n");
					exit(-1);
				}
				size = 16;
				if ( ( s[*args-1] = malloc ( size*sizeof( char ) ) ) == NULL ) {
					perror("Error in memory allocation.\n");
					exit(-1);
				}
			}

			if ( chars >= size-1 ) {                                                        
				size *= 2;
				if ( ( s[*args-1] = realloc ( s[*args-1], size*sizeof( char ) ) )== NULL ) {
					perror("Error in memory allocation.\n");
					exit(-1);
				}
			}
			
			s[*args-1][chars] = c;                                   //grafei ton xarakthra
			chars++;                                                 //au3anei tou xarakthres tou string kata 1
		}
	}
	
	if ( s != NULL && chars && (*args ))                         //teleiwnei to teleutaio string otan vrei kapoion apo tous 3 
		s[*args-1][chars] = '\0';                 //xarakthres termatismou
	
	return s;                                         //epistrefei ton pinaka
}	
		
	
