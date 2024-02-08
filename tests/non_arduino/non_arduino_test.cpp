
#include <CSV_Parser.h>
#include <stdio.h>

const char * csv_str = "my_strings,my_ints\n"
                       "hello,1\n"
                       "world,2\n"
                       "noice,3\n"
                       "hehe,4\n";

int main() {
    CSV_Parser cp(/*format*/ "Ls-------s--");
    // read csv file 
    FILE * file = fopen("file4.csv", "r");
    if (file == NULL) {
        printf("Error: file not found\n");
        return 1;
    }
    char c = fgetc(file);
    while (c != EOF) {
        cp << c;
        c = fgetc(file);
    }
    cp.parseLeftover();
    cp.print();
    return 0;
}