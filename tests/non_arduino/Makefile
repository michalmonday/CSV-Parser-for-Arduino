

CC = g++
CSV_PARSER_DIR = ../../
CSV_PARSER_NAME = CSV_Parser
CFLAGS = -g -Wall -I$(CSV_PARSER_DIR) -L$(CSV_PARSER_DIR) -DNON_ARDUINO -DCSV_PARSER_DONT_IMPORT_SD
TARGET = non_arduino_test

all: $(TARGET) 

library: *.cpp $(CSV_PARSER_DIR)*.cpp 
	$(CC) $(CFLAGS) -c $(CSV_PARSER_DIR)non_arduino_adaptations.cpp -o $(CSV_PARSER_DIR)non_arduino_adaptations.o
	$(CC) $(CFLAGS) -c $(CSV_PARSER_DIR)$(CSV_PARSER_NAME).cpp -o $(CSV_PARSER_DIR)$(CSV_PARSER_NAME).o
	$(CC) $(CFLAGS) -c $(TARGET).cpp -o $(TARGET).o

$(TARGET): library $(TARGET).cpp
	$(CC) $(CFLAGS) $(CSV_PARSER_DIR)non_arduino_adaptations.o $(TARGET).o $(CSV_PARSER_DIR)$(CSV_PARSER_NAME).o -o $(TARGET)

clean:
	rm -rf *.o $(CSV_PARSER_DIR)*.o 