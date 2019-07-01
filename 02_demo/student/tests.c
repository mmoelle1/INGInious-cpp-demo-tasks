#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "CTester/CTester.h"
#include "bridge.hpp"

#include "config/problem_definition.h"

// Function to save output to a file so it can be displayed later
void SaveOutputToFile(char* fileName, int contentInt, char* contentData, ssize_t* contentLength) {

    FILE* file = fopen(fileName, "w");
    char buf[BUFSIZ];
    if (file != NULL) {
      	ssize_t readlen = read(contentInt, buf, BUFSIZ);
	if (readlen <= 0) {
	    // Write error message to file
	    //char* message = "Your program does not produce any output!";
	    //fprintf(file, message);
	    fclose(file);
	    *contentLength = 0;
	    // Write error message to contentData
	    //*contentLength = strlen(message);
	    //contentData = (char*)malloc((*contentLength + 1) * sizeof(char));
	    //strcpy(contentData, message);
	} else {
	    // Write program output to file in chunks of size BUFSIZ
  	    *contentLength = 0;
	    while (readlen > 0) {
	        *contentLength += readlen;
		fwrite(buf, sizeof(char), readlen, file);
		readlen = read(contentInt, buf, BUFSIZ);
	    }
	    if (*contentLength > 65535) {
  	        fclose(file);
	        // Write error message to file
	        char* message = "!!! Your program's output exceeds the maximum limit of 65535 characters!\n!!! This is a limitation of the INGInious website.\n!!! Please reduce the verbosity of your program's output and try again.";
		file = fopen(fileName, "w");
		fprintf(file, message);
		fclose(file);
		// Write error message to contentData
		*contentLength = strlen(message);
		contentData = (char*)malloc((*contentLength + 1) * sizeof(char));
		strcpy(contentData, message);
		abort();
	    } else {
	      fclose(file);
	    }

	    // Import program output into contentData
	    contentData = (char*)malloc((*contentLength + 1) * sizeof(char));
	    file = fopen(fileName, "r");
	    fread(contentData, sizeof(contentData), *contentLength, file);
	    fclose(file);
	}
    } else {
      // Write error message to file
      char* message = "An internal error occured while opening the output file. If the error persists, contact the system administrator";
      contentData = (char*)malloc((strlen(message) + 1) * sizeof(char));
      strcpy(contentData, message);
      fprintf(file, message);
      fclose(file);
    }
}

void test_code(int argc, char* argv[]) {
    set_test_metadata(problemID, _(problemDescription), 1);

    SANDBOX_BEGIN;
    test_student_code(argc, argv);
    SANDBOX_END;
    
    char* studentOutput = NULL;
    ssize_t studentOutputLength = 0;
    SaveOutputToFile("student_code_output", stdout_cpy, studentOutput, &studentOutputLength);

    char* studentError = NULL;
    ssize_t studentErrorLength = 0;
    SaveOutputToFile("student_code_error", stderr_cpy, studentError, &studentErrorLength);

    char* templateOutput = problemAnswer;
    int templateOutputLength = strlen(templateOutput);
        
    if(templateOutputLength != 0){
        CU_ASSERT_EQUAL(studentOutputLength, templateOutputLength);
        CU_ASSERT_TRUE(!strncmp(studentOutput, templateOutput, templateOutputLength));
    }

    free(studentOutput);
    free(studentError);
}

int main(int argc, char* argv[])
{
    RUN(test_code);
}
