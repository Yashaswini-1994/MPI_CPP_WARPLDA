#ifndef __TOPIC_MODELLING_H__
#define __TOPIC_MODELLING_H__

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include "config.h"
#include "utils.h"
#include "scanner.h"
#include "timer.h"


#include "./warp.hpp"

using namespace std;


class TopicModelling {
private:

    //#####################################################
    //############## General Variables ####################
    //#####################################################

    string outputFile;
    string filename;
    ConfigOptions* cfg;
    int numberOfTopics;
    int numberOfIterations;
    int numberOfDocuments;
    long seed;
    vector<string>* doc_index_map;


    

    //#####################################################
    //############### WarpLDA Variables ###################
    //#####################################################
    double** wldaDocTopDist;

public:
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg, string filename);

  ~TopicModelling();

  //#####################################################
  //############## General Functions ####################
  //#####################################################
  double getDistribution(int topic, int docNum);
  string getDocNameByNumber(int num);
  void WriteFiles(bool isfinal, string fileid);
  long LDA(string MyCount = "");
  int getMainTopic(int docNum);
  bool LoadDocMap();

  

  //#####################################################
  //################ WarpLDA Functions ##################
  //#####################################################
  long WLDA_LDA(string MyCount);
  bool WLDA_loadDistributions();
  bool WLDA_loadWords(vector<string>* topWords);
  void WLDA_WriteFiles(bool isfinal, string fileid);
};

#endif
