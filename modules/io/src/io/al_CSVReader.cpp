#include "al/io/al_CSVReader.hpp"

#include <cassert>
#include <cctype>

using namespace al;

CSVReader::~CSVReader() {
  for (auto row : mData) {
    delete[] row;
  }
  mData.clear();
}

bool CSVReader::readFile(std::string fileName, bool hasColumnNames) {
  if (mBasePath.size() > 0) {
    if (mBasePath.back() == '/') {
      fileName = mBasePath + fileName;
    } else {
      fileName = mBasePath + "/" + fileName;
    }
  }
  std::ifstream f(fileName);
  if (!f.is_open()) {
    std::cout << "Could not open:" << fileName << std::endl;
    return false;
  }

  mColumnNames.clear();
  mData.clear();

  std::string line;
  size_t rowLength = calculateRowLength();

  // Infer separator
  bool commaSeparated = false;
  {
    if (hasColumnNames) {
      getline(f, line);
    }
    getline(f, line);  // Infer from first line of data
    if (std::count(line.begin(), line.end(), ',') > 0) {
      commaSeparated = true;
    }
    // Reset file
    f.clear();
    f.seekg(0, std::ios::beg);
  }

  if (hasColumnNames) {
    getline(f, line);
    std::stringstream columnNameStream(line);
    std::string columnName;
    if (commaSeparated) {
      while (std::getline(columnNameStream, columnName, ',')) {
        mColumnNames.push_back(columnName);
      }
    } else {
      while (std::getline(columnNameStream, columnName, ' ')) {
        if (columnName.size() > 0) {
          mColumnNames.push_back(columnName);
        }
      }
    }
  }

  //	std::cout << line << std::endl;
  while (getline(f, line)) {
    if (line.size() == 0) {
      continue;
    }
    std::stringstream ss(line);
    char *row = new char[rowLength];
    memset(row, 0, rowLength);
    mData.push_back(row);
    if (commaSeparated) {
      if ((unsigned long)std::count(line.begin(), line.end(), ',') ==
          mDataTypes.size() - 1) {  // Check that we have enough commas
        size_t byteCount = 0;
        for (auto type : mDataTypes) {
          std::string field;
          std::getline(ss, field, ',');
          size_t stringLen = std::min(maxStringSize - 1, field.size());
          int64_t intValue;
          double doubleValue;
          bool booleanValue;
          auto data = row + byteCount;
          assert((size_t)(data - mData.back()) < rowLength);
          switch (type) {
            case STRING:
              std::memcpy(data, field.data(), stringLen * sizeof(char));
              byteCount += maxStringSize * sizeof(char);
              break;
            case INT64:
              intValue = std::atol(field.data());
              std::memcpy(data, &intValue, sizeof(int64_t));
              byteCount += sizeof(int64_t);
              break;
            case REAL:
              doubleValue = std::atof(field.data());
              std::memcpy(data, &doubleValue, sizeof(double));
              byteCount += sizeof(double);
              break;
            case BOOLEAN:
              booleanValue = field == "True" || field == "true" || field == "1";
              std::memcpy(data, &booleanValue, sizeof(bool));
              byteCount += sizeof(bool);
              break;
            case IGNORE_COLUMN:
              break;
          }
        }
      }
    } else {  // Space separated
      int byteCount = 0;
      for (auto type : mDataTypes) {
        std::string field;
        while (field.size() == 0) {
          std::getline(ss, field, ' ');
          if (ss.eof()) {
            break;
          }
        }
        if (field.size() == 0) {
          break;
        }
        // Trim white space from start
        field.erase(field.begin(),
                    std::find_if(field.begin(), field.end(),
                                 [](int ch) { return !std::isspace(ch); }));
        // Trim white space from end
        field.erase(std::find_if(field.rbegin(), field.rend(),
                                 [](int ch) { return !std::isspace(ch); })
                        .base(),
                    field.end());
        size_t stringLen = std::min(maxStringSize - 1, field.size());
        int64_t intValue;
        double doubleValue;
        bool booleanValue;

        auto data = row + byteCount;
        assert((size_t)(data - mData.back()) < rowLength);
        switch (type) {
          case STRING:
            std::memcpy(data, field.data(), stringLen * sizeof(char));
            byteCount += maxStringSize * sizeof(char);
            break;
          case INT64:
            intValue = std::atol(field.data());
            std::memcpy(data, &intValue, sizeof(int64_t));
            byteCount += sizeof(int64_t);
            break;
          case REAL:
            doubleValue = std::atof(field.data());
            std::memcpy(data, &doubleValue, sizeof(double));
            byteCount += sizeof(double);
            break;
          case BOOLEAN:
            booleanValue = field == "True" || field == "true" || field == "1";
            std::memcpy(data, &booleanValue, sizeof(bool));
            byteCount += sizeof(bool);
            break;
          case IGNORE_COLUMN:
            break;
        }
      }
    }
  }
  f.close();
  //        if (f.bad()) {
  //                std::cout << "Error ____ reading:" << fileName << std::endl;
  //        }
  return !f.bad();
}

std::vector<double> CSVReader::getColumn(int index) {
  std::vector<double> out;
  int offset = 0;
  for (int i = 0; i < index; i++) {
    switch (mDataTypes[i]) {
      case STRING:
        offset += maxStringSize * sizeof(char);
        break;
      case INT64:
        offset += sizeof(int64_t);
        break;
      case REAL:
        offset += sizeof(double);
        break;
      case BOOLEAN:
        offset += sizeof(bool);
        break;
      case IGNORE_COLUMN:
        break;
    }
  }
  //	std::cout << offset << std::endl;
  for (auto row : mData) {
    double *val = (double *)(row + offset);
    out.push_back(*val);
  }
  return out;
}

size_t CSVReader::calculateRowLength() {
  size_t len = 0;
  ;
  for (auto type : mDataTypes) {
    switch (type) {
      case STRING:
        len += maxStringSize * sizeof(char);
        break;
      case INT64:
        len += sizeof(int64_t);
        break;
      case REAL:
        len += sizeof(double);
        break;
      case BOOLEAN:
        len += sizeof(bool);
        break;
      case IGNORE_COLUMN:
        break;
    }
  }
  //	std::cout << len << std::endl;
  return len;
}
