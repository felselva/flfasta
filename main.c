/*
Copyright (c) 2019 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
     that you wrote the original software. If you use this software in a
     product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define APPLICATION_VERSION_YYYY 2019
#define APPLICATION_VERSION_MM 01
#define APPLICATION_VERSION_DD 24
#define APPLICATION_VERSION_MICRO 0

#define PATH_LENGTH_MAX 4096

struct context {
	uint8_t print_help;
	uint8_t file_input_path[PATH_LENGTH_MAX];
	uint8_t file_output_path[PATH_LENGTH_MAX];
	FILE *file_input;
	FILE *file_output;
	uint32_t sequence_length_minimum;
	uint32_t sequence_length_maximum;
	uint8_t show_progress;
};

#define STATE_NONE 0
#define STATE_HEADER 1
#define STATE_SEQUENCE 2
#define PROGRESS_DIVISOR 100

static void read_write_entries(struct context *context)
{
	uint8_t state;
	uint64_t header_position;
	uint64_t sequence_position;
	uint64_t sequence_size;
	uint8_t byte;
	uint64_t file_position;
	uint64_t file_size;
	uint64_t total_of_sequences_kept;
	uint64_t total_of_sequences_discarded_above_maximum;
	uint64_t total_of_sequences_discarded_bellow_minimum;
	uint64_t current_line;
	state = 0;
	header_position = 0;
	sequence_position = 0;
	sequence_size = 0;
	current_line = 1;
	total_of_sequences_kept = 0;
	total_of_sequences_discarded_bellow_minimum = 0;
	total_of_sequences_discarded_above_maximum = 0;
	fseek(context->file_input, 0, SEEK_END);
	file_size = ftell(context->file_input);
	fseek(context->file_input, 0, SEEK_SET);
	while (fread(&byte, 1, 1, context->file_input) == 1) {
		if (byte == '\n') {
			current_line = current_line + 1;
		}
		if (state == STATE_NONE) {
			if (byte == '>') {
				state = STATE_HEADER;
				header_position = ftell(context->file_input) - 1;
			} else if (byte == '\n') {
				/* There are empty lines in the file */
			}
		} else if (state == STATE_HEADER) {
			if (byte == '\n') {
				state = STATE_SEQUENCE;
				sequence_position = ftell(context->file_input);
				sequence_size = 0;
			}
		} else if (state == STATE_SEQUENCE) {
			if (byte == '\n') {
				state = STATE_NONE;
				if (sequence_size < context->sequence_length_minimum) {
					total_of_sequences_discarded_bellow_minimum = total_of_sequences_discarded_bellow_minimum + 1;
				} else if (sequence_size > context->sequence_length_maximum && context->sequence_length_maximum > 0) {
					total_of_sequences_discarded_above_maximum = total_of_sequences_discarded_above_maximum + 1;
				} else {
					total_of_sequences_kept = total_of_sequences_kept + 1;
					fseek(context->file_input, header_position, SEEK_SET);
					while ((uint64_t)ftell(context->file_input) < sequence_position + sequence_size + 1) {
						fputc(fgetc(context->file_input), context->file_output);
					}
				}
			} else {
				sequence_size = sequence_size + 1;
			}
		}
		file_position = ftell(context->file_input);
		if (context->show_progress == 1 && (file_position % PROGRESS_DIVISOR == 0 || file_position >= file_size)) {
			fprintf(stdout, "\rFiltering by length at %0.2f%%.", ((double)file_position / (double)file_size) * 100.0);
			fflush(stdout);
		}
	}
	if (context->show_progress == 1) {
		fprintf(stdout, "\n");
		fflush(stdout);
	}
	printf("Total of sequences kept: %lu.\n", total_of_sequences_kept);
	printf("Total of sequences discarded bellow minimum: %lu.\n", total_of_sequences_discarded_bellow_minimum);
	printf("Total of sequences discarded above maximum: %lu.\n", total_of_sequences_discarded_above_maximum);
}

static void print_help(void)
{
	printf("flfasta (Filter by Length FASTA) version %04u.%02u.%02u.%u.\n",
		APPLICATION_VERSION_YYYY,
		APPLICATION_VERSION_MM,
		APPLICATION_VERSION_DD,
		APPLICATION_VERSION_MICRO);
	puts("Created by Felipe Ferreira da Silva.");
	puts("");
	puts("Usage:");
	puts("  sc [1] [2] [3] [4] [5]");
	puts("");
	puts("  1: Path to input file.");
	puts("  2: Path to output file.");
	puts("  3: Minimum length.");
	puts("  4: Maximum length (0 to ignore maximum length).");
	puts("  5: Show progress (0 for no, 1 for yes).");
	puts("");
	puts("File input must be in single-line FASTA format.");
}

int main(int argc, char **args)
{
	uint8_t success;
	int32_t status;
	struct context *context;
	success = 1;
	context = malloc(sizeof(struct context));
	if (context == NULL) {
		success = 0;
		goto done;
	}
	memset(context, 0, sizeof(struct context));
	/* Arguments */
	if (argc == 6) {
		strncat((char *)context->file_input_path, args[1], PATH_LENGTH_MAX - 1);
		strncat((char *)context->file_output_path, args[2], PATH_LENGTH_MAX - 1);
		context->sequence_length_minimum = atoi(args[3]);
		context->sequence_length_maximum = atoi(args[4]);
		context->show_progress = atoi(args[5]);
	} else {
		context->print_help = 1;
	}
	if (context->print_help == 1) {
		print_help();
		goto freecontext;
	}
	printf("  Input: \"%s\"\n", context->file_input_path);
	printf("  Output: \"%s\"\n", context->file_output_path);
	printf("  Sequence minimum length: %u\n", context->sequence_length_minimum);
	printf("  Sequence maximum length: %u\n", context->sequence_length_maximum);
	context->file_input = fopen((char *)context->file_input_path, "r");
	if (context->file_input == NULL) {
		puts("Failed to open the input file for reading.");
		success = 0;
		goto freecontext;
	}
	context->file_output = fopen((char *)context->file_output_path, "w");
	if (context->file_output == NULL) {
		puts("Failed to open the output file for writing.");
		success = 0;
		goto freecontext;
	}
	read_write_entries(context);
freecontext:
	if (context->file_input != NULL) {
		fclose(context->file_input);
	}
	if (context->file_output != NULL) {
		fclose(context->file_output);
	}
	free(context);
done:
	if (success == 1) {
		status = EXIT_SUCCESS;
	} else {
		status = EXIT_FAILURE;
	}
	return status;
}
