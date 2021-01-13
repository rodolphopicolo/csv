#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//int parse(char *path, struct Field **fields_ptr);
int main(int argument_count, char **arguments);
struct CSV_Field *initialize_csv_field(struct CSV_Field *field, int *fields_quantity, int current_field, int fields_block_quantity, int previous_char_position, int row, int column);

struct CSV_Field {
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


/*
 * After execution of parse_csv, it is import to free the memory allocated with:
 * free(*fields_ptr);
 * free(*csv_file_ptr);
 */
int parse_csv(char *path, struct CSV_Field **fields_ptr, int *fields_size, char **csv_file_ptr, int *csv_file_size){

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
	struct CSV_Field *fields = malloc(sizeof(struct CSV_Field)*fields_block_quantity);

	field_in_row = 0;
	int current_field = 0;
	fields = initialize_csv_field(fields, &fields_quantity, current_field, fields_block_quantity, -1, 0, current_field);

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

			current_char = *(buffer + i);

			*(csv_file + absolute_position) = current_char;

			if (current_char == '\n'){
				if((*(fields + current_field)).quotes % 2 == 1){
					fprintf(stderr, "Error 3 - Open quotes at absolute position %d, row %d, character %d\n", absolute_position, current_row, row_position);
					exit(EXIT_FAILURE);
				}
				current_row++;
				row_position = -1;
				field_in_row = 0;
				fields = initialize_csv_field(fields, &fields_quantity, ++current_field, fields_block_quantity, absolute_position, current_row, field_in_row);
			} else if (current_char == ',' && ((*(fields + current_field)).quoted == 0 || (*(fields + current_field)).quotes % 2 == 0)){
				fields = initialize_csv_field(fields, &fields_quantity, ++current_field, fields_block_quantity, absolute_position, current_row, ++field_in_row);
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
						//Last char should be been quotes.
						if(*(csv_file + absolute_position - 1) != '"'){
							fprintf(stderr, "Error 4 - Open quotes at absolute position %d, row %d, character %d\n", absolute_position, current_row, row_position);
							exit(EXIT_FAILURE);
						}
					}
				}
			}
		}
	}

	fclose(file);

	if(row_position == -1){
		current_field--;
	}

	if((*(fields + current_field)).quotes % 2 == 1){
		fprintf(stderr, "Error 5 - Open quotes at absolute position %d, row %d, character %d\n", absolute_position, current_row, row_position);
		exit(EXIT_FAILURE);
	}

	if(allocated_bytes > byte_counter){
		csv_file = realloc(csv_file, byte_counter);
		allocated_bytes = byte_counter;
	}

	if(fields_quantity > current_field){
		fields_quantity = current_field + 1;
		fields = realloc(fields, sizeof(struct CSV_Field)* fields_quantity);
	}

	*fields_ptr = fields;
	*csv_file_ptr = csv_file;

	*fields_size = fields_quantity;
	*csv_file_size = byte_counter;
}

void show_csv(struct CSV_Field *fields, int fields_quantity, char *csv_file){
	struct CSV_Field field;
	for(int i = 0; i < fields_quantity; i++){
		field = *(fields + i);
		char buffer[10];

		int bytes = field.last_non_blank_char - field.first_non_blank_char + 1;
		char message[128];

		snprintf(buffer, 10, "%d", bytes);

		strcpy(message, "\nrow %d, column %d, value %.");
		strcat(message, buffer);
		strcat(message, "s");
		printf(message, field.row, field.column, (csv_file + field.first_non_blank_char));
	}
}


struct CSV_Field *initialize_csv_field(struct CSV_Field *fields, int *fields_quantity, int current_field, int fields_block_quantity, int previous_char_position, int row, int column){
	if(*fields_quantity < current_field + 1){
		*fields_quantity += fields_block_quantity;
		fields = realloc(fields, sizeof(struct CSV_Field)* *fields_quantity);
	}

	*(fields + current_field) = (struct CSV_Field) {.row = row, .column = column, .quoted = -1, .quotes = 0, .first_char = -1, .first_non_blank_char = -1, .last_non_blank_char = -1, .last_char = -1};
	return fields;
}

/*
 * To run a test:
 * csv /path/to/csv/file.csv
 *
 * if the file has less than 100 rows, these rows will be output, else,
 * just the total rows will be output with the elapsed time.
 */
int main(int argument_count, char **arguments){

	if(argument_count < 2){
		fprintf(stderr, "Error 0 - No file specified");
		exit(EXIT_FAILURE);
	}


	char **csv_file_ptr = malloc(sizeof(char));
	struct CSV_Field **fields_ptr = malloc(sizeof(struct CSV_Field));
	int *fields_size, *csv_file_size, size1 = -1, size2 = -1;

	fields_size = &size1;
	csv_file_size = &size2;

	char file_name[strlen(arguments[1])];

	strcpy(&file_name, arguments[1]);

	printf("\n\n\nFile name: %s\n", file_name);

	struct timeval stop, start;
	long start_microseconds, stop_microseconds;
	double start_miliseconds, stop_miliseconds;


	gettimeofday(&start, NULL);
	parse_csv(file_name, fields_ptr, fields_size, csv_file_ptr, csv_file_size);
	gettimeofday(&stop, NULL);

	start_microseconds = start.tv_sec * 1e6 + start.tv_usec;
	stop_microseconds = stop.tv_sec * 1e6 + stop.tv_usec;

	long elapsed_micro = stop_microseconds - start_microseconds;

	float elapsed_mili = elapsed_micro / 1e3;

	struct CSV_Field last_field = *(*fields_ptr + *fields_size -1);
	int rows = last_field.row + 1;

	if(rows < 100){
		show_csv(*fields_ptr, *fields_size, *csv_file_ptr);
	} else {
		printf("\nIf in case of tests, if you want to see the output, choose a csv file with less than 100 rows");
	}
	printf("\n\n%d rows parsed in %.2f ms", rows, elapsed_mili);

	free(*fields_ptr);
	free(*csv_file_ptr);

	free(fields_ptr);
	free(csv_file_ptr);
    return 0;
}
