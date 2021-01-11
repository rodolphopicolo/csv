#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int parse(char *path);
int main(int argument_count, char **arguments);

struct Field {
	int row;
	int column;
	int quoted;
	int quotes;
	int previous_char;
	int first_char;
	int first_non_blank_char;
	int last_non_blank_char;
	int last_char;
};


struct Field *initialize_field(struct Field *field, int *fields_quantity, int current_field, int fields_block_quantity, int previous_char_position, int row, int column);

int parse(char *path){

	const int initial_block_size = 4096;
	const int resize_block_size = 4096;
	const int buffer_size = 4096;
	const int fields_block_quantity = 1024;
	int fields_quantity, field_in_row;

	FILE *file;
	char buffer[buffer_size];
	int absolute_position, row_position, current_row;

	int allocated_bytes;

	char *csv_file;
	char current_char;

	allocated_bytes = initial_block_size * sizeof(char);
	csv_file = malloc(allocated_bytes);
	if(csv_file == NULL){
		fprintf(stderr, "Error 1 - Unable to allocate required memory\n");
		exit(EXIT_FAILURE);
	}

	file = fopen(path, "r");

	int byte_counter = 0;

	fields_quantity = fields_block_quantity;
	struct Field *fields = malloc(sizeof(struct Field)*fields_block_quantity);

	field_in_row = 0;
	int current_field = 0;
	fields = initialize_field(fields, &fields_quantity, current_field, fields_block_quantity, -1, 0, current_field);

	current_row = 0;
	row_position = -1;
	while( fgets(buffer, sizeof buffer, file) != NULL ){
		for(int i = 0; i < (sizeof buffer) && *(buffer + i) != 0; i++){

			if(byte_counter == allocated_bytes){
				allocated_bytes += resize_block_size * sizeof(char);
				csv_file = realloc(csv_file, allocated_bytes);
				if(csv_file == NULL){
					fprintf(stderr, "Error 2 - Unable to allocate required memory\n");
					exit(EXIT_FAILURE);
				}
			}

			absolute_position = byte_counter++ * sizeof(char);
			row_position++;
			//current_char = buffer[i]
			current_char = *(buffer + i);

			*(csv_file + absolute_position) = current_char;

			if (current_char == '\n'){
				if((*(fields + current_field)).quotes % 2 == 1){
					fprintf(stderr, "Error 3 - Open quotes at absolute position %d, row %d, character %d\n", absolute_position, current_row, row_position);
					exit(EXIT_FAILURE);
				}
//				printf("\nnew line");
//				printf("\nnew field");
				current_row++;
				row_position = 0;
				field_in_row = 0;
				fields = initialize_field(fields, &fields_quantity, ++current_field, fields_block_quantity, absolute_position, current_row, field_in_row);
			} else if (current_char == ',' && ((*(fields + current_field)).quoted == 0 || (*(fields + current_field)).quotes % 2 == 0)){
//				printf("\nnew field");
				fields = initialize_field(fields, &fields_quantity, ++current_field, fields_block_quantity, absolute_position, current_row, ++field_in_row);
			} else {

				if((*(fields + current_field)).first_char == -1){
					(*(fields + current_field)).first_char = absolute_position;
				}
				if((*(fields + current_field)).first_non_blank_char == -1 && current_char != ' '){
					(*(fields + current_field)).first_non_blank_char = absolute_position;
					if(current_char == '"'){
						(*(fields + current_field)).quoted = 1;
					} else {
						(*(fields + current_field)).quoted = 0;
					}
				}
				(*(fields + current_field)).last_char = absolute_position;
				if(current_char != ' '){
					(*(fields + current_field)).last_non_blank_char = absolute_position;
				}
				if(current_char == '"'){
					(*(fields + current_field)).quotes++;

					if((*(fields + current_field)).quotes > 1 && (*(fields + current_field)).quotes % 2 == 1){
						//Último caracter tem que ter sido aspas.
						if(*(csv_file + absolute_position - 1) != '"'){
							fprintf(stderr, "Error 4 - Open quotes at absolute position %d, row %d, character %d\n", absolute_position, current_row, row_position);
							exit(EXIT_FAILURE);
						}
					}
				}
			}

//			printf("%c", current_char);
		}
	}
	if((*(fields + current_field)).quotes % 2 == 1){
		fprintf(stderr, "Error 5 - Open quotes at absolute position %d, row %d, character %d\n", absolute_position, current_row, row_position);
		exit(EXIT_FAILURE);
	}

	if(allocated_bytes > byte_counter){
		csv_file = realloc(csv_file, byte_counter);
		allocated_bytes = byte_counter;
	}

	fclose(file);
//	printf("\n\n\ncsv_file:\n[");
//	for(int i = 0; i < byte_counter; i++){
//		printf("%c", csv_file[i]);
//	}
//	printf("]\n\n\nFim de arquivo.");

	free(csv_file);
	free(fields);
}


struct Field *initialize_field(struct Field *fields, int *fields_quantity, int current_field, int fields_block_quantity, int previous_char_position, int row, int column){
	if(*fields_quantity < current_field + 1){
		*fields_quantity += fields_block_quantity;
		fields = realloc(fields, sizeof(struct Field)* *fields_quantity);
	}

	*(fields + current_field) = (struct Field) {.row = row, .column = column, .quoted = -1, .quotes = 0, .first_char = -1, .first_non_blank_char = -1, .last_non_blank_char = -1, .last_char = -1};
	return fields;
}

int main(int argument_count, char **arguments){

	if(argument_count < 2){
		fprintf(stderr, "Error 0 - No file specified");
		exit(EXIT_FAILURE);
	}


	char **csv_file_pointer;
	struct Field **fields_pointer;

	char file_name[strlen(arguments[1])];

	strcpy(&file_name, arguments[1]);

	printf("\n\n\n=================================\nCSV Parser\n=================================\n\n\n");
	printf("File name: %s\n", file_name);

	struct timeval stop, start;
	long start_microseconds, stop_microseconds;
	double start_miliseconds, stop_miliseconds;

	for(int i = 0; i < 10; i++){
		gettimeofday(&start, NULL);
		parse(file_name);
		gettimeofday(&stop, NULL);

		start_microseconds = start.tv_sec * 1e6 + start.tv_usec;
		stop_microseconds = stop.tv_sec * 1e6 + stop.tv_usec;

		start_miliseconds = start_microseconds / 1e3;
		stop_miliseconds = stop_microseconds / 1e3;

		long elapsed = stop_miliseconds - start_miliseconds;

		printf("\n\n\nElapsed time: %i ms", elapsed);
	}
    return 0;
}
