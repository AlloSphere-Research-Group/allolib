
#include <cstdio>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>

#include "al/spatialaudio/al_Convolver.hpp"
#include "al/core/io/al_AudioIO.hpp"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#define IR_SIZE 1024
#define BLOCK_SIZE 64 //min 64, max 8192

using namespace std;

TEST_CASE( "Class construction", "[convolver]" ) {
  al::Convolver conv;

  //create dummy IRs
  float IR1[IR_SIZE];
  memset(IR1, 0, sizeof(float)*IR_SIZE);
  IR1[0] = 1.0f;IR1[3] = 0.5f;
  float IR2[IR_SIZE];
  memset(IR2, 0, sizeof(float)*IR_SIZE);
  IR2[1] = 1.0f;IR2[2] = 0.25f;
  vector<float *> IRs;
  IRs.push_back(IR1);
  IRs.push_back(IR2);
  uint32_t IRlength = IR_SIZE;

  bool ret = conv.configure(BLOCK_SIZE, IRs, IRlength, {{0,{0}}, {1,{1}}});
  REQUIRE(ret);
  conv.processBuffer();
  conv.shutdown();
}

TEST_CASE( "Many to many", "[convolver]" ) {
  al::Convolver conv;

  //create dummy IRs
  float IR1[IR_SIZE];
  memset(IR1, 0, sizeof(float)*IR_SIZE);
  IR1[0] = 1.0f;IR1[3] = 0.5f;
  float IR2[IR_SIZE];
  memset(IR2, 0, sizeof(float)*IR_SIZE);
  IR2[1] = 1.0f;IR2[2] = 0.25f;
  vector<float *> IRs;
  IRs.push_back(IR1);
  IRs.push_back(IR2);
  uint32_t IRlength = IR_SIZE;


  unsigned int basePartitionSize = BLOCK_SIZE, options = 1;
  options = 1; //FFTW MEASURE
  //many to many mode
  bool ret = conv.configure(BLOCK_SIZE, IRs, IRlength, {{0,{0}}, {1,{1}}});

  REQUIRE(ret);
  // Fill input buffers
  float * busBuffer1 = conv.getInputBuffer(0);
  memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
  busBuffer1[0] = 1.0f;
  float * busBuffer2 = conv.getInputBuffer(1);
  memset(busBuffer2, 0, sizeof(float) * BLOCK_SIZE);
  busBuffer2[0] = 1.0f;

  bool procRet = conv.processBuffer();

  REQUIRE(procRet);

  for(int i = 0; i < BLOCK_SIZE; i++) {
    //std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
    //std::cout << "Y2: " << io.out(1, i) << ", H2: " << IR2[i] << std::endl;
    REQUIRE(fabs(conv.getOutputBuffer(0)[i] - IR1[i]) < 1e-07f);
    REQUIRE(fabs(conv.getOutputBuffer(1)[i] - IR2[i]) < 1e-07f);
  }
  conv.shutdown();
}

TEST_CASE( "One to many", "[convolver]" ) {
  al::Convolver conv;

  //create dummy IRs
  float IR1[IR_SIZE];
  memset(IR1, 0, sizeof(float)*IR_SIZE);
  IR1[0] = 1.0f;IR1[3] = 0.5f;
  float IR2[IR_SIZE];
  memset(IR2, 0, sizeof(float)*IR_SIZE);
  IR2[1] = 1.0f;IR2[2] = 0.25f;
  vector<float *> IRs;
  IRs.push_back(IR1);
  IRs.push_back(IR2);
  uint32_t IRlength = IR_SIZE;


  unsigned int basePartitionSize = BLOCK_SIZE;
  unsigned int options = 1; //FFTW MEASURE
  //one to many mode
  bool ret = conv.configure(BLOCK_SIZE, IRs, IRlength, {{0,{0, 1}}});

  // Fill input buffers
  float * busBuffer1 = conv.getInputBuffer(0);
  memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
  busBuffer1[0] = 1.0f;

  bool procRet = conv.processBuffer();

  REQUIRE(procRet);

  for(int i = 0; i < BLOCK_SIZE; i++) {
    //std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
    //std::cout << "Y2: " << io.out(1, i) << ", H2: " << IR2[i] << std::endl;
    REQUIRE(fabs(conv.getOutputBuffer(0)[i] - IR1[i]) < 1e-07f);
    REQUIRE(fabs(conv.getOutputBuffer(1)[i] - IR2[i]) < 1e-07f);
  }
  conv.shutdown();
}

//TEST_CASE( "Vector mode", "[convolver]" ) {
//  al::Convolver conv;
//  al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2, al::AudioIO::DUMMY);
//  REQUIRE(io.channelsOut() == 2);
//  REQUIRE(io.framesPerSecond() == 44100.0f);
//  io.append(conv);
//  io.channelsBus(2);

//  //create dummy IRs
//  float IR1[IR_SIZE];
//  memset(IR1, 0, sizeof(float)*IR_SIZE);
//  IR1[0] = 1.0f;IR1[3] = 0.5f;
//  float IR2[IR_SIZE];
//  memset(IR2, 0, sizeof(float)*IR_SIZE);
//  IR2[1] = 1.0f;IR2[2] = 0.25f;
//  vector<float *> IRs;
//  IRs.push_back(IR1);
//  IRs.push_back(IR2);
//  int IRlength = IR_SIZE;

//  //create dummy input buffers
//  float * busBuffer1 = io.busBuffer(0);
//  memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
//  busBuffer1[0] = 1.0f;
//  float * busBuffer2 = io.busBuffer(1);
//  memset(busBuffer2, 0, sizeof(float) * BLOCK_SIZE);
//  busBuffer2[0] = 1.0f;

//  unsigned int basePartitionSize = BLOCK_SIZE, options = 1;
//  options |= 2; //vector mode
//  conv.configure(io, IRs, IRlength, -1, true, vector<int>(), basePartitionSize, options);
//  io.processAudio();
//  for(int i = 0; i < BLOCK_SIZE; i++) {
//    //std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
//    //std::cout << "Y2: " << io.out(1, i) << ", H2: " << IR2[i] << std::endl;
//    REQUIRE(fabs(io.out(0, i) - IR1[i]) < 1e-07f);
//    REQUIRE(fabs(io.out(1, i) - IR2[i]) < 1e-07f);
//  }
//  conv.shutdown();
//}

