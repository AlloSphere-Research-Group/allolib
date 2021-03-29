
#include "al/io/al_CSVReader.hpp"

using namespace al;

typedef struct {
  char s[32];
  double val1, val2, val3;
  bool b;
} RowTypes;

typedef struct {
  char s[32];
  int64_t intVal;
  double val1, val2, val3, val4;
} RowTypes2;

int main(int argc, char *argv[]) {
  CSVReader reader;
  reader.addType(CSVReader::STRING);
  reader.addType(CSVReader::REAL);
  reader.addType(CSVReader::REAL);
  reader.addType(CSVReader::REAL);
  reader.addType(CSVReader::BOOLEAN);
  reader.readFile("data/test.csv");

  // print column names
  for (auto name : reader.getColumnNames()) {
    std::cout << name << " | ";
  }
  std::cout << std::endl << "---------------" << std::endl;

  // Copy data to struct
  std::vector<RowTypes> rows = reader.copyToStruct<RowTypes>();
  for (auto row : rows) {
    std::cout << std::string(row.s) << " : " << row.val1 << "   " << row.val2
              << "   " << row.val3 << "   " << (row.b ? "+" : "-") << std::endl;
  }
  std::cout << " ---------- Num rows:" << rows.size() << std::endl;

  // Get data by column
  std::vector<double> column1 = reader.getColumn(1);
  for (auto value : column1) {
    std::cout << value << std::endl;
  }

  // Now try a file separated by spaces
  reader.clearTypes();

  reader.addType(CSVReader::STRING);
  reader.addType(CSVReader::INT64);
  reader.addType(CSVReader::REAL);
  reader.addType(CSVReader::REAL);
  reader.addType(CSVReader::REAL);
  reader.addType(CSVReader::REAL);

  reader.readFile("data/test2.txt");

  for (auto name : reader.getColumnNames()) {
    std::cout << name << " | ";
  }
  std::cout << std::endl << "---------------" << std::endl;
  auto rowsFromTxt = reader.copyToStruct<RowTypes2>();
  for (auto row : rowsFromTxt) {
    std::cout << std::string(row.s) << " : " << row.intVal << "   " << row.val1
              << "   " << row.val2 << "   " << row.val3 << "   " << row.val4
              << std::endl;
  }

  return 0;
}
