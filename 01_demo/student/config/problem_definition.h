// This should be equal to the "Problem id: xxxx" stated in the Subproblems tab
char* const problemID = "student_code";

// This is shown when the result is given to the student
char* const problemDescription = "";

// Output that the student should give
char* const problemAnswer = "";


/*
 *  Example how to use problemAnswer:
 *
 *  ----- Student output code -----
 *  std::cout << "My first output line! ";
 *  std::cout << "Still my first output line!" << std::endl;
 *  std::cout << "My second output line!" << std::endl;
 *
 *  ----- Template code to accept student output code -----
 *  char* const problemAnswer = "My first output line! Still my first output line!\n"
 *                            "My second output line!\n";
 *
 *
 * Note: Take care with "std::endl" --> this translates to "\n" !!!
 */